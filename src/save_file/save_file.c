#include "save_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "OneTimePads.h"
#include "md5.h"

#define DECL_STAMP(name) char name[16]
#define STAMP_CMP(stamp1, stamp2) memcmp(stamp1, stamp2, 16)
#define STAMP_SIZE	16

//#define DEBUG_PRINT

struct SSaveFile{
	FILE* file;
	int saving;
	otpKey key;
	int keySize;
	md5_state_t state;
	unsigned long bytesWritten;
};

#define DECL_ERRORSTRING(name)	char name[200]

DECL_ERRORSTRING(lastErrorDetected);

/*
int main () {
  FILE * pFile;
  long lSize;
  char * buffer;

  pFile = fopen ( "myfile.txt" , "rb" );
  if (pFile==NULL) exit (1);

  // obtain file size.
  fseek (pFile , 0 , SEEK_END);
  lSize = ftell (pFile);
  rewind (pFile);

  // allocate memory to contain the whole file.
  buffer = (char*) malloc (lSize);
  if (buffer == NULL) exit (2);

  // copy the file into the buffer.
  fread (buffer,1,lSize,pFile);

  // the whole file is loaded in the buffer

  // terminate
  fclose (pFile);
  free (buffer);
  return 0;
}
*/

int md5checkFile(FILE* file){
	md5_state_t state;
	long fileSize = 0L;
	long bufferSize = 0L;
	char* buffer=0;
	int res=0;
	DECL_STAMP(fileStamp);
	DECL_STAMP(calcStamp);
	
	rewind(file); // begining
	res = fseek (file , 0 , SEEK_END);

	if( res != 0 ){
		sprintf(lastErrorDetected, "Failed to find the end of the file");
		return 0;
	}
	fileSize = ftell(file);
	if( fileSize == -1L && errno > 0){
		sprintf(lastErrorDetected, "Failed to find out the file size");
		return 0;
	}
	rewind(file);

	bufferSize = fileSize - STAMP_SIZE;
	buffer = 0;
	if( bufferSize <= 0 ){
		sprintf(lastErrorDetected, "File not large enough");
		return 0;
	}
	buffer = malloc(bufferSize);
	if(! buffer ){
		sprintf(lastErrorDetected, "Failed to allocate the file buffer");
		return 0;
	}
	res = fread(buffer, bufferSize, 1, file);
	if( res != 1 ){
		sprintf(lastErrorDetected, "Failed to read buffer");
		return 0;
	}
	res = fread(fileStamp, STAMP_SIZE, 1, file);
	if( res != 1 ){
		sprintf(lastErrorDetected, "Failed to read the md5 digest");
		return 0;
	}
	
	md5_init(&state);
	md5_append(&state, buffer, bufferSize );
	md5_finish(&state, calcStamp);
	free(buffer); buffer = 0;

	if( STAMP_CMP(calcStamp, fileStamp)!=0 ){
		sprintf(lastErrorDetected, "md5 check failed");
		return 0;
	}

	rewind(file);
	return 1;
}

