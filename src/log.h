/*
log.h
Logging is good, here is functions that takes care of that.
*/

#ifndef LOG_H
#define LOG_H

#include "useful_macros.h"

char str[STRING_LENGTH];

void startLog();//Initiate the log file
void printLog(char *c);//print to log file
void printExtended(char* sz);// calls printLog if debug is aviable
void error(char* info);// reports an error
void errorx(char* info, char* reason);// reports an error

void cerror(char* info);// reports an error
void cerrorx(char* info, char* reason);// reports an error

#endif