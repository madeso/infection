/******************************************************************************/
/*  EffManager.h                                                              */
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

#ifndef EFFECTMANAGER_H
#define EFFECTMANAGER_H

#include "tpool.h"
#include "spool.h"
#include "Decal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_EFF_ITEMS   1000  // Maximun number of effects

#define EFF_LIGHT       1
#define EFF_SPRAY       2
#define EFF_SPRITE      3
#define EFF_SND         4
#define EFF_BOLT        5
#define EFF_CORONA      6
#define EFF_BLOOD		7

#define EFFECTC_CLIP_LEAF		( 1 << 0 )
#define EFFECTC_CLIP_LINEOFSIGHT	( 1 << 1 )
#define EFFECTC_CLIP_SEMICIRCLE		( 1 << 2 )

#define SPRAY_SOURCE		( 1 << 0 )
#define SPRAY_DEST		( 1 << 1 )
#define SPRAY_FOLLOWTAIL	( 1 << 2 )

typedef struct
{
	geVec3d		Position;
	GE_RGBA		Color;
	geVec3d		Gravity;
	geVec3d		Velocity;
	geBitmap	*Texture;
	float		TextureScale;
	float		life;

	GE_RGBA		decalColor;
	int			decalType;
	DecalMgr	*mgr;
	geExtBox	ExtBox;
} Blood;

typedef enum
{
	SSS_RANDOM_SCALE,
	SSS_SIZE_UP
} Spray_ScaleStyle;

typedef struct
{
	geVec3d		*ParticleGravity;	// RESERVED
	float		TimeRemaining;		// RESERVED
	float		PolyCount;		// RESERVED
	geXForm3d	Xf;			// RESERVED
	GE_LVertex	Vertex;			// RESERVED
	geBoolean	Paused;			// RESERVED, whether or not the sprite is paused
	int32		Leaf;			// RESERVED
	geBoolean	ShowAlways;		// RESERVED
	geBitmap	*Texture;		// texture to use
	float		SprayLife;		// life of effect
	float		Rate;			// add a new texture every "Rate" seconds
	geVec3d		*AnchorPoint;		// point to which particles are hooked to
	GE_RGBA		ColorMin;		// min values for each color
	GE_RGBA		ColorMax;		// max values for each color
	geVec3d		Gravity;		// gravity vector
	float		DistanceMax;		// distance past which the effect is not drawn
	float		DistanceMin;		// distance up to which no level of detail processing is done
	geVec3d		Source;			// source point
	int		SourceVariance;		// +/- units to vary the source point
	geVec3d		Dest;			// dest point
	int		DestVariance;		// +/- units to vary the dest point
	float		MinScale;		// min scale for the art
	float		MaxScale;		// max scale for the art
	float		MinUnitLife;		// min life of each texture
	float		MaxUnitLife;		// max life of each texture
	float		MinSpeed;		// min speed of each texture
	float		MaxSpeed;		// max speed of each texture
	Spray_ScaleStyle ScaleStyle;
	float waitTime;
} Spray;

#define GLOW_POS	( 1 << 0 )
#define GLOW_RADIUSMIN	( 1 << 1 )
#define GLOW_RADIUSMAX	( 1 << 2 )
#define GLOW_INTENSITY	( 1 << 3 )

typedef struct
{
	geLight		*Light;			// RESERVED
	int32		Leaf;			// RESERVED
	geVec3d		Pos;			// vector which light will follow
	float		RadiusMin;		// min light radius
	float		RadiusMax;		// max light radius
	GE_RGBA		ColorMin;		// min color info
	GE_RGBA		ColorMax;		// max color info
	float		Intensity;		// light intensity
	geBoolean	DoNotClip;		// whether or not clipping should be ignored
	geBoolean	CastShadows;	// whether or not the light should cast shadows

} Glow;

#define SPRITE_POS	( 1 << 0 )
#define SPRITE_SCALE	( 1 << 1 )
#define SPRITE_ROTATION	( 1 << 2 )
#define SPRITE_COLOR	( 1 << 3 )

