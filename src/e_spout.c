/******************************************************************************/
/*  e_spout.c                                                                 */
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
//#include "gebmutil.h"
#include "StaticEntity.h"
#include "e_spout.h"




static geBoolean EffectC_IsStringNull(char *String );
static float EffectC_Frand(float Low, float High );

//--------------------------------------------------------------------
// Spout Constructor
//--------------------------------------------------------------------

geBoolean Spout_Create(geEngine *Engine, Eff_Manager *EM)
{
	geEntity_EntitySet *	Set;
	geEntity *		Entity;
	
	Set = geWorld_GetEntitySet(EM->World, "env_spout");
	if (Set == NULL)
		return GE_TRUE;
	
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);
	
	while	(Entity)
	{
		env_spout *	S;
		Spray	Sp;
		geVec3d	In;
		geXForm3d	Xf;
		
		S = (env_spout *)geEntity_GetUserData(Entity);
		
		if(S->MinPauseTime < 0.0f)
			S->MinPauseTime = 0.0f;
		if(S->MaxPauseTime < S->MinPauseTime)
			S->MaxPauseTime = S->MinPauseTime;
		if (S->MaxPauseTime > 0.0f)
			S->PauseTime = EffectC_Frand( S->MinPauseTime, S->MaxPauseTime );
		
		memset( &Sp, 0, sizeof( Sp ) );
		
		Sp.Texture=TPool_Bitmap(".\\gfx\\fx\\lvsmoke.Bmp", ".\\gfx\\fx\\A_lvsmoke.Bmp", S->BmpName, S->AlphaName, EM->World);
		
		Sp.SourceVariance = S->SourceVariance;
		if(Sp.SourceVariance < 0 )
			Sp.SourceVariance = 0;
		Sp.DestVariance = S->DestVariance;
		if(Sp.DestVariance < 0 )
			Sp.DestVariance = 0;
		Sp.MinScale = S->MinScale;
		Sp.MaxScale = S->MaxScale;
		if(Sp.MinScale <= 0.0f )
			Sp.MinScale = 0.1f;
		if(Sp.MaxScale < Sp.MinScale )
			Sp.MaxScale = Sp.MinScale;
		Sp.MinSpeed = S->MinSpeed;
		Sp.MaxSpeed = S->MaxSpeed;
		if(Sp.MinSpeed < 0.0f )
			Sp.MinSpeed = 0.0f;
		if(Sp.MaxSpeed < Sp.MinSpeed )
			Sp.MaxSpeed = Sp.MinSpeed;
		Sp.MinUnitLife = S->MinUnitLife;
		Sp.MaxUnitLife = S->MaxUnitLife;
		if(Sp.MinUnitLife <= 0.0f )
			Sp.MinUnitLife = 0.1f;
		if(Sp.MaxUnitLife < Sp.MinUnitLife )
			Sp.MaxUnitLife = Sp.MinUnitLife;
		Sp.ColorMin.r = S->ColorMin.r;
		if(Sp.ColorMin.r < 0.0f )
			Sp.ColorMin.r = 0.0f;
		if(Sp.ColorMin.r > 255.0f )
			Sp.ColorMin.r = 255.0f;
		Sp.ColorMax.r = S->ColorMax.r;
		if(Sp.ColorMax.r < Sp.ColorMin.r )
			Sp.ColorMax.r = Sp.ColorMin.r;   
		Sp.ColorMin.g = S->ColorMin.g;
		if(Sp.ColorMin.g < 0.0f )
			Sp.ColorMin.g = 0.0f;
		if(Sp.ColorMin.g > 255.0f )
			Sp.ColorMin.g = 255.0f;
		Sp.ColorMax.g = S->ColorMax.g;
		if(Sp.ColorMax.g < Sp.ColorMin.g )
			Sp.ColorMax.g = Sp.ColorMin.g;
		Sp.ColorMin.b = S->ColorMin.b;
		if(Sp.ColorMin.b < 0.0f )
			Sp.ColorMin.b = 0.0f;
		if(Sp.ColorMin.b > 255.0f )
			Sp.ColorMin.b = 255.0f;
		Sp.ColorMax.b = S->ColorMax.b;
		if(Sp.ColorMax.b < Sp.ColorMin.b )
			Sp.ColorMax.b = Sp.ColorMin.b;
		Sp.ColorMin.a = 255.0f;
		Sp.ColorMax.a = 255.0f;
		Sp.Rate = S->ParticleCreateRate;
		if(Sp.Rate < 0.0f )
			Sp.Rate = 0.1f;
		geVec3d_Copy( &( S->Gravity ), &( Sp.Gravity ) );
		Sp.ScaleStyle = SSS_RANDOM_SCALE;
		
		// setup orientation
		geXForm3d_SetIdentity( &( Xf ) );
		geXForm3d_RotateX( &( Xf ), S->Angles.X / 57.3f );  
		geXForm3d_RotateY( &( Xf ), ( S->Angles.Y - 90.0f ) / 57.3f );  
		geXForm3d_RotateZ( &( Xf ), S->Angles.Z / 57.3f ); 
		geVec3d_Copy( &( S->Position ), &( Sp.Source ) );

		geXForm3d_GetIn( &( Xf ), &In );
		geVec3d_Inverse( &In );
		geVec3d_AddScaled( &( Sp.Source ), &In, 50.0f, &( Sp.Dest ) );
		
		S->EffectList = EM_Item_Add(EM, EFF_SPRAY, (void *)&Sp);
		
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
	EM->m_LastTick = timeGetTime();
	return GE_TRUE;
}


//--------------------------------------------------------------------
// Spout Destructor
//--------------------------------------------------------------------

Spout_Destroy(Eff_Manager *EM)
{
	return 1;
}


//--------------------------------------------------------------------
// Spout Tick
//--------------------------------------------------------------------

void Spout_Tick(Eff_Manager *EM)
{
	geEntity_EntitySet *	Set;
	geEntity *		Entity;
	
	float dwTicks = (float)(timeGetTime() - EM->m_LastTick);
	
	if(dwTicks > 100)
		dwTicks = 100;	// Correct for possible stalls elsewhere
	
	Set = geWorld_GetEntitySet(EM->World, "env_spout");
	if (Set == NULL)
		return;
	
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);
	
	while	(Entity)
	{
		env_spout *	S;
		
		S = (env_spout *)geEntity_GetUserData(Entity);
		
		// kill the effect if its time has run out
		if ( S->TotalLife > 0.0f )
		{
			S->TotalLife -= (dwTicks*0.001f);
			if ( S->TotalLife <= 0.0f )
			{
				S->TotalLife = 0.0f;
				EM_Item_Delete(EM, EFF_SPRAY, S->EffectList);
			}
		}
        else
        {
			// adjust pause time
			if ( S->PauseTime > 0.0f )
			{
				S->PauseTime -= (dwTicks*0.001f);
				if ( S->PauseTime <= 0.0f )
				{
					S->PauseTime = EffectC_Frand( S->MinPauseTime, S->MaxPauseTime );
					S->PauseState = !S->PauseState;
					EM_Item_Pause(EM, EFF_SPRAY, S->EffectList, S->PauseState);
				}
			}
        } 
		
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
	EM->m_LastTick = timeGetTime();
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

////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectC_Frand()
//
//	Picks a random float within the supplied range.
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


