#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "disk.h"
#include "memdefs.h"
#include "fat.h"
#include "memory.h"
#include "ctype.h"

#define SECTOR_SIZE 512
#define MAX_PATH_SIZE 256
#define MAX_FILES_HANDLE 10
#define ROOT_DIRECTORY_HANDLE -1

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

static FAT_Data b_ActualData;
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

bool readFat(DISK* disk)
{
    return diskReadSectors(disk, b_Data->BS.bootSector.ReservedSectorCount, b_Data->BS.bootSector.SectorsPerFat, b_Fat);
}

bool fatInitialize(DISK* disk) {
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

    rootDirSectors = (rootDirSize + b_Data->BS.bootSector.BytesPerSector - 1) / b_Data->BS.bootSector.BytesPerSector;
    b_DataSectionLba = rootDirLba + rootDirSectors;

    for (i = 0; i < MAX_FILES_HANDLE; i++) {
        b_Data->OpenedFiles[i].Opened = false;
    }

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
        return false;
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
    // convert from name into fat name

    for (i = 0; i < sizeof(fatName); i++)
        fatName[i] = ' ';

    fatName[11] = '\0';

    extension = strchr(name, '.');
    if (extension == NULL) {
        extension = name + 11;
    }

    for (i = 0; i < 8 && name[i] != '\0' && (name + i < extension); i++) {
        fatName[i] = toUpper(name[i]);
    }

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
            path += length;
            isLast = true;
        }

        if (findFile(disk, current, name, &entry)) {
            close(current);

            if (!isLast && entry.Attributes & ATTRIBUTE_DIRECTORY == 0) {
                printf("The %s is not a directory.", name);
                return NULL;
            }

            current = openEntry(disk, &entry);
        }
        else {
            close(current);

            printf("%s not found", name);

            return NULL;
        }
    }
    
    return current;
}
