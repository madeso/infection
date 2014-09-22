/******************************************************************************/
/*  StaticEntity.h                                                            */
/*                                                                            */
/*  Author: David Wulff                                                       */
/*  Description: Effect manager header                                        */
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

#ifndef SE_H
#define SE_H


#ifdef __cplusplus
extern "C" {
#endif

//	Static Entity proxy

#pragma GE_Type("Model.ico")
typedef struct _StaticEntityProxy
{
#pragma GE_Private
	geSound_Def *theSound;			// Handle of audio effect
	geSound *SoundHandle;				// For later modification of sound
	geActor *Actor;							// Actor for this entity
	geActor_Def *ActorDef;			// Actor def.
	int RunningTime;						// Timer for this static actor
	geBoolean bAnimated;				// Animation active flag
	geVec3d LastBBoxMin;				// Last bounding-box min. point
	geVec3d LastBBoxMax;				// Last bounding-box max. point
	int GravityTime;						// Used in gravity computations
	float TimeInAir;						// Time spent in the air, for gravity
	geBoolean bCollisionTest;		// Collision test enable/disable
	geBoolean bPlayingCollisionAnimation;	// TRUE if playing collision animation
	geBoolean bFollower;				// TRUE if entity is bound to a motion path
#pragma GE_Published
  geVec3d Origin;
	char *szSoundFile;
	float fRadius;							// Audio audible radius
	char *szActorFile;
	char *szEntityName;
	char *szDefaultAction;			// Default animation to play
	char *szImpactAction;				// Animation to play when collided with
	geBoolean bCanPush;					// Static entity can be pushed around
	int InitialAlpha;						// Transparency of actor
	geBoolean SubjectToGravity;	// TRUE if entity subject to gravity
	geFloat ScaleFactor;				// Factor to scale model by
	geBoolean FaceDestination;	// TRUE if entity rotated to face destination
	geVec3d ActorRotation;			// Rotation needed to properly orient actor
#pragma GE_Origin(Origin)
#pragma GE_DefaultValue(szSoundFile, "audio/st_entity.wav")
#pragma GE_DefaultValue(fRadius, "200.0")
#pragma GE_DefaultValue(szEntityName, "")
#pragma GE_DefaultValue(szActorFile, "actors/static.act")
#pragma GE_DefaultValue(szDefaultAction, "")
#pragma GE_DefaultValue(szImpactAction, "")
#pragma GE_DefaultValue(bCanPush, "False")
#pragma GE_DefaultValue(InitialAlpha, "255")
#pragma GE_DefaultValue(SubjectToGravity, "False")
#pragma GE_DefaultValue(ScaleFactor, "1.0")
#pragma GE_DefaultValue(FaceDestination, "False")
#pragma GE_DefaultValue(ActorRotation, "-89.55 3.14159268359 0.0")
#pragma GE_Documentation(szSoundFile, "Sound to play on collision/activation")
#pragma GE_Documentation(fRadius, "Audible range for sound")
#pragma GE_Documentation(szActorFile, "Actor file to load for this proxy")
#pragma GE_Documentation(szEntityName, "Name of entity (used in scripting and triggers)")
#pragma GE_Documentation(szDefaultAction, "Default animation, if any, for this prop")
#pragma GE_Documentation(szImpactAction, "Animation, if any, to play when collided with")
#pragma GE_Documentation(bCanPush, "TRUE if the player can push actor around")
#pragma GE_Documentation(InitialAlpha,"Transparency of actor, from 0-255 with 0=transparent")
#pragma GE_Documentation(SubjectToGravity, "TRUE if actor will be subject to gravity")
#pragma GE_Documentation(ScaleFactor, "Number to scale actor by")
#pragma GE_Documentation(FaceDestination, "TRUE if actor rotated to face destination")
#pragma GE_Documentation(ActorRotation, "Rotation needed to properly orient actor in game")
} StaticEntityProxy;

//	Particle System proxy

#pragma GE_Type("Model.ico")
typedef struct _ParticleSystemProxy
{
#pragma GE_Private
	geSound_Def *theSound;			// Handle of audio effect
	geSound *SoundHandle;				// For later modification of sound
	int psHandle;								// Particle system handle
	int pmHandle;								// Particle map handle
	geBoolean bFollower;				// TRUE if entity is bound to a motion path
#pragma GE_Published
  geVec3d Origin;
	int nStyle;									// Particle system style
	char *szSoundFile;					// Audio effect for particle system
	float fRadius;							// Audio audible radius
	char *szTexture;						// Texture for particles
	GE_RGBA clrColor;						// Color for particles
	int BirthRate;							// Over-ride default system birth rate
	int MaxParticleCount;				// Over-ride default max particle count
	int ParticleLifespan;				// Over-ride default particle lifespan
	char *szEntityName;					// Name for this particle system
#pragma GE_Origin(Origin)
#pragma GE_DefaultValue(nStyle, "2")
#pragma GE_DefaultValue(szSoundFile, "audio/psystem.wav")
#pragma GE_DefaultValue(fRadius, "200.0")
#pragma GE_DefaultValue(szTexture, "bitmaps/flare.bmp")
#pragma GE_DefaultValue(clrColor, "255 255 255")
#pragma GE_DefaultValue(BirthRate, "50")
#pragma GE_DefaultValue(MaxParticleCount, "10")
#pragma GE_DefaultValue(ParticleLifespan, "2000")
#pragma GE_DefaultValue(szEntityName, "")
#pragma GE_Documentation(nStyle, "Particle system style")
#pragma GE_Documentation(szSoundFile, "Audio loop to play")
#pragma GE_Documentation(fRadius, "Audible range for sound")
#pragma GE_Documentation(szTexture, "Texture file to use for particles")
#pragma GE_Documentation(clrColor, "Color for particles")
#pragma GE_Documentation(BirthRate, "Particle birth rate to use instead of system default")
#pragma GE_Documentation(MaxParticleCount, "Maximum particle count to use instead of system default")
#pragma GE_Documentation(ParticleLifespan, "Particle life span to use instead of system default")
#pragma GE_Documentation(szEntityName, "Name of entity (used in scripting and triggers)")
} ParticleSystemProxy;

#pragma warning( default : 4068 )
#endif
