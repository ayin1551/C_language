/*
 * Error handling routines
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "error.h"

int errors;
int warnings;
int dbflag = 1;

void fatal(char* fmt, ...)
{
        va_list args;
        va_start(args,fmt);
        vfprintf(stderr,fmt,args);
        va_end(args);       
        exit(1);
}

void error(char* fmt, ...)
{
        va_list args;
        va_start(args,fmt);
        vfprintf(stderr,fmt,args);
        va_end(args);
        errors++;
}

void warning(char* fmt, ...)
{
        va_list args;
        va_start(args,fmt);
        vfprintf(stderr,fmt,args);
        va_end(args);
        warnings++;
}

void debug(char* fmt, ...)
{
        if(!dbflag) return;
        va_list args;
        va_start(args,fmt);
        vfprintf(stderr,fmt,args);
        va_end(args);
}
