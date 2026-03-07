#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "disk.h"
#include "memdefs.h"
#include "fat.h"
#include "memory.h"
#include "ctype.h"

#define SECTOR_SIZE 512
#define CLUSTER_SIZE 2048
#define MAX_PATH_SIZE 256
#define MAX_FILES_HANDLE 10
#define ROOT_DIRECTORY_HANDLE -1

bool formatDirectory(DISK* disk, uint32_t newCluster, uint32_t parentCluster);
bool writeEntryToDirectory(DISK* disk, uint32_t directoryCluster, DirectoryEntry* newDirectoryEntry);
bool getEntryByPath(DISK* disk, const char* path, DirectoryEntry* dirEntryOut);

typedef struct {
    uint8_t Buffer[SECTOR_SIZE];
    File Public;
    bool Opened;
    uint32_t FirstCluster;
    uint32_t CurrentCluster;
    uint32_t CurrentSectorInCluster;
} FileData;

typedef struct {
    union 
    {
        BootSector bootSector;
        uint8_t BootSectorBytes[SECTOR_SIZE];
    } BS;

    FileData RootDirectory;
    FileData OpenedFiles[MAX_FILES_HANDLE];
} FAT_Data;

static FAT_Data far* b_Data;
static uint8_t far* b_Fat = NULL;
static uint32_t b_DataSectionLba;


bool readBootSector(DISK* disk)
{
    return diskReadSectors(disk, 0, 1, b_Data->BS.BootSectorBytes);
}

bool readSectors(DISK* disk, uint32_t lba, uint32_t count, void* bufferOut)
{
    bool ok = true;
    ok = ok && diskReadSectors(disk, lba, count, bufferOut);
    return ok;
}

bool writeSectors(DISK* disk, uint32_t lba, uint8_t count, void* buffer) {
    bool ok = true;
    ok = ok && diskWriteSectors(disk, lba, count, buffer);
    return ok;
}

uint32_t findFreeCluster(DISK* disk) {
    int i;
    uint32_t totalSectors = b_Data->BS.bootSector.TotalSectors32;
    uint32_t sectorsPerCluster = b_Data->BS.bootSector.SectorsPerCluster;
    uint32_t totalClusters;
    uint8_t shift = 0;

    uint32_t spc = sectorsPerCluster;

    while (spc > 1)
    {
        spc = spc >> 1;
        shift++;
    }

    totalClusters = totalSectors >> shift;

    for (i = 2; i < totalClusters; i++) {
        if (b_Fat[i] == 0x0000)
        {
            printf("Free cluster: %d", i);
            return i;
        }
    }

    return 0;
}

bool readFat(DISK* disk)
{
    return diskReadSectors(disk, b_Data->BS.bootSector.ReservedSectorCount, b_Data->BS.bootSector.SectorsPerFat, b_Fat);
}

bool fatInitialize(DISK* disk, DirectoryEntry* dirEntry) {
    uint32_t fatSize;
    uint32_t rootDirLba;
    uint32_t rootDirSize;
    uint32_t rootDirSectors;
    int i;
    
    b_Data = (FAT_Data far*)MEMORY_FAT_ADDR;

    if (!readBootSector(disk)) {
        printf("Can't read boot sector.\r\n");
        return false;
    }

    b_Fat = (uint8_t far*)b_Data + sizeof(FAT_Data);
    fatSize = b_Data->BS.bootSector.BytesPerSector * b_Data->BS.bootSector.SectorsPerFat;
    if (sizeof(FAT_Data) + fatSize >= MEMORY_FAT_SIZE) {
        printf("Not enough memory to read FAT.");
        return false;
    }

    if (!readFat(disk)) {
        printf("Can't read FAT.\r\n");
        return false;
    }

    rootDirLba = b_Data->BS.bootSector.ReservedSectorCount + b_Data->BS.bootSector.TableCount * b_Data->BS.bootSector.SectorsPerFat;
    rootDirSize = sizeof(DirectoryEntry) * b_Data->BS.bootSector.RootEntryCount;

    b_Data->RootDirectory.Public.Handle = ROOT_DIRECTORY_HANDLE;
    b_Data->RootDirectory.Public.IsDirectory = true;
    b_Data->RootDirectory.Public.Position = 0;
    b_Data->RootDirectory.Public.Size = rootDirSize;
    b_Data->RootDirectory.Opened = true;
    b_Data->RootDirectory.FirstCluster = rootDirLba;
    b_Data->RootDirectory.CurrentCluster = rootDirLba;
    b_Data->RootDirectory.CurrentSectorInCluster = 0;

    if (!diskReadSectors(disk, rootDirLba, 1, b_Data->RootDirectory.Buffer))
    {
        printf("FAT: read root directory failed\r\n");
        return false;
    }
    printf("OEM Name: %s\r\n", b_Data->BS.bootSector.OemName);
    printf("Bytes Per Sector: %d\r\n", b_Data->BS.bootSector.BytesPerSector);
    printf("Signature: %x\r\n", b_Data->BS.bootSector.Signature);

    rootDirSectors = (rootDirSize + b_Data->BS.bootSector.BytesPerSector - 1) / b_Data->BS.bootSector.BytesPerSector;
    b_DataSectionLba = rootDirLba + rootDirSectors;

    for (i = 0; i < MAX_FILES_HANDLE; i++) {
        b_Data->OpenedFiles[i].Opened = false;
    }

    for (i = 0; i < 11; i++)
    {
        dirEntry->FileName[i] = ' ';
    }

    dirEntry->FileName[10] = '\0';
    dirEntry->FileSize = rootDirSize;
    dirEntry->LowFirstClusterNumber = b_Data->RootDirectory.FirstCluster;

    return true;
}

