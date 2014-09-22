/*
log.h
Logging is good, here is functions that takes care of that.
*/

#ifndef LOG_H
#define LOG_H

#include "useful_macros.h"

char str[STRING_LENGTH];

void startLog();//Initiate the log file
void printLog(const char *c);//print to log file
void printExtended(const char* sz);// calls printLog if debug is aviable
void error(const char* info);// reports an error
void errorx(const char* info, const char* reason);// reports an error

void cerror(const char* info);// reports an error
void cerrorx(const char* info, const char* reason);// reports an error

#endif