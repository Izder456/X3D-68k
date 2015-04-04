// C Source File
// Created 3/9/2015; 11:23:36 AM

#include <tigcclib.h>

#include "link.h"

#ifndef NDEBUG

// Throws an error, prints out the message, and then quits the program
void error(const char* format, ...) {
	char buf[512];
	va_list list;
	
	cleanup_link();
	
	va_start(list, format);
	vsprintf(buf, format, list);
	
	PortRestore();
	clrscr();
	printf("Error: %s\nPress Esc to quit\n", buf);
	
	while(!_keytest(RR_ESC)) ;
	
	exit(-1);
}

#endif