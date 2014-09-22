/******************************************************************************/
/*  SPool.c                                                                   */
/*                                                                            */
/*  Author: David Wulff                                                       */
/*  Description: Sound Manager                                                */
/*                                                                            */
/*  Copyright (c) 1999, 2000, Battleaxe Studios; All rights reserved.         */
/*           ______  __   __  ___      __  __  __  __  __  _____    __        */
/*          / ____/ / /  / / / _ \    / / / / / / / / / / / ___ \  / /        */
/*         / /___  / /  / / / /_\ \  / / / / / / / / / / / /__/ / / /         */
/*        /___  / / /  / / / ___  / / / / / / / / / / / /  __  / / /          */
/*       ____/ / / /__/ / / /  / /  \ \/ / / /  \ \/ / / /  / / / /___        */
/*      /_____/ /______/ /_/  /_/    \__/ /_/    \__/ /_/  /_/ /_____/        */
/*                                                                            */
/*                              www.bttlxe.co.uk                              */
/******************************************************************************/

#include <memory.h>
#include <malloc.h>
#include <string.h>
#include "Genesis.h"
#include "Ram.h"
#include "SPool.h"


#include "useful_macros.h"


static geBoolean EffectC_IsStringNull(char *String );

SPool *Bottom;


void SPool_Initalize()
{
	Bottom=(SPool *)NULL;
}

geSound_Def *SPool_Sound(char *SName, geSound_System *SoundSystem)
{
	
  SPool *pool;
  geVFile *MainFS;

  if(EffectC_IsStringNull(SName)==GE_TRUE)
    return (geSound_Def *)NULL;

  pool=Bottom;
  while ( pool != NULL )
  {
    if(!stricmp(SName, pool->Name))
      return pool->SoundDef;
    pool = pool->next;
  } 

  MainFS = geVFile_OpenNewSystem(NULL, GE_VFILE_TYPE_DOS, SName, NULL, GE_VFILE_OPEN_READONLY);
  pool = ALLOCATE_STRUCT(SPool);
  memset(pool, 0, sizeof(SPool));
  pool->next = Bottom;
  Bottom = pool;
  if(pool->next)
    pool->next->prev = pool;
  pool->Name=strdup(SName);
  pool->SoundDef = geSound_LoadSoundDef(SoundSystem, MainFS);
  geVFile_Close(MainFS);

  if(!pool->SoundDef)
    return (geSound_Def *)NULL;

  return pool->SoundDef;
}

void SPool_Delete(geSound_System *SoundSystem)
{
  SPool *pool, *temp;

  pool = Bottom;

  while	(pool!= NULL)
  {
	  temp = pool->next;

    geSound_FreeSoundDef(SoundSystem, pool->SoundDef );
	free(pool->Name);
    free(pool);

	pool = temp;
  }
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectC_IsStringNull()
//
//	Determines if a string is NULL, accounting for additional editor posibilities.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean EffectC_IsStringNull(char *String )
{

	// first way
	if ( String == NULL )
	{
		return GE_TRUE;
	}

	// second way
	if ( strlen( String ) < 1 )
	{
		return GE_TRUE;
	}

	// third way
	if ( strnicmp( String, "<null>", 6 ) == 0 )
	{
		return GE_TRUE;
	}

	// fourth way
	if ( strnicmp( String, "NULL", 4 ) == 0 )
	{
		return GE_TRUE;
	}

	// if we got to here then the string is not null
	return GE_FALSE;

} // EffectC_IsStringNull()
