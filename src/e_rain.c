/******************************************************************************/
/*  e_rain.c                                                                  */
/*                                                                            */
/*  Author: David Wulff, Ralph Deane                                          */
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

#include "e_rain.h"

#include <stdlib.h>
#include <memory.h>


#define RAINM_RADIUSTOEFFECTRATIO	50

extern geBitmap *TPool_Bitmap(char *DefaultBmp, char *DefaultAlpha, char *BName, char *AName, geWorld *World);


//======================
// Rain_Create
//======================
geBoolean Rain_Create(geEngine *Engine, Eff_Manager *EM)
{
	geEntity_EntitySet	*Set;
	geEntity				*Entity;

	Set = geWorld_GetEntitySet(EM->World, "env_rain");
	if (Set == NULL)
		return GE_TRUE;
	
	// get first rain entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while	(Entity)
	{
		env_rain *	R;
		Spray	Sp;
		int i;
		
		// get data
		R = (env_rain *)geEntity_GetUserData(Entity);
		// calculate number of Sprays needed
		R->EffectCount = (int)R->Radius / RAINM_RADIUSTOEFFECTRATIO;
		// create dynamic array to hold effect indexes
		R->EffectList = (int *)malloc(sizeof(int) * R->EffectCount);

		if(!R->EffectList) return GE_FALSE;

		// make sure data is valid
		if( R->Severity < 0.0f )
			R->Severity=0.0f;
		if( R->Severity > 1.0f )
			R->Severity=1.0f;
		if( R->DropLife <= 0.0f )
			R->DropLife=0.1f;

		// clear out spray data
		memset( &Sp, 0, sizeof( Sp ) );

		// get bitmap for use as texture
		// rain/a_rain are default bitmap names
		Sp.Texture=TPool_Bitmap(".\\gfx\\fx\\rain.Bmp", ".\\gfx\\fx\\A_rain.Bmp", R->BmpName, R->AlphaName, EM->World);

		// setup spray data
		Sp.MinScale = 0.5f;
		Sp.MaxScale = 1.5f;
		Sp.ShowAlways=GE_TRUE;
		Sp.Rate = ( 1.1f - R->Severity ) * 0.1f;
		geVec3d_Copy( &( R->Gravity ), &( Sp.Gravity ) );
		// set source position
		geVec3d_Copy( &( R->Position ), &( Sp.Source ) );
		Sp.SourceVariance = (int)( R->Radius / 2.0f );
		Sp.MinUnitLife = R->DropLife;
		Sp.MaxUnitLife = R->DropLife;
		// set destination position
		geVec3d_AddScaled( &( Sp.Source ), &( Sp.Gravity ), Sp.MinUnitLife, &( Sp.Dest ) );
		Sp.DestVariance = (int)( R->Radius / 2.0f );
		memcpy( &( Sp.ColorMin ), &( R->ColorMin ), sizeof( Sp.ColorMin ) );
		memcpy( &( Sp.ColorMax ), &( R->ColorMax ), sizeof( Sp.ColorMax ) );
		Sp.ColorMin.a = Sp.ColorMax.a = 255.0f;
		Sp.ScaleStyle = SSS_RANDOM_SCALE;
		// check if hooked to actor bone

		// for all sprays add to effect manager
		for ( i = 0; i < R->EffectCount; i++ )
			R->EffectList[i]=EM_Item_Add(EM, EFF_SPRAY, (void *)&Sp);

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
	return GE_TRUE;
}


//--------------------------------------------------------------------
// CRain  Destructor
//--------------------------------------------------------------------

Rain_Destroy(Eff_Manager *EM)
{
	geEntity_EntitySet *	Set;
	geEntity *		Entity;
	
	Set = geWorld_GetEntitySet(EM->World, "env_rain");
	if (Set == NULL)
		return 0;
	
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);
	
	while	(Entity)
	{
		env_rain *	R;
		
		R = (env_rain *)geEntity_GetUserData(Entity);
		// free dynamic array
		free(R->EffectList);
		
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
	return 1;
}

//-------------------------------------------------------
// Tick
//-------------------------------------------------------

void Rain_Tick(Eff_Manager *EM)
{
	geEntity_EntitySet *	Set;
	geEntity *		Entity;
	return;
	
	Set = geWorld_GetEntitySet(EM->World, "env_rain");
	if (Set == NULL)
		return;
	
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);
	
	/*
	while	(Entity)
	{
		env_rain *	R;
		
		R = (env_rain *)geEntity_GetUserData(Entity);
		
		// adjust position if its locked to an actor
		
		if((R->Entity != NULL) && (R->Entity->Actor != NULL))
		{
			// locals
			geXForm3d	Xf;
			Spray		Sp;
			int		i;
			
			// get bone location
			if(geActor_GetBoneTransform(R->Entity->Actor, R->BoneName, &Xf )==GE_TRUE)
			{
				geVec3d_Copy( &( Xf.Translation ), &( Sp.Source ) );
				geVec3d_AddScaled( &( Sp.Source ), &( Sp.Gravity ), Sp.MinUnitLife, &( Sp.Dest ) );
				// adjust position
				for ( i = 0; i < R->EffectCount; i++ )
					EM_Item_Modify(EM, EFF_SPRAY, R->EffectList[i], (void *)&Sp, SPRAY_SOURCE | SPRAY_DEST);
			}
		}
		
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
	*/
}

