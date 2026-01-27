#pragma once
#include "stdint.h"

void far* memcpy(void far* dst, const void far* src, uint16_t num);
void* memset(void* ptr, int value, uint16_t num);
int memcmp(const void far* ptr1, const void far* ptr2, uint16_t num);
void fillSpaces(char* ptr, uint16_t num);