uint32_t clusterToLba(uint32_t cluster) {
    return b_DataSectionLba + (cluster - 2) * b_Data->BS.bootSector.SectorsPerCluster;
}

File far* openEntry(DISK* disk, DirectoryEntry* entry) {
    int handle = -1;
    int i;
    FileData far* fileData;
    for (i = 0; i < MAX_FILES_HANDLE; i++) {
        if (!b_Data->OpenedFiles[i].Opened) {
            handle = i;
        }
    }

    if (handle < 0) {
        printf("Fat is out of file handles.\r\n");
        return NULL;
    }

    fileData = &b_Data->OpenedFiles[handle];
    fileData->Public.Handle = handle;
    fileData->Public.IsDirectory = (entry->Attributes & ATTRIBUTE_DIRECTORY) != 0;
    fileData->Public.Position = 0;
    fileData->Public.Size = entry->FileSize;
    fileData->FirstCluster = entry->LowFirstClusterNumber;
    fileData->CurrentCluster = fileData->FirstCluster;
    fileData->CurrentSectorInCluster = 0;

    if (!diskReadSectors(disk, clusterToLba(fileData->CurrentCluster), 1, fileData->Buffer)) {
        printf("Couldn't read the first sector of the file.\r\n");
        return NULL;
    }

    fileData->Opened = true;
    return &fileData->Public;
}

uint32_t nextCluster(uint32_t currentCluster) {
    return (*(uint16_t*)(b_Fat + 2 * currentCluster));
}

uint32_t read(DISK* disk, File far* file, uint32_t byteCount, void* bufferOut) {
    uint32_t leftInBuffer; //how many free bytes the buffer has.
    uint32_t take; // how many bytes we will take into buffer.

    FileData far* fileData = (file->Handle == ROOT_DIRECTORY_HANDLE) ? &(b_Data->RootDirectory) : &(b_Data->OpenedFiles[file->Handle]);

    uint8_t* u8OutBuffer = (uint8_t*) bufferOut;

    if (!fileData->Public.IsDirectory) {
        byteCount = min(byteCount, fileData->Public.Size - fileData->Public.Position);
    }

    while (byteCount > 0) {
        leftInBuffer = SECTOR_SIZE - (fileData->Public.Position % SECTOR_SIZE);
        take = min(byteCount, leftInBuffer);
        
        memcpy(u8OutBuffer, fileData->Buffer + (fileData->Public.Position % SECTOR_SIZE), (uint16_t)take);
        u8OutBuffer += take;
        fileData->Public.Position += take;
        byteCount -= take;
        if (leftInBuffer == take) {
            if (fileData->Public.Handle == ROOT_DIRECTORY_HANDLE) {
                ++fileData->CurrentCluster;

                if (!diskReadSectors(disk, fileData->CurrentCluster, 1, fileData->Buffer)) {
                    printf("Couldn't read next sector of root directory.");
                    return 0;
                }
            }
            else {
                if (++fileData->CurrentSectorInCluster >= b_Data->BS.bootSector.SectorsPerCluster) {
                    fileData->CurrentSectorInCluster = 0;
                    fileData->CurrentCluster = nextCluster(fileData->CurrentCluster);
                }

                if (fileData->CurrentCluster >= 0xFFF8) {
                    fileData->Public.Size = fileData->Public.Position;
                    break;
                }

                if (!diskReadSectors(disk, clusterToLba(fileData->CurrentCluster) + fileData->CurrentSectorInCluster, 1, fileData->Buffer)) {
                    printf("Couldn't read the next sector.");
                    break;
                }
            }
        }
    }

    return u8OutBuffer - (uint8_t*)bufferOut;
}

