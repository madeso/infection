/****************************************************************************************/
/*  SPool.h																				*/
/****************************************************************************************/
#ifndef SPOOL_H
#define SPOOL_H

#include "genesis.h"

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct	SPool SPool;

typedef struct	SPool
{
  SPool    *next;
  SPool    *prev;
  geSound_Def	*SoundDef;
  char     *Name;
} SPool;


void SPool_Initalize();
geSound_Def *SPool_Sound(char *SName, geSound_System *SoundSystem);
void SPool_Delete(geSound_System *SoundSystem);



#ifdef __cplusplus
	}
#endif
#pragma warning ( default : 4068 )

#endif
