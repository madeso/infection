/******************************************************************************/
/*  e_sprite.c                                                                */
/*                                                                            */
/*  Author: David Wulff                                                       */
/*  Description: Env implementation                                           */
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
//#include "GMain.h"

#include "EffManager.h"

#include "e_sprite.h"

#include <malloc.h>
#include <memory.h>
#include <stdlib.h>
#include <windows.h>


extern geBitmap *TPool_Bitmap(char *DefaultBmp, char *DefaultAlpha, char *BName, char *AName, geWorld *World);

static float EffectC_Frand(float Low, float High );

//--------------------------------------------------------------------
// CSprite Constructor
//--------------------------------------------------------------------

geBoolean Sprite_Create(geEngine *Engine, Eff_Manager *EM)
{
	geEntity_EntitySet *	Set;
	geEntity *		Entity;
	int i;
	
	// test for any spriteing particle entities
	Set = geWorld_GetEntitySet(EM->World, "env_sprite");
	// there is none
	if (Set == NULL)
		return GE_TRUE;
	
	// get the first entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);
	
	// wade thru all of them
	while	(Entity)
	{
		env_sprite *	S;
		Sprite	Spr;
		
		// get data
		S = (env_sprite *)geEntity_GetUserData(Entity);
		
		// get total number of sprites to use
		S->EffectCount=S->ParticleCount;
		
		// create dynamic array to hold index numbers
		S->EffectList = (int *)malloc(sizeof(int) * S->EffectCount);

		if(! S->EffectList ) return GE_FALSE;

		// create dynamic xform array
		S->Xf = (geXForm3d *)calloc( S->ParticleCount * sizeof ( *( S->Xf ) ) ,1);
		// create dynamic speed array
		S->Speed = (float *)calloc( S->ParticleCount * sizeof( *( S->Speed ) ) ,1);
		
		// clear out sprite data
		memset( &Spr, 0, sizeof( Spr ) );
		
		// get the bitmap used as the sprite
		// g_bubble/A__bubl are default bitmap names
		S->Bitmap=TPool_Bitmap(".\\gfx\\fx\\g_bubble.Bmp", ".\\gfx\\fx\\A_bubl.Bmp", S->BmpName, S->AlphaName, EM->World);
		Spr.Texture = &S->Bitmap;
		
		// make sure colors are valid values
		if( S->Color.r < 0.0f )
			S->Color.r = 0.0f;
		Spr.Color.r = S->Color.r;
		if( S->Color.g < 0.0f )
			S->Color.g = 0.0f;
		Spr.Color.g = S->Color.g;
		if( S->Color.b < 0.0f )
			S->Color.b = 0.0f;
		Spr.Color.b = S->Color.b;
		Spr.Color.a = 255.0f;
		// only one sprite so don't cycle at all
		Spr.TotalTextures = 1;
		Spr.Style = SPRITE_CYCLE_NONE;
		// set sprite scale
		Spr.Scale = S->Scale;
		
		// set sprite position
		geVec3d_Copy( &( S->Position ), &( S->BasePos ) );
		 
		// do for required number of sprites
		for ( i = 0; i < S->EffectCount; i++ )
		{
			// set random direction
			geXForm3d_SetIdentity( &( S->Xf[i] ) );
			geXForm3d_RotateX( &( S->Xf[i] ), EffectC_Frand( -S->XSlant, S->XSlant ) );
			geXForm3d_RotateZ( &( S->Xf[i] ), EffectC_Frand( -S->ZSlant, S->ZSlant ) );
			
			// pick random start spot
			geVec3d_Copy( &( S->BasePos ), &( S->Xf[i].Translation ) );
			S->Xf[i].Translation.X += EffectC_Frand( -( S->Radius / 2.0f ), S->Radius / 2.0f );
			S->Xf[i].Translation.Z += EffectC_Frand( -( S->Radius / 2.0f ), S->Radius / 2.0f );
			geVec3d_Copy( &( S->Xf[i].Translation ), &( Spr.Pos ) );
			
			// set speed
			S->Speed[i] = EffectC_Frand( S->MinSpeed, S->MaxSpeed );
			
			// add effect to manager
			S->EffectList[i] = EM_Item_Add(EM, EFF_SPRITE, (void *)&Spr);
		}
		
		// process next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
	// set inital time
	EM->m_LastTick = timeGetTime();
	
	return GE_TRUE;
}


//--------------------------------------------------------------------
// CSprite  Destructor
//--------------------------------------------------------------------

Sprite_Destroy(Eff_Manager *EM)
{
	geEntity_EntitySet *	Set;
	geEntity *		Entity;
	
	Set = geWorld_GetEntitySet(EM->World, "env_sprite");
	if (Set == NULL)
		return 0;
	
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);
	
	while	(Entity)
	{
		env_sprite *	R;
		
		R = (env_sprite *)geEntity_GetUserData(Entity);
		// free any dynamic arrays
		free(R->EffectList);
		if ( R->Xf != NULL )
			free( R->Xf );
		if ( R->Speed != NULL )
			free( R->Speed );
		
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
	return 1;
}


void Sprite_Tick(Eff_Manager *EM)
{
	geEntity_EntitySet *	Set;
	geEntity *		Entity;
	int i;
	
	// get amount of time since last call
	float dwTicks = (float)(timeGetTime() - EM->m_LastTick)*0.001f;
	
	Set = geWorld_GetEntitySet(EM->World, "env_sprite");
	if (Set == NULL)
		return;
	
	// wade thru all entities
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);
	
	while	(Entity)
	{
		env_sprite *	S;
		geVec3d Movement;
		geVec3d  Pos;
		geVec3d  Offset;
		Sprite  Spr;
		
		// get data
		S = (env_sprite *)geEntity_GetUserData(Entity);
		
		{
			geVec3d_Set( &Movement, 0.0f, 0.0f, 0.0f );
		}
		// modify all sprites
		for ( i = 0; i < S->EffectCount; i++ )
		{
			// adjust this particles orientation
			geVec3d_Copy( &( S->Xf[i].Translation ), &Pos );
			geVec3d_Set( &( S->Xf[i].Translation ), 0.0f, 0.0f, 0.0f );
			geXForm3d_RotateY( &( S->Xf[i] ), dwTicks * 5.0f );
			geVec3d_Copy( &Pos, &( S->Xf[i].Translation ) );
			
			// adjust particle position
			geXForm3d_GetUp( &( S->Xf[i] ), &Offset );
			geVec3d_AddScaled( &( S->Xf[i].Translation ), &Offset, dwTicks * S->Speed[i], &( S->Xf[i].Translation ) );
			geVec3d_Add( &( S->Xf[i].Translation ), &Movement, &( S->Xf[i].Translation ) );
			geVec3d_Copy( &( S->Xf[i].Translation ), &( Spr.Pos ) );
			
			// reset particle if it has hit its height limit
			if ( ( S->Xf[i].Translation.Y - S->BasePos.Y ) > S->Height )
			{
				// set random direction
				geXForm3d_SetIdentity( &( S->Xf[i] ) );
				geXForm3d_RotateX( &( S->Xf[i] ), EffectC_Frand( -S->XSlant, S->XSlant ) );
				geXForm3d_RotateZ( &( S->Xf[i] ), EffectC_Frand( -S->ZSlant, S->ZSlant ) );
				
				// pick random start spot
				geVec3d_Copy( &( S->BasePos ), &( S->Xf[i].Translation ) );
				S->Xf[i].Translation.X += EffectC_Frand( -( S->Radius / 2.0f ), S->Radius / 2.0f );
				S->Xf[i].Translation.Z += EffectC_Frand( -( S->Radius / 2.0f ), S->Radius / 2.0f );
				geVec3d_Copy( &( S->Xf[i].Translation ), &( Spr.Pos ) );
				
				// set speed
				S->Speed[i] = EffectC_Frand( S->MinSpeed, S->MaxSpeed );
			}
			
			// modify sprite in effect manager
			EM_Item_Modify(EM, EFF_SPRITE, S->EffectList[i], (void *)&Spr, SPRITE_POS);
		}
		
		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
	// save current time
	EM->m_LastTick = timeGetTime();
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectC_Frand()
//
//	Picks a random sprite within the supplied range.
//
////////////////////////////////////////////////////////////////////////////////////////
static float EffectC_Frand(float Low, float High )
{
	
	// locals
	float	Range;
	
	// if they are the same then just return one of them
	if ( High == Low )
	{
		return Low;
	}
	
	// pick a random float from whithin the range
	Range = High - Low;
	return ( (float)( ( ( rand() % 1000 ) + 1 ) ) ) / 1000.0f * Range + Low;
	
} // EffectC_Frand()