bool makeDirectory(DISK* disk, const char* parentPath, const char* dirName)
{
    File far* parentFile = open(disk, parentPath);
    DirectoryEntry parentDirEntry;
    DirectoryEntry newDirectory;
    int i;
    int dirNameLength = getLength(dirName); 
    uint32_t newCluster;
    uint16_t far* fat16 = (uint16_t far*)b_Fat;

    if (dirNameLength > 11)
    {
        printf("Directory name can't be more than 11 symbols length\n\r");
        if (parentFile)
            close(parentFile);
        return false;
    }

    printf("Parent path: %s\n\r", parentPath);


    if (!getEntryByPath(disk, parentPath, &parentDirEntry))
    {
        printf("Couldn't open entry\n\r");
        return false;
    }
    printf("ParentEntry is %s\r\n", parentDirEntry.FileName);

    close(parentFile);    

    newCluster = findFreeCluster(disk);

    if (newCluster == 0)
    {
        printf("No clusters available\n\r");
        return false;
    }

    newDirectory.LowFirstClusterNumber = newCluster;
    fat16[newCluster] = 0xFFF8;
    
    // creating FAT name
    for (i = 0; i < 11; i++)
    {
        newDirectory.FileName[i] = ' ';
    }

    for (i = 0; i < dirNameLength; i++)
    {
        newDirectory.FileName[i] = dirName[i];
    }

    newDirectory.Attributes = ATTRIBUTE_DIRECTORY;
    newDirectory.FileSize = 0;

    if (!writeEntryToDirectory(disk, parentDirEntry.LowFirstClusterNumber, &newDirectory))
    {
        return false;
    }

    formatDirectory(disk, newCluster, parentDirEntry.LowFirstClusterNumber);

    if (!diskWriteSectors(disk, b_Data->BS.bootSector.ReservedSectorCount, b_Data->BS.bootSector.SectorsPerFat, b_Fat))
    {
        return false;
    }
    
    if (!diskWriteSectors(disk, b_Data->BS.bootSector.ReservedSectorCount + b_Data->BS.bootSector.SectorsPerFat, b_Data->BS.bootSector.SectorsPerFat, b_Fat))
    {
        return false;
    }

    return true;
}

bool formatDirectory(DISK* disk, uint32_t newCluster, uint32_t parentCluster) {
    uint8_t clusterBuffer[CLUSTER_SIZE];
    DirectoryEntry* dot = (DirectoryEntry*) clusterBuffer;
    DirectoryEntry* dotdot = (DirectoryEntry*)(clusterBuffer + sizeof(DirectoryEntry));
    uint32_t lba; 
    memset(clusterBuffer, 0, CLUSTER_SIZE);

    memcpy(dot->FileName, ".           ", 11);
    dot->FileSize = 0;
    dot->Attributes = ATTRIBUTE_DIRECTORY;
    dot->LowFirstClusterNumber = newCluster;

    memcpy(dotdot->FileName, "..         ", 11);
    dotdot->FileSize = 0;
    dotdot->Attributes = ATTRIBUTE_DIRECTORY;
    dotdot->LowFirstClusterNumber = parentCluster;

    lba = clusterToLba(newCluster);
    diskWriteSectors(disk, lba, b_Data->BS.bootSector.SectorsPerCluster, clusterBuffer);

    return true;
}

bool writeEntryToDirectory(DISK* disk, uint32_t directoryCluster, DirectoryEntry* newDirectoryEntry)
{
    uint32_t lba;
    int dirEntries = CLUSTER_SIZE / sizeof(DirectoryEntry);
    int i;
    uint8_t clusterBuffer[CLUSTER_SIZE];
    DirectoryEntry* entry; 
    uint32_t nextCluster;
    uint32_t newCluster; 
    uint32_t newLba; 

    uint16_t far* fat16 = (uint16_t far*)b_Fat;
    while (true)
    {
        lba = clusterToLba(directoryCluster);
        if (!diskReadSectors(disk, lba, b_Data->BS.bootSector.SectorsPerCluster, clusterBuffer))
        {
            printf("Can't read parent cluster.\n\r");
            return false;
        }

        entry = (DirectoryEntry*)clusterBuffer;

        for (i = 0; i < dirEntries; i++)
        {
            if (entry[i].FileName[0] == 0x00 || (uint8_t)entry[i].FileName[0] == 0xE5)
            {
                memcpy(&entry[i], newDirectoryEntry, sizeof(DirectoryEntry));

                if (diskWriteSectors(disk, lba, b_Data->BS.bootSector.SectorsPerCluster, clusterBuffer))
                {
                    return true;
                }
                else 
                {
                    return false;
                }
            }
        }

        nextCluster = fat16[directoryCluster];

        if (nextCluster >= 0xFFF8)
        {
            newCluster = findFreeCluster(disk);

            if (newCluster == 0)
            {
                printf("Couldn't allocate new cluster.\n\r");
                return false;
            }

            fat16[directoryCluster] = newCluster;
            fat16[newCluster] = 0xFFF8;

            memset(clusterBuffer, 0, CLUSTER_SIZE);

            entry = (DirectoryEntry*)clusterBuffer;
            memcpy(&entry[0], newDirectoryEntry, sizeof(DirectoryEntry));

            newLba = clusterToLba(newCluster);
            
            if (diskWriteSectors(disk, newLba, b_Data->BS.bootSector.SectorsPerCluster, clusterBuffer))
            {
                return true;
            }
            else 
            {
                return false;
            }
        }
        directoryCluster = nextCluster;
    }

    return false;
}

