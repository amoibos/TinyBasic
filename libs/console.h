#ifndef CONSOLE_H
#define CONSOLE_H

#define SMS


//screen limitations
#ifdef SG1000
#define SCR_MAX_X  ((256 / 8))
#define SCR_MAX_Y ((192 / 8))
#endif

#ifdef SMS
#define SCR_MAX_X  ((256 / 8))
// DEVKITSMS supports only SMS1-VDPs
#define SCR_MAX_Y ((192 / 8))
#endif





#define  SCR_BOTTOM_LINE ((MAX_Y)) 

//int G_x;
//int G_y;

#define KB_SPACE ((32))

int random(void);

void delay(int duration);

void clreol(void);
void clrscr(void);
char wherex(void);
char wherey(void);

void gotoxy(char x, char y);
char readkey(void);
void beep(void);

void putchar(char ch);
void puts(const char* str);
void gets(char *input);

#endif