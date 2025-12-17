// #include "stdio.h"
// #include "stdint.h"
// #include "string.h"

// #define SECTOR_SIZE 512
// #define MAX_PATH_SIZE 256
// #define ROOT_DIRECTORY_ENTRY -1

// BootSector bootSector;
// uint8_t* fat = NULL;
// DirectoryEntry* rootDirectory = NULL;
// uint32_t rootDirectoryEnd;

// bool readBootSector(FILE* disk)
// {
//     return fread(&bootSector, sizeof(BootSector), 1, disk) > 0;
// }

// bool readSectors(FILE* disk, uint32_t lba, uint32_t count, void* bufferOut)
// {
//     bool ok = true;
//     ok = ok && (fseek(disk, lba * bootSector.BytesPerSector, SEEK_SET) == 0);
//     ok = ok && (fread(bufferOut, bootSector.BytesPerSector, count, disk) == count);
//     return ok;
// }

// bool readFat(FILE* disk)
// {
//     fat = (uint8_t*)malloc(bootSector.BytesPerSector * bootSector.TableSize16);
//     return readSectors(disk, bootSector.ReservedSectorCount, bootSector.TableSize16, fat);
// }

// bool readRootDirectory(FILE* disk)
// {
//     uint32_t lba = (bootSector.ReservedSectorCount + (bootSector.TableCount * bootSector.TableSize16));
//     uint32_t size = sizeof(DirectoryEntry) * bootSector.RootEntryCount;
//     uint32_t sectors = size / bootSector.BytesPerSector;
//     if (size % bootSector.BytesPerSector != 0)
//         sectors++;

//     rootDirectoryEnd = lba + sectors; // at which sector root directory ends
//     return readSectors(disk, lba, sectors, rootDirectory);
// }

// DirectoryEntry* findFile(const char* name)
// {
//     for (int i = 0; i < bootSector.RootEntryCount; i++)
//     {
//         if (memcmp(name, rootDirectory[i].FileName, 11))
//             return &rootDirectory[i];
//     }

//     return NULL;
// }

// bool readFile(DirectoryEntry* file, FILE* disk, uint8_t* bufferOut)
// {
//     bool ok = true;
//     uint16_t clusterNumber = file -> LowFirstClusterNumber;

//     uint32_t DataStartOffset = rootDirectoryEnd * bootSector.BytesPerSector;
//     uint32_t clusterOffset = DataStartOffset + ((clusterNumber - 2) * bootSector.SectorsPerCluster * bootSector.BytesPerSector);

//     do {
//         uint32_t lba = (DataStartOffset + ((clusterNumber - 2) * bootSector.SectorsPerCluster * bootSector.BytesPerSector)) / bootSector.BytesPerSector;
//         readSectors(disk, lba, bootSector.SectorsPerCluster, bufferOut);

//         bufferOut += bootSector.BytesPerSector * bootSector.SectorsPerCluster;

//         clusterNumber = (*(uint16_t*)(fat + 2 * clusterNumber));
//     } while (ok && clusterNumber < 0xFFF8);

//     return ok;
// }