#pragma once
#include "stdint.h"

void _cdecl x86_Video_Write(char c, uint8_t page);
void _cdecl x86_Video_Clear(void);
char _cdecl x86_Video_Read_Char(void);
void _cdecl x86_Shutdown(void);
