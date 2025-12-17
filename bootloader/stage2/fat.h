

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

typedef struct {
    int Handle;
    bool IsDirectory;
    uint32_t Position;
    uint32_t Size;
} File;

enum Attributes {
    ATTRIBUTE_READ_ONLY = 0x01,
    ATTRIBUTE_HIDDEN    = 0x01,
    ATTRIBUTE_SYSTEM    = 0x04,
    ATTRIBUTE_VOLUME_ID = 0x08,
    ATTRIBUTE_DIRECTORY = 0x10,
    ATTRIBUTE_ARCHIVE   = 0x20,
    ATTRIBUTE_LFN       = ATTRIBUTE_READ_ONLY | ATTRIBUTE_HIDDEN | ATTRIBUTE_SYSTEM | ATTRIBUTE_VOLUME_ID
};

bool initialize(DISK* disk);
File* open(DISK* disk, const char* path);
uint32_t read(DISK* disk, File* file, uint32_t count, void* outData);
bool readEntry(DISK* disk, FILE* file, DirectoryEntry* entry);
void close(FILE* file);