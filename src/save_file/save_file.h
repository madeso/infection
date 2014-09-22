#ifndef __SAVE_FILE_H
#define __SAVE_FILE_H

typedef struct SSaveFile SaveFile;
SaveFile* sf_open(const char* fileName, char* application, unsigned int saveVersion, char* key, int save);
int sf_handleInteger(SaveFile* file, int* i);
int sf_handleString(SaveFile* file, char* string);
int sf_handleFloat(SaveFile* file, float* f);
int sf_handleUchar(SaveFile* file, unsigned char* f);
int sf_closeFile(SaveFile* file);
char* sf_getLastError();

#define FLOAT(value, message)	if(! sf_handleFloat(file, &value) )		{cerrorx(message, sf_getLastError()); return 0;}
#define STRING(value, message)	if(! sf_handleString(file, value) )		{cerrorx(message, sf_getLastError()); return 0;}
#define INT(value, message)		if(! sf_handleInteger(file, &value) )	{cerrorx(message, sf_getLastError()); return 0;}
#define UCHAR(value, message)	if(! sf_handleUchar(file, &value) )		{cerrorx(message, sf_getLastError()); return 0;}

#define VECTOR(value, message)		if(! handle_Vector(file, &value) )	{error(message); return 0;}
#define VECTORPTR(value, message)		if(! handle_Vector(file, value) )	{error(message); return 0;}

#endif