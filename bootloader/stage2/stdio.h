#pragma once
#ifndef NULL
#define NULL ((void*)0)
#endif

void putc(char c);
void puts(const char* str);
void clear(void);
char readKey(void);
void readPrompt(char* buffer);

void _cdecl printf(const char* fmt, ...);
