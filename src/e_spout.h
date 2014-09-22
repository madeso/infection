/******************************************************************************/
/*  e_env_spout.h                                                                 */
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

#ifndef E_env_spout_H
#define E_env_spout_H

#include <Windows.H>

#ifdef __cplusplus
extern "C" {
#endif

#pragma warning( disable : 4068 )

#pragma GE_Type("env_spout.bmp")
typedef struct env_spout
{
#pragma	GE_Private
	int		EffectList;
	int		EffectCount;
	float		PauseTime;
	geBoolean 	PauseState;
#pragma GE_Published
	geVec3d		Position;
	geVec3d		Angles;
	float		ParticleCreateRate;
	float		MinScale;
	float		MaxScale;
	float		MinSpeed;
	float		MaxSpeed;
	float		MinUnitLife;
	float		MaxUnitLife;
	int		SourceVariance;
	int		DestVariance;
        GE_RGBA		ColorMin;
        GE_RGBA		ColorMax;
	geVec3d		Gravity;
	char		*BmpName;
	char		*AlphaName;
	char		*TriggerName;
	float		MinPauseTime;
	float		MaxPauseTime;
	float		TotalLife;
//	StaticEntityProxy *Entity;
//	char		*BoneName;

#pragma GE_Origin( Position )
#pragma GE_Angles(Angles)			
#pragma GE_DefaultValue( ParticleCreateRate, "0.1" )
#pragma GE_DefaultValue( MinScale, "1.0" )
#pragma GE_DefaultValue( MaxScale, "1.0" )
#pragma GE_DefaultValue( MinSpeed, "10.0" )
#pragma GE_DefaultValue( MaxSpeed, "30.0" )
#pragma GE_DefaultValue( MinUnitLife, "3.0" )
#pragma GE_DefaultValue( MaxUnitLife, "6.0" )
#pragma GE_DefaultValue( SourceVariance, "0" )
#pragma GE_DefaultValue( DestVariance, "1" )
#pragma GE_DefaultValue( ColorMin, "255.0 255.0 255.0" )
#pragma GE_DefaultValue( ColorMax, "255.0 255.0 255.0" )
#pragma GE_DefaultValue( Gravity, "0.0 0.0 0.0" )
#pragma GE_DefaultValue( TotalLife, "0.0" )
#pragma GE_DefaultValue( TriggerName, "NULL" )
#pragma GE_DefaultValue( MinPauseTime, "0.0" )
#pragma GE_DefaultValue( MaxPauseTime, "0.0" )

#pragma GE_Documentation( Position, "Location of effect, if it's not hooked to an actor" )
#pragma GE_Documentation( Angles, "Direction in which particles will shoot" )
#pragma GE_Documentation( ParticleCreateRate, "Every how many seconds to add a new particle" )
#pragma GE_Documentation( MinScale, "Min scale of the textures" )
#pragma GE_Documentation( MaxScale, "Max scale of the textures" )
#pragma GE_Documentation( MinSpeed, "Min speed of the textures" )
#pragma GE_Documentation( MaxSpeed, "Max speed of the textures" )
#pragma GE_Documentation( MinUnitLife, "Min life of each texture" )
#pragma GE_Documentation( MaxUnitLife, "Max life of each texture" )
#pragma GE_Documentation( SourceVariance, "How much to vary spray source point" )
#pragma GE_Documentation( DestVariance, "How much to vary spray dest point" )
#pragma GE_Documentation( ColorMin, "Minimum RGB values for each particle" )
#pragma GE_Documentation( ColorMax, "Maximum RGB values for each particle" )
#pragma GE_Documentation( Gravity, "Gravity vector to apply to each particle" )
#pragma GE_Documentation( BmpName, "Name of bitmap file to use" )
#pragma GE_Documentation( AlphaName, "Name of alpha bitmap file to use" )
#pragma GE_Documentation( TotalLife, "How many seconds this spout lasts. Set to 0 for continuous." )
#pragma GE_Documentation( TriggerName, "Name of the associated trigger" )
#pragma GE_Documentation( MinPauseTime, "Low range of randomly chosen pause time (seconds)" )
#pragma GE_Documentation( MaxPauseTime, "High range of randomly chosen pause time (seconds)" )

} env_spout;
#pragma warning ( default:4068)
geBoolean Spout_Create(geEngine *Engine, Eff_Manager *EM);
Spout_Destroy(Eff_Manager *EM);
void Spout_Tick(Eff_Manager *EM);

#ifdef __cplusplus
}
#endif

#endif
