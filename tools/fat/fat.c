#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef uint8_t bool;
#define false 0
#define true 1

typedef struct {
    uint8_t BootJumpInstruction[3];
    uint8_t OemName[8];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectorCount;
    uint8_t TableCount;
    uint16_t RootEntryCount;
    uint16_t TotalSectors16;
    uint8_t MediaType;
    uint16_t TableSize16;
    uint16_t SectorsPerTrack;
    uint16_t HeadSideCount;
    uint32_t HiddenSectorCount;
    uint32_t TotalSectors32;

    // extended boot sector
    uint8_t DriveNumber;
    uint8_t Reserved;
    uint8_t Signature;
    uint32_t VolumeId;
    uint8_t VolumeLabel[11];
    uint8_t SystemId[8];
} __attribute__((packed)) BootSector;

typedef struct {
    uint8_t FileName[11];
    uint8_t Attributes;
    uint8_t Reserved;
    uint16_t CreationTime;
    uint16_t CreationDate;
    uint16_t LastAccessedDate;
    uint16_t HighFirstClusterNumber;    // for our system is always zero
    uint16_t LastModificationTime;
    uint16_t LastModificationDate;
    uint16_t LowFirstClusterNumber;
    uint32_t FileSize;
} __attribute__((packed)) DirectoryEntry;

BootSector bootSector;
uint8_t* fat = NULL;
DirectoryEntry* rootDirectory = NULL;
uint32_t rootDirectoryEnd;

bool readBootSector(FILE* disk)
{
    return fread(&bootSector, sizeof(BootSector), 1, disk) > 0;
}

bool readSectors(FILE* disk, uint32_t lba, uint32_t count, void* bufferOut)
{
    bool ok = true;
    ok = ok && (fseek(disk, lba * bootSector.BytesPerSector, SEEK_SET) == 0);
    ok = ok && (fread(bufferOut, bootSector.BytesPerSector, count, disk) == count);
    return ok;
}

bool readFat(FILE* disk)
{
    fat = (uint8_t*)malloc(bootSector.BytesPerSector * bootSector.TableSize16);
    return readSectors(disk, bootSector.ReservedSectorCount, bootSector.TableSize16, fat);
}

bool readRootDirectory(FILE* disk)
{
    uint32_t lba = (bootSector.ReservedSectorCount + (bootSector.TableCount * bootSector.TableSize16));
    uint32_t size = sizeof(DirectoryEntry) * bootSector.RootEntryCount;
    uint32_t sectors = size / bootSector.BytesPerSector;
    if (size % bootSector.BytesPerSector != 0)
        sectors++;

    rootDirectoryEnd = lba + sectors; // at which sector root directory ends
    return readSectors(disk, lba, sectors, rootDirectory);
}

DirectoryEntry* findFile(const char* name)
{
    for (int i = 0; i < bootSector.RootEntryCount; i++)
    {
        if (memcmp(name, rootDirectory[i].FileName, 11))
            return &rootDirectory[i];
    }

    return NULL;
}

bool readFile(DirectoryEntry* file, FILE* disk, uint8_t* bufferOut)
{
    bool ok = true;
    uint16_t clusterNumber = file -> LowFirstClusterNumber;

    uint32_t DataStartOffset = rootDirectoryEnd * bootSector.BytesPerSector;
    uint32_t clusterOffset = DataStartOffset + ((clusterNumber - 2) * bootSector.SectorsPerCluster * bootSector.BytesPerSector);

    do {
        uint32_t lba = (DataStartOffset + ((clusterNumber - 2) * bootSector.SectorsPerCluster * bootSector.BytesPerSector)) / bootSector.BytesPerSector;
        readSectors(disk, lba, bootSector.SectorsPerCluster, bufferOut);

        bufferOut += bootSector.BytesPerSector * bootSector.SectorsPerCluster;

        clusterNumber = (*(uint16_t*)(fat + 2 * clusterNumber));
    } while (ok && clusterNumber < 0xFFF8);

    return ok;
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        printf("Syntax %s <disk image> <file name>\n", argv[0]);
        return -1;
    }

    FILE* disk = fopen(argv[1], "rb");
    if (!disk) {
        fprintf(stderr, "Cannot open disk image!\n");
        return -2;
    }

    if (!readBootSector(disk)) {
        fprintf(stderr, "Could not read boot sector!\n");
        return -3;
    }

    if (!readFat(disk)) {
        fprintf(stderr, "Could not read FAT!\n");
        free(fat);
        return -4;
    }

    if (!readRootDirectory(disk)) {
        fprintf(stderr, "Could not read root directory!\n");
        free(fat);
        free(rootDirectory);
        return -5;
    }

    DirectoryEntry* fileEntry = findFile(argv[2]);
    if (!fileEntry) {
        fprintf(stderr, "Couldn't find file %s!", argv[2]);
        free(fat);
        free(rootDirectory);
        return -6;
    }

    uint8_t* buffer = (uint8_t*)malloc(fileEntry->FileSize + bootSector.SectorsPerCluster * bootSector.BytesPerSector);
    if (!readFile(fileEntry, disk, buffer)) {
        fprintf(stderr, "Couldn't find file %s!", argv[2]);
        free(fat);
        free(rootDirectory);
        free(buffer);
        return -6;
    }

    for (size_t i = 0; i < fileEntry -> FileSize; i++) {
        if (isprint(buffer[i]))
            fputc(buffer[i], stdout);
        else
            printf("<%02x>", buffer[i]);
    }

    printf("\n");

    free(buffer);
    free(rootDirectory);
    free(fat);

    return 0;
}