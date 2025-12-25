#pragma once
#include "stdint.h"

void _cdecl x86_Video_Write(char c, uint8_t page);
void _cdecl x86_Video_Clear(void);
char _cdecl x86_Video_Read_Char(void);
void _cdecl x86_Shutdown(void);
void _cdecl x86_Move_Cursor(int xOffset, int yOffset);
void _cdecl x86_div64_32(uint64_t divident, uint32_t divisor, uint64_t* quotioent, uint32_t* remainder);

bool _cdecl x86_Disk_Get_Drive_Params(uint8_t driveNumber,
                                      uint16_t* sectors,
                                      uint16_t* cylinders,
                                      uint16_t* heads);
bool _cdecl x86_Disk_Reset(uint8_t driveNumber);
bool _cdecl x86_Disk_Read(uint8_t driveNumber, uint16_t sectors, uint16_t cylinders, uint16_t heads, uint8_t count, void far* buffer);
