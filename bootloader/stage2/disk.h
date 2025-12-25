#pragma once
#include "stdint.h"

typedef unsigned char bool;

typedef struct {
    uint8_t id;
    uint16_t sectors;
    uint16_t cylinders;
    uint16_t heads;
} DISK;

bool diskInitialize(DISK* disk, uint8_t driveNumber);
bool diskReadSectors(DISK* disk, uint32_t lba, uint8_t count, void far* outData);