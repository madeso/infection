/******************************************************************************/
/*  TPool.c                                                                   */
/*                                                                            */
/*  Author: David Wulff                                                       */
/*  Description: Texture Management                                           */
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

#include <windows.h>
#include <memory.h>
#include <malloc.h>
#include <string.h>
#include "Genesis.h"
#include "Ram.h"
#include "tpool.h"


#include "useful_macros.h"

static geBitmap *CreateFromFileAndAlphaNames(const geVFile * BaseFS,const char * BmName,const char *AlphaName);
static geBitmap * CreateFromFileName(const geVFile * BaseFS,const char * BmName);
static geBoolean EffectC_IsStringNull(char *String );

TPool *TPool_Bottom;

void TPool_Initalize()
{
  TPool_Bottom=(TPool *)NULL;
}

geBitmap *TPool_Bitmap(char *DefaultBmp, char *DefaultAlpha, char *BName, char *AName, geWorld *World)
{
  TPool *pool;
  geVFile *MainFS;
  char	TempName[256];
  char *TBName, *TAName;

  if(EffectC_IsStringNull(BName ) == GE_TRUE)
  {
	  TBName=DefaultBmp;
	  TAName=DefaultAlpha;
  }
  else
  {
	   if(EffectC_IsStringNull(AName ) == GE_TRUE)
	   {
	     TBName=BName;
		 TAName=BName;
	   }
	   else
	   {
	     TBName=BName;
		 TAName=AName;
	   }
  }

  pool=TPool_Bottom;
  while ( pool != NULL )
  {
    if((!stricmp(TBName, pool->BmpName)) && (!stricmp(TAName, pool->AlphaName)))
      return pool->Bitmap;
    pool = pool->next;
  } 
  GetCurrentDirectory(sizeof(TempName), TempName);
  MainFS = geVFile_OpenNewSystem(NULL, GE_VFILE_TYPE_DOS, TempName, NULL, GE_VFILE_OPEN_READONLY | GE_VFILE_OPEN_DIRECTORY);

  pool = ALLOCATE_STRUCT(TPool);
  memset(pool, 0, sizeof(TPool));
  pool->next = TPool_Bottom;
  TPool_Bottom = pool;
  if(pool->next)
    pool->next->prev = pool;
  pool->BmpName=strdup(TBName);
  pool->AlphaName=strdup(TAName);
  pool->Bitmap = CreateFromFileAndAlphaNames(MainFS, TBName, TAName);

  if(!pool->Bitmap)
    return (geBitmap *)NULL;

  if (!geWorld_AddBitmap(World, pool->Bitmap))
  {
    geBitmap_Destroy(&pool->Bitmap);
    return (geBitmap *)NULL;
  }

  return pool->Bitmap;
}

void TPool_Delete(geWorld *World)
{
  TPool *pool, *temp;

  pool = TPool_Bottom;
  while	(pool!= NULL)
  {
	temp = pool->next;

	if( pool->Bitmap ){
		geWorld_RemoveBitmap(World, pool->Bitmap);
		geBitmap_Destroy(&pool->Bitmap);
	}
	free(pool->BmpName);
	free(pool->AlphaName);
        free(pool);

	pool = temp;
  }
}

/*----------------------------------------------------------------------------------------------------*/
//
// Utility functions to load bitmaps
//   from gebmutil.c
//
static geBitmap * CreateFromFileAndAlphaNames(const geVFile * BaseFS,const char * BmName,const char *AlphaName)
{
geBitmap *Bmp,*AlphaBmp;

	Bmp = CreateFromFileName(BaseFS,BmName);
	if ( ! Bmp )
		return NULL;

	AlphaBmp = CreateFromFileName(BaseFS,AlphaName);
	if ( ! AlphaBmp )
	{
		geBitmap_Destroy(&Bmp);
		return NULL;
	}

	if ( ! geBitmap_SetAlpha(Bmp,AlphaBmp) )
	{
		geBitmap_Destroy(&Bmp);
		geBitmap_Destroy(&AlphaBmp);
		return NULL;
	}

	geBitmap_Destroy(&AlphaBmp);

	geBitmap_SetPreferredFormat(Bmp,GE_PIXELFORMAT_16BIT_4444_ARGB);

return Bmp;
}

static geBitmap * CreateFromFileName(const geVFile * BaseFS,const char * BmName)
{
geVFile * File;
geBitmap * Bmp;

	if ( BaseFS )
		File = geVFile_Open((geVFile *)BaseFS,BmName,GE_VFILE_OPEN_READONLY);
	else
		File = geVFile_OpenNewSystem(NULL,GE_VFILE_TYPE_DOS,BmName,NULL,GE_VFILE_OPEN_READONLY);
	if ( ! File )
		return NULL;

	Bmp = geBitmap_CreateFromFile(File);
	geVFile_Close(File);

return Bmp;
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
