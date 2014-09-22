#include "OneTimePads.h"

#include <memory.h>
#include <malloc.h>
#include <time.h>
#include "string.h"
#include <stdio.h>

#define N_CHARS 94

char otp_lastError[200];

int otp_generateKey(unsigned int size, otpKey *key)
{
	unsigned int i;
	char r;
	sprintf(otp_lastError, "");

	srand( (unsigned)time( NULL ) );

	*key = 0;

	if( size == 0 ){
		sprintf(otp_lastError, "Unable to generate a key with the size of 0");
		return 0;
	}

	*key = (char*) malloc( sizeof(char)*(size+1) );

	if( ! *key ){
		sprintf(otp_lastError, "Failed to allocate key");
		return 0;
	}

	memset(*key, 0, sizeof(char)*(size+1));

	for( i=0; i< size; i++ )
	{
		r = rand();
		r = (r>=0)?r:-r;
		(*key)[i] = r % N_CHARS;
	}

	(*key)[size] = N_CHARS+1;

	return 1;
}

void otp_destroyKey(otpKey *key)
{
	sprintf(otp_lastError, "");
	if( *key )
		free(*key);

	*key = 0;
}

int otp_encryptString(char** str, otpKey key, unsigned int size)
{
	unsigned int len = strlen(*str);
	unsigned int i;
	int c;

	sprintf(otp_lastError, "");

	if( len== 0 ){
		sprintf(otp_lastError, "Can't encrypt a string with 0 length");
		return 0;
	}

	for( i=0; i< len; i++)
	{
		c = (*str)[i];

		if( c== 0 )
		{
			sprintf(otp_lastError, "Found null in string");
			return 0;
		}

		if( c >= 0x20 && c <= 0x7E )
		{
			c -= 0x20;
		}
		else
		{
			sprintf(otp_lastError, "Unvalid sign found in string");
			return 0;
		}

		c = ( c + key[i%size] ) % N_CHARS;

		 if(c >= 94)
		 {
			 sprintf(otp_lastError, "Modulus error");
			 return 0;
		 }
		 else
		 {
			  c = c+0x20;
		 }

		 (*str)[i] = c;
	}

	return 1;
}

int otp_decryptString(char** str, otpKey key, unsigned int size)
{
	unsigned int len = strlen(*str);
	unsigned int i;
	int c;

	sprintf(otp_lastError, "");

	if( len== 0 ){
		sprintf(otp_lastError, "Input string has a length of zero");
		return 0;
	}

	for( i=0; i< len; i++)
	{
		c = (*str)[i];

		if( c== 0 )
		{
			sprintf(otp_lastError, "Found null in input string");
			return 0;
		}

		if( c >= 0x20 && c <= 0x7E )
		{
			c -= 0x20;
		}
		else
		{
			sprintf(otp_lastError, "Found a non-valid sign in inputstring");
			return 0;
		}

		c = ( c - key[i%size] ) % N_CHARS;

		if( c<0 ) c+= N_CHARS;

		 if(c >= 94)
		 {
			 sprintf(otp_lastError, "Modolus error");
			 return 0;
		 }
		 else
		 {
			  c = c+0x20;
		 }

		 (*str)[i] = c;
	}

	return 1;
}

void otp_printKey(otpKey key)
{
	char pos=0;

	while(key[pos] != N_CHARS+1)
	{
		printf("%c", key[pos]+0x20);
		pos++;
	}
}

int otp_initializeKey(char* baseKey, otpKey *key, int* externSize)
{
	unsigned int i;
	unsigned int size;
	char t;
	sprintf(otp_lastError, "");

	*key = 0;

	if( baseKey == 0 ){
		sprintf(otp_lastError, "Basekey is null");
		return 0;
	}

	size = strlen(baseKey);
	if( size == 0 ) return 0;

	*key = (char*) malloc( sizeof(char)*(size+1) );

	if( ! *key ){
		sprintf(otp_lastError, "Failed to allocate key");
		return 0;
	}

	memset(*key, 0, sizeof(char)*(size+1));

	for( i=0; i< size; i++ )
	{
		t = baseKey[i];
		if( t < 0x20 ) return 0;
		t-= 0x20;
		if( t> N_CHARS ) return 0;
		(*key)[i] = t;
	}

	(*key)[size] = N_CHARS+1;

	if( externSize )
		*externSize = size;

	return 1;
}

char* otp_getLastError(){
	return otp_lastError;
}