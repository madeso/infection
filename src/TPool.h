/****************************************************************************************/
/*  TPool.h																				*/
/****************************************************************************************/
#ifndef TPOOL_H
#define TPOOL_H

#include "genesis.h"

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct	TPool TPool;

typedef struct	TPool
{
  TPool    *next;
  TPool    *prev;
  geBitmap *Bitmap;
  char     *BmpName;
  char     *AlphaName;
} TPool;

geBitmap *TPool_Bitmap(char *DefaultBmp, char *DefaultAlpha, char *BName, char *AName, geWorld *World);
void TPool_Initalize();
void TPool_Delete(geWorld *World);



#ifdef __cplusplus
	}
#endif
#pragma warning ( default : 4068 )

#endif
