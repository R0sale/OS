#pragma once

typedef char bool;
#define true 1
#define false 0

bool strcmp(const char* str1, const char* str2);
void trim(char* str);
int getLength(char* str);
const char* strchr(const char* str, char chr);
char* get_token(char** current_pos, char separator);
void formatDisplayString(char* initialString, char* newString);
