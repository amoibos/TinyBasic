#ifndef STRINGS_H
#define STRINGS_H

void itoa(int value, char *sp, int radix);
int atoi(char* str);
float atof(char* arr);
char* ftoa(float value, int decimals, char* buf);


void strcat(char* str1, char* str2);
char strcmp(char* str1, char* str2);
void strncpy(char* dest, char* src, int n); 
char strlen(char* str);
char strpos(char* search, char* content, char start);
char* strcpy(char* dst, char* src);
char* strchr(const char *s, char ch);

char upcase(char c);
char downcase(char c);
char isalpha(const char mark);
char* to_upper(char* string);
char isdigit(const char ch);

#endif