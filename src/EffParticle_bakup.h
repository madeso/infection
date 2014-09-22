/******************************************************************************/
/*  EffParticle.h                                                             */
/*                                                                            */
/*  Author: David Wulff, Eli Boling, Peter Siamidis                           */
/*  Description: Partical System Manager                                      */
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

#ifndef EFFECTPARTICLE_H
#define EFFECTPARTICLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "genesis.h"

typedef struct Particle Particle;

typedef struct	Particle
{
	GE_LVertex		ptclVertex;
	geBitmap *		ptclTexture;
	gePoly *		ptclPoly;
	unsigned 	 	ptclFlags;
	Particle *		ptclNext;
	Particle *		ptclPrev;
	float			Scale;
	geVec3d			Gravity;
	float			Alpha;
	geVec3d			CurrentAnchorPoint;
	const geVec3d		*AnchorPoint;
	geFloat	 	 	ptclTime;
	geFloat			ptclTotalTime;
	geVec3d	 	        ptclVelocity;

	geBoolean UseSizeUp;
	float ScaleFrom;
	float ScaleTo;
}	Particle;

typedef	struct	Particle_System
{
	Particle *		psParticles;
	geWorld *		psWorld;
	geFloat			psLastTime;
	geFloat			psQuantumSeconds;
}	Particle_System;

/*
void Particle_SetTexture(Particle *p, geBitmap *Texture);
void  Particle_SystemRemoveAll(Particle_System *ps);
int32 	Particle_GetCount(Particle_System *ps);
void  Particle_SystemReset(Particle_System *ps);
*/

Particle_System *  Particle_SystemCreate(geWorld *World);
void 	Particle_SystemDestroy(Particle_System *ps);
void 	Particle_SystemFrame(Particle_System *ps, geFloat DeltaTime);
geBoolean Particle_SystemRemoveAnchorPoint(Particle_System *ps, geVec3d	*AnchorPoint );

void 	Particle_SystemAddParticle(Particle_System		*ps,
								   geBitmap	*Texture,
								   const GE_LVertex	*Vert,
								   const geVec3d		*AnchorPoint,
								   geFloat				Time,
								   const geVec3d		*Velocity,
								   float				Scale,
								   float				ScaleTo,
								   geBoolean			DoScale,
								   const geVec3d		*Gravity );


#endif
