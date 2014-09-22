/******************************************************************************/
/*  e_rain.h                                                                  */
/*                                                                            */
/*  Author: David Wulff                                                       */
/*  Description: Env header file                                              */
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
#ifndef E_RAIN_H
#define E_RAIN_H

#include "genesis.h"
//#include "env.h"

#include "EffManager.h"
#include "StaticEntity.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma warning( disable : 4068 )

#pragma GE_Type("Item.ico")

typedef struct env_rain
{
#pragma GE_Private
	int 	*EffectList;
	int		EffectCount;
#pragma GE_Published
	geVec3d		Position;
//	StaticEntityProxy *Entity;
	geVec3d		Gravity;
	float		Radius;
	float		Severity;
	float		DropLife;
  GE_RGBA		ColorMin;
  GE_RGBA		ColorMax;
	char		*BmpName;
	char		*AlphaName;
	char		*TriggerName;
//	char		*BoneName;
#pragma GE_Origin( Position )
#pragma GE_DefaultValue( Gravity, "0.0 -60.0 0.0" )
#pragma GE_DefaultValue( Radius, "100.0" )
#pragma GE_DefaultValue( Severity, "0.5" )
#pragma GE_DefaultValue( DropLife, "1.0" )
#pragma GE_DefaultValue( ColorMin, "255.0 255.0 255.0" )
#pragma GE_DefaultValue( ColorMax, "255.0 255.0 255.0" )
#pragma GE_DefaultValue( TriggerName, "NULL" )
#pragma GE_Documentation( Position, "Location of effect, if it's not hooked to an actor" )
#pragma GE_Documentation( Gravity, "The velocity applied to each drop each second (world units)" )
#pragma GE_Documentation( Radius, "The redius of the rain coverage aren (world units)" )
#pragma GE_Documentation( Severity, "How severe the rain is, 0.0 being tame, 1.0 being insanity" )
#pragma GE_Documentation( DropLife, "How long a drop lasts (seconds)" )
#pragma GE_Documentation( ColorMin, "Minimum RGB for each drop" )
#pragma GE_Documentation( ColorMax, "Maximum RGB for each drop" )
#pragma GE_Documentation( BmpName, "Name of bitmap file to use" )
#pragma GE_Documentation( AlphaName, "Name of alpha bitmap file to use" )
#pragma GE_Documentation( TriggerName, "Name of the associated trigger" )
} env_rain;

#pragma warning( default : 4068 )


// Definitions
geBoolean Rain_Create(geEngine *Engine, Eff_Manager *EM);
Rain_Destroy(Eff_Manager *EM);
void Rain_Tick(Eff_Manager *EM);


#ifdef __cplusplus
}
#endif

#endif