typedef enum
{
	SPRITE_CYCLE_NONE,
	SPRITE_CYCLE_RESET,
	SPRITE_CYCLE_REVERSE,
	SPRITE_CYCLE_RANDOM,
	SPRITE_CYCLE_ONCE

} Sprite_CycleStyle;

typedef struct
{
	GE_LVertex	Vertex[4];	// RESERVED, vert info
	int32		Direction;	// RESERVED
	float		ElapsedTime;	// RESERVED
	int32		CurrentTexture;	// RESERVED
	int32		Leaf;		// RESERVED
	geBoolean	Paused;		// RESERVED, whether or not the sprite is paused
	geBoolean	ShowAlways;		// RESERVED
	geVec3d		Pos;		// location
	GE_RGBA		Color;		// color
	geBitmap	**Texture;	// list of textures to use
	int32		TotalTextures;	// total number of textures in the list
	float		TextureRate;	// every how many seconds to switch to the next texture
	float		Scale;		// how to scale the art
	float		ScaleRate;	// how much to subtract from scale each second
	float		RotationRate;	// how much to add to art rotation each second (radians)
	float		AlphaRate;	// how much to subtract from alpha each second
	Sprite_CycleStyle	Style;	// how to cycle through the images
	float		Rotation;	// art rotation amount (radians)
} Sprite;

#define SND_POS		( 1 << 0 )
#define SND_MINAUDIBLEVOLUME	0.1f

typedef struct
{
	geSound		*Sound;			// RESERVED, pointer to the active sound
	geBoolean	Paused;
	geFloat		LastVolume;		// RESERVED, its volume the last time it was modified
	geFloat		LastPan;		// RESERVED, its pan the last time it was modified
	geSound_Def	*SoundDef;		// sound def to play from
	geVec3d		Pos;			// location of the sound
	geFloat		Min;			// min distance whithin which sound is at max volume
	geBoolean	Loop;			// whether or not to loop it

} Snd;

#define BOLT_START	( 1 << 0 )
#define BOLT_END	( 1 << 1 )
#define BOLT_COLOR	( 1 << 2 )

typedef struct
{
	GE_LVertex	Vertex[4];
	geVec3d		Current;
	geVec3d		LastA0;
	geVec3d		LastB0;
	geVec3d		LastA3;
	geVec3d		LastB3;
	int32		BoltCount;
	int32		CurrentBolt;
	gePoly		**Poly;
	float		*BoltLifeList;
	float		BoltDelay;
	geBoolean	EndReached;

} Reserved;

typedef struct Bolt
{
	Reserved	*ReservedData;	// RESERVED
	geBitmap	*Texture;	// art to use
	float		CompleteLife;	// life of entire bolt
	geVec3d		Start;		// starting position
	geVec3d		End;		// ending position
	int		SegmentLength;	// length of each bolt segment
	float		SegmentWidth;	// width of each bolt segment
	float		Offset;		// how much each bolt will vary off the most direct path
	int		BoltLimit;	// max number of individual bolts that can exist
	float		BoltLife;	// how low each bolt can last (in seconds)
	float		BoltCreate;	// every how many seconds to create a new bolt
	geBoolean	Loop;		// whether or not the lighting loops continously
	GE_RGBA		Color;		// color of the complete electric bolt

} Bolt;

#define CORONA_POS	( 1 << 0 )

typedef struct
{
	float		LastVisibleRadius;	// RESERVED, last visible radius
	int32		Leaf;			// RESERVED, leaf it resides in
	geBoolean	Paused;			// RESERVED, whether or not the sprite is paused
	geBitmap	*Texture;		// texture to use
	GE_LVertex	Vertex;			// location and color info
	geFloat		FadeTime;		// how many seconds to spend fading away the corona
        float		MinRadius;		// mix corona radius
        float		MaxRadius;		// max corona radius
	float		MaxRadiusDistance;	// above this distance, corona is capped at MaxRadius
	float		MinRadiusDistance;	// below this distance, corona is capped at MinRadius
	float		MaxVisibleDistance;	// beyond this distance the corona is not visible

} EffCorona;

typedef	struct	Particle_System	Particle_System;

typedef struct Eff_Item
{
  geBoolean	Active;
  int		Type;
  void *        Data;
  geBoolean	Pause;
} Eff_Item;

