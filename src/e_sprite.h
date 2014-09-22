/******************************************************************************/
/*  e_sprite.h                                                                */
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
#pragma warning( disable : 4068 )

#ifndef E_SPRITE_H
#define E_SPRITE_H

#include "genesis.h"
//#include "env.h"

#include "EffManager.h"
#include "StaticEntity.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma warning( disable : 4068 )

#pragma GE_Type("env_sprite.bmp")
typedef struct env_sprite
{
#pragma	GE_Private
	int 	*EffectList;
	int	EffectCount;
	geXForm3d	*Xf;
	float		*Speed;
	geVec3d		BasePos;
	geBitmap	*Bitmap;

#pragma GE_Published
	geVec3d		Position;
        GE_RGBA		Color;
	float		Scale;
	int		ParticleCount;
	float		Radius;
	float		Height;
	float		XSlant;
	float		ZSlant;
	float		MinSpeed;
	float		MaxSpeed;
	char		*BmpName;
	char		*AlphaName;
	char		*TriggerName;
//	StaticEntityProxy *Entity;
//	char		*BoneName;

#pragma GE_Origin( Position )
#pragma GE_DefaultValue( Color, "255.0 255.0 255.0" )
#pragma GE_DefaultValue( Scale, "1.0" )
#pragma GE_DefaultValue( ParticleCount, "10" )
#pragma GE_DefaultValue( Radius, "100" )
#pragma GE_DefaultValue( Height, "100" )
#pragma GE_DefaultValue( XSlant, "0.65" )
#pragma GE_DefaultValue( ZSlant, "0.65" )
#pragma GE_DefaultValue( MinSpeed, "60.0" )
#pragma GE_DefaultValue( MaxSpeed, "150.0" )
#pragma GE_DefaultValue( TriggerName, "NULL" )
#pragma GE_Documentation( Position, "Location of effect, if it's not hooked to an actor" )
#pragma GE_Documentation( Color, "Texture color" )
#pragma GE_Documentation( Scale, "What scale to use for the bitmap" )
#pragma GE_Documentation( ParticleCount, "How many particles to use" )
#pragma GE_Documentation( Radius, "Radius of particle cylinder" )
#pragma GE_Documentation( Height, "Max height each particle will go from base" )
#pragma GE_Documentation( XSlant, "Upwards slant on X axis" )
#pragma GE_Documentation( ZSlant, "Upwards slant on Z axis" )
#pragma GE_Documentation( MinSpeed, "Min speed of each particle" )
#pragma GE_Documentation( MaxSpeed, "Max speed of each particle" )
#pragma GE_Documentation( BmpName, "Name of bitmap file to use" )
#pragma GE_Documentation( AlphaName, "Name of alpha bitmap file to use" )
#pragma GE_Documentation( TriggerName, "Name of the associated trigger" )

} env_sprite;

#pragma warning( default : 4068 )

// Definitions
geBoolean Sprite_Create(geEngine *Engine, Eff_Manager *EM);
Sprite_Destroy(Eff_Manager *EM);
void Sprite_Tick(Eff_Manager *EM);


#ifdef __cplusplus
}
#endif

#endif
