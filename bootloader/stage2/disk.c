#include "disk.h"
#include "x86.h"

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