SaveFile* sf_open(char* fileName, char* application, unsigned int saveVersion, char* key, int save){
	SaveFile *sf = 0;
	int res=0;
	char string[200];
	unsigned int i = 0;

	sprintf(lastErrorDetected, "");
	sf = malloc(sizeof(SaveFile) );
	if(! sf ){
		sprintf(lastErrorDetected, "Failed to allocate SaveFile structure");
		return 0;
	}
	memset(sf, 0, sizeof(SaveFile) );

	sf->bytesWritten = 0;
	sf->saving = save;

	sf->file = 0;
	if( save )	sf->file = fopen(fileName, "wb");
	else		sf->file = fopen(fileName, "rb");

	if(! sf->file ){
		sprintf(lastErrorDetected, "Failed to open file");
		free(sf);
		return 0;
	}
	// init file
	md5_init(&(sf->state));


	if(! sf->saving )
	{
		res = md5checkFile(sf->file); // this subroutine is responsible for reporting errormessage(if any occured)
		if( !res ){
			fclose(sf->file);
			free(sf);
			return 0;
		}
	}

	res = otp_initializeKey(key, &(sf->key), &(sf->keySize) );
	if( !res){
		fclose(sf->file);
		free(sf);
		sprintf(lastErrorDetected, "Failed load key");
		return 0;
	}

	sprintf(string, "%s", application);
	i = saveVersion;

	res = sf_handleString(sf, string);
	if( !res){
		DECL_ERRORSTRING(temp);
		strcpy(temp, lastErrorDetected);
		fclose(sf->file);
		free(sf);
		sprintf(lastErrorDetected, "Failed to handle applicationame (%s)", temp);
		return 0;
	}

	if( strcmp(string, application) != 0 ){
		fclose(sf->file);
		free(sf);
		sprintf(lastErrorDetected, "Bad application name %s", string);
		return 0;
	}

	res = sf_handleInteger(sf, &i);
	if( !res){
		DECL_ERRORSTRING(temp);
		strcpy(temp, lastErrorDetected);
		fclose(sf->file);
		free(sf);
		sprintf(lastErrorDetected, "Failed to handle version (%s)", temp);
		return 0;
	}

	if( i != saveVersion ){
		fclose(sf->file);
		free(sf);
		sprintf(lastErrorDetected, "Bad application version %i", i);
		return 0;
	}

	return sf;
}

int sf_handleInteger(SaveFile* file, int* i){
	unsigned int bytes=0;
	sprintf(lastErrorDetected, "");
	if(! file ){
		sprintf(lastErrorDetected, "file input is null");
		return 0;
	}

	if( i == 0 ){
		sprintf(lastErrorDetected, "Integer is null");
		return 0;
	}


	if( file->saving ){
		bytes = fwrite (i, sizeof(int) , 1, file->file);
		md5_append(&file->state, (unsigned char*)i, sizeof(int) );
		file->bytesWritten += sizeof(int);
	}
	else {
		bytes = fread (i, sizeof(int) , 1, file->file);
	}

	if( bytes != 1 ){
		sprintf(lastErrorDetected, "Failed to handle reading/saving from/to file");
		return 0;
	}

	return 1;
}

int sf_handleUchar(SaveFile* file, unsigned char* uc){
	unsigned int bytes=0;
	sprintf(lastErrorDetected, "");
	if(! file ){
		sprintf(lastErrorDetected, "file input is null");
		return 0;
	}

	if( uc == 0 ){
		sprintf(lastErrorDetected, "Unsigned char is null");
		return 0;
	}


	if( file->saving ){
		bytes = fwrite (uc, sizeof(unsigned char) , 1, file->file);
		md5_append(&file->state, uc, sizeof(unsigned char) );
		file->bytesWritten += sizeof(unsigned char);
	}
	else {
		bytes = fread (uc, sizeof(unsigned char) , 1, file->file);
	}

	if( bytes != 1 ){
		sprintf(lastErrorDetected, "Failed to handle reading/saving from/to file");
		return 0;
	}

	return 1;
}

#define CLEAR_STRING(str) if( str )free(str); str = 0;

