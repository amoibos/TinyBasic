#include "console.h"

#include "../SEGA/SMSlib.h"

int _SCREEN_POS_X = 0;
int _SCREEN_POS_Y = 0;

char readkey(void) 
{
	return 0;	
}

void delay(int duration) 
{
	/*
	wait
	*/
	for (int i=0; i < duration; ++i) ;	
}

void gotoxy(char x, char y) 
{
	
	_SCREEN_POS_X = x;
	_SCREEN_POS_Y = y;
	SMS_setNextTileatXY(_SCREEN_POS_X, _SCREEN_POS_Y);
}

void putc(char ch)
{
	//set tile in tilemap
	
	gotoxy(_SCREEN_POS_X, _SCREEN_POS_Y);
	SMS_setTile(ch);
}

void scroll()
{

}

void putchar(char ch)
{
	if (ch == '\n') 
	{
		_SCREEN_POS_X=-1;
		++_SCREEN_POS_Y;	
	}
	else
		putc(ch);
	
	++_SCREEN_POS_X;
	// last line
	if (_SCREEN_POS_X  == SCR_MAX_X)
	{
		
		// last line
		if(_SCREEN_POS_Y == SCR_MAX_Y)
			scroll();
		else
			++_SCREEN_POS_Y;
		gotoxy(0, _SCREEN_POS_Y);
	}	
}

void clreol(void)
{
	gotoxy(0, _SCREEN_POS_Y);
	for(int i=0;  i < SCR_MAX_X; ++i)
		putchar(' ');
}

void clrscr(void)
{
	for(int _SCREEN_POS_Y=0; _SCREEN_POS_Y < SCR_MAX_Y; ++_SCREEN_POS_Y)
	{
		gotoxy(0, _SCREEN_POS_Y);
		clreol();
	}
	_SCREEN_POS_X = 0;
	_SCREEN_POS_Y = 0;
}

char wherex(void)
{
	return _SCREEN_POS_X;
}

char wherey(void) 
{
	return _SCREEN_POS_Y;	
}

void beep(void)
{
	/* not implemented*/
}

void puts(char* str)
{
	for(; *str!=0; ++str)
		putchar(*str);
}

void gets(char *input)
{
	*input=0;
}