#ifndef ONE_TIME_PADS_H
#define ONE_TIME_PADS_H

#include "stdlib.h"

typedef char* otpKey;
int otp_generateKey(unsigned int size, otpKey *key);
int otp_initializeKey(char* baseKey, otpKey *key, int* externSize);
void otp_destroyKey(otpKey *key);
void otp_printKey(otpKey key);
int otp_encryptString(char** str, otpKey key, unsigned int size);
int otp_decryptString(char** str, otpKey key, unsigned int size);
char* otp_getLastError();

#endif