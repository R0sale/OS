#include "disk.h"
#include "x86.h"
#include "stdio.h"

bool diskInitialize(DISK* disk, uint8_t driveNumber) {
    uint16_t sectors, cylinders, heads;

    if (!x86_Disk_Get_Drive_Params(driveNumber, &sectors, &cylinders, &heads))
        return false;

    disk->id = driveNumber;
    disk->cylinders = cylinders + 1;
    disk->heads = heads + 1;
    disk->sectors = sectors;

    return true;
}

void lbaToChs(DISK* disk, uint32_t lba, uint16_t* sectors, uint16_t* cylinders, uint16_t* heads) {
    *sectors = (uint16_t)lba % disk->sectors + 1;
    *heads = (uint16_t)lba / disk->sectors % disk->heads;
    *cylinders = (uint16_t)lba / disk->sectors / disk->heads;
}

bool diskReadSectors(DISK* disk, uint32_t lba, uint8_t count, void far* outData) {
    uint16_t sectors, cylinders, heads;
    int i;

    lbaToChs(disk, lba, &sectors, &cylinders, &heads);

    for (i = 0; i < 3; i++) {
        if(x86_Disk_Read(disk->id, sectors, cylinders, heads, count, outData))
            return true;

        x86_Disk_Reset(disk->id);
    }

    return false;
}   