int sf_handleString(SaveFile* file, char* string){
	unsigned int bytes=0;
	sprintf(lastErrorDetected, "");
	if(! file ){
		sprintf(lastErrorDetected, "File input is null");
		return 0;
	}

	if( string == 0 ){
		sprintf(lastErrorDetected, "String is null");
		return 0;
	}

	if( file->saving ){
		unsigned long size = strlen(string);
		char* encryptedString =0;
		int res = 0;
		encryptedString = malloc(sizeof(char)*size +1);
		if(! encryptedString ) {
			sprintf(lastErrorDetected, "Failed to allocate temporary string");
			return 0;
		}
		memset(encryptedString, 0, sizeof(char)*size+1);
		strcpy(encryptedString, string);

		res = otp_encryptString(&encryptedString, file->key, file->keySize);
		if(! res ){
			CLEAR_STRING(encryptedString);
			sprintf(lastErrorDetected, "Failed to encrypt string, OTP reason: %s", otp_getLastError());
			return 0;
		}

		bytes = fwrite (&size, sizeof(unsigned long), 1, file->file);
		if( bytes != 1 ){
			CLEAR_STRING(encryptedString);
			sprintf(lastErrorDetected, "Failed to write string length");
			return 0;
		}
		md5_append(&file->state, (unsigned char*)&size, sizeof(unsigned long) );
		file->bytesWritten += sizeof(unsigned long);

		bytes = fwrite (encryptedString, sizeof(char), size, file->file);
		if( bytes != size ){
			CLEAR_STRING(encryptedString);
			sprintf(lastErrorDetected, "Failed to write whole string (%i of %i)", bytes, size);
			return 0;
		}
		md5_append(&file->state, encryptedString, size );
		file->bytesWritten += size;

		CLEAR_STRING(encryptedString);
	} else {
		unsigned long size = 0;
		int res = 0;
		//unsigned int inputsize = 0;

		bytes = fread (&size, sizeof(unsigned long), 1, file->file);
		if( bytes != 1 ){
			sprintf(lastErrorDetected, "Failed to read string length");
			return 0;
		}
		// check length of the input string length
		/*
		inputsize = sizeof(string); // this only gives the length of the current assigned data, and not the total sizeof the string

		if( inputsize < size+1 ){
			sprintf(lastErrorDetected, "The length of the input string needs to be atleast %i", size+1);
			return 0;
		}*/

		memset(string, 0, sizeof(char)*size + 1 );
		bytes = fread(string, sizeof(char), size, file->file);

		if( bytes != size ){
			sprintf(lastErrorDetected, "Failed to read whole string (%i of %i)", bytes, size);
			return 0;
		}

		res = otp_decryptString(&string, file->key, file->keySize);
		if(! res ){
			sprintf(lastErrorDetected, "Failed to decrypt string, OTP reason: %s", otp_getLastError());
			return 0;
		}
	}
	
	return 1;
}

int sf_handleFloat(SaveFile* file, float* f){
	unsigned int bytes = 0;
	sprintf(lastErrorDetected, "");
	if(! file ){
		sprintf(lastErrorDetected, "file input is null");
		return 0;
	}

	if( f == 0 ){
		sprintf(lastErrorDetected, "Float is null");
		return 0;
	}

	if( file->saving ){
		bytes = fwrite (f, sizeof(float) , 1, file->file);
		md5_append(&file->state, (unsigned char*) f, sizeof(float) );
		file->bytesWritten += sizeof(float);
	} else {
		bytes = fread (f, sizeof(float) , 1, file->file);
	}

	if( bytes != 1 ){
		sprintf(lastErrorDetected, "Failed to write/read to/from file");
		return 0;
	}

	return 1;
}

#ifdef DEBUG_PRINT
void printmd5(char* input, char* md5stamp){
	int i=0;

	printf("md5(%s): ", input);
	for(i=0; i<15; i++){
		printf("%d.", md5stamp[i] +500 );
	}
	printf("%d\n", md5stamp[15] + 500 );
}
#endif

int sf_closeFile(SaveFile* file){
	sprintf(lastErrorDetected, "");

	if( file->saving ){
		DECL_STAMP(stamp);
		md5_finish(&file->state, stamp);
#ifdef DEBUG_PRINT
		printmd5("", stamp);
		printf("Bytes written(without stamp): %i\n", file->bytesWritten );
		printf("Bytes written(with stamp): %i\n", file->bytesWritten + STAMP_SIZE);
#endif
		fwrite(stamp, STAMP_SIZE, 1, file->file);
	}

	if(! file ){
		sprintf(lastErrorDetected, "file input is null");
		return 0;
	}
	if( file->file ){
		fclose( file->file );
		file->file = 0;
	}

	if( file->key ){
		otp_destroyKey(&file->key);
		file->key = 0;
	}

	free(file);

	return 1;
}

char* sf_getLastError(){
	return lastErrorDetected;
}