bool readEntry(DISK* disk, File far* file, DirectoryEntry* entry) {
    return read(disk, file, sizeof(DirectoryEntry), entry) == sizeof(DirectoryEntry);
}

void close(File far* file) {
    if (file->Handle == ROOT_DIRECTORY_HANDLE) {
        file->Position = 0;
        b_Data->RootDirectory.CurrentCluster = b_Data->RootDirectory.FirstCluster;
    }
    else {
        b_Data->OpenedFiles[file->Handle].Opened = false;

    }
}

bool findFile(DISK* disk, File far* file, const char* name, DirectoryEntry* dirEntryOut) {
    char fatName[12];
    DirectoryEntry entry;
    const char* extension;
    int i;
    file->Position = 0; 
    // convert from name into fat name

    for (i = 0; i < sizeof(fatName); i++)
        fatName[i] = ' ';

    fatName[11] = '\0';

    for (i = 0; i < 8 && name[i] != '\0' && name[i] != '.'; i++) {
        fatName[i] = toUpper(name[i]);
    }

    extension = strchr(name, '.');
    if (extension != NULL) {
        for (i = 0; i < 3 && extension[i + 1]; i++) {
            fatName[8 + i] = toUpper(extension[i + 1]);
        }
    }

    i = 0;
    
    while (readEntry(disk, file, &entry) && i++ < 5) {
        if (memcmp(fatName, entry.FileName, 11) == 0) {
            *dirEntryOut = entry;
            return true;
        }
    }

    return false;
}

File far* open(DISK* disk, const char* path) {
    bool isLast;
    int length;
    const char* delim;
    File far* current;
    DirectoryEntry entry;
    char name[MAX_PATH_SIZE];

    if (path[0] == '/') {
        path++;
    }
        
    current = &b_Data->RootDirectory.Public;

    while (*path) {
        isLast = false;

        delim = strchr(path, '/');

        if (delim != NULL) {
            memcpy(name, path, delim - path);
            name[delim - path] = '\0';
            path = delim + 1;
        }
        else {
            length = getLength(path);
            memcpy(name, path, length);
            name[length] = '\0';
            path += length;
            isLast = true;
        }

        if (findFile(disk, current, name, &entry)) {
            close(current);

            if (!isLast && entry.Attributes & ATTRIBUTE_DIRECTORY == 0) {
                printf("The %s is not a directory.\n\r", name);
                return NULL;
            }

            current = openEntry(disk, &entry);
        }
        else {
            close(current);

            printf("%s not found\n\r", name);

            return NULL;
        }
    }
    
    return current;
}

bool getEntryByPath(DISK* disk, const char* path, DirectoryEntry* dirEntryOut)
{
    int length = getLength(path);
    int i;
    int index = 0;
    char parentPath[100];
    char name[13];
    File far* file;
    DirectoryEntry* dirEntry;
    if (*path == '\0')
    {
        printf("Can't create directories in root directory\n\r");
        return false;
    }
    i = length - 1;

    while (path[i] != '/' && i >= 0)
    {
        i--;
    }

    index = i;

    if (index == -1) 
    {
        parentPath[0] = '\0'; 
        
        memcpy(name, path, length);
        name[length] = '\0';
    }
    else 
    {
        memcpy(parentPath, path, index);
        parentPath[index] = '\0';

        memcpy(name, path + index + 1, length - index - 1);
        name[length - index - 1] = '\0';
    } 

    file = open(disk, parentPath);
    if (file == NULL)
    {
        printf("Parent dir doesnt exist\n\r");
        return false;
    }
    if (!findFile(disk, file, name, dirEntryOut))
    {
        printf("There is no such an entry\n\r");
        close(file);
        return false;
    }

    close(file);
    return true;
}