typedef struct Eff_Manager
{
  geEngine *Engine;
  geWorld *World;
  geCamera *Camera;
  geSound_System	*AudioSystem;
  Eff_Item      Item[MAX_EFF_ITEMS];
  Particle_System *Ps;
  int m_LastTick;	// Internal timing counter
} Eff_Manager;


geBoolean EM_ParticleExplosion(Eff_Manager* EM, geVec3d location, geVec3d direction, geBitmap* image, float speed, geVec3d gravity, float change, float speedMultiply, int count, float life, float scale);

Eff_Manager *EM_Create(geEngine *Engine, geCamera *Camera, geWorld *World);
EM_Destroy(Eff_Manager *EM);	
geBoolean EM_Tick(Eff_Manager *EM, float timePassed);
int EM_Item_Add(Eff_Manager *EM, int Itype, void *Idata);
void EM_Item_Modify(Eff_Manager *EM, int Itype, int Index, void *Data, uint32 Flags);
void EM_Item_Delete(Eff_Manager *EM, int Itype, int Index);
void EM_Item_Pause(Eff_Manager *EM, int Itype, int Index, geBoolean Flag);


void *EM_Spray_Add(Eff_Manager *EM, void *Data);
void EM_Spray_Remove(Eff_Manager *EM, Spray *Data);
geBoolean EM_Spray_Process(Eff_Manager *EM, Spray  *Data,  float  TimeDelta);
geBoolean EM_Spray_Modify(Eff_Manager *EM, Spray *Data, Spray *NewData, uint32 Flags);

void *EM_Glow_Add(Eff_Manager *EM, void *Data);
void EM_Glow_Remove(Eff_Manager *EM, Glow *Data);
geBoolean EM_Glow_Process(Eff_Manager *EM, Glow  *Data,  float  TimeDelta);
geBoolean EM_Glow_Modify(Eff_Manager *EM, Glow *Data, Glow *NewData, uint32 Flags);

void *EM_Sprite_Add(Eff_Manager *EM, void *Data);
void EM_Sprite_Remove(Eff_Manager *EM, Sprite *Data);
geBoolean EM_Sprite_Process(Eff_Manager *EM, Sprite  *Data,  float  TimeDelta);
geBoolean EM_Sprite_Modify(Eff_Manager *EM, Sprite *Data, Sprite *NewData, uint32 Flags);

void *EM_Snd_Add(Eff_Manager *EM, void *Data);
void EM_Snd_Remove(Eff_Manager *EM, Snd *Data);
geBoolean EM_Snd_Process(Eff_Manager *EM, Snd  *Data,  float  TimeDelta);
geBoolean EM_Snd_Modify(Eff_Manager *EM, Snd *Data, Snd *NewData, uint32 Flags);

void *EM_Bolt_Add(Eff_Manager *EM, void *Data);
void EM_Bolt_Remove(Eff_Manager *EM, Bolt *Data);
geBoolean EM_Bolt_Process(Eff_Manager *EM, Bolt  *Data,  float  TimeDelta);
geBoolean EM_Bolt_Modify(Eff_Manager *EM, Bolt *Data, Bolt *NewData, uint32 Flags);
void EM_Bolt_Pause(Eff_Manager *EM, Bolt *Data, geBoolean Pause );

void *EM_Corona_Add(Eff_Manager *EM, void *Data);
void EM_Corona_Remove(Eff_Manager *EM, EffCorona *Data);
geBoolean EM_Corona_Process(Eff_Manager *EM, EffCorona  *Data,  float  TimeDelta);
geBoolean EM_Corona_Modify(Eff_Manager *EM, EffCorona *Data, EffCorona *NewData, uint32 Flags);

void *EM_Blood_Add(Eff_Manager *EM, void *Data);
void EM_Blood_Remove(Eff_Manager *EM, Blood *Data);
geBoolean EM_Blood_Process(Eff_Manager *EM, Blood *Data,  float  TimeDelta);
geBoolean EM_Blood_Modify(Eff_Manager *EM, Blood *Data, Blood *NewData, uint32 Flags);

#endif
