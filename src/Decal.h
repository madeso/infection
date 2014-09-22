/******************************************************************************/
/*  Decal.c                                                                   */
/*                                                                            */
/*  Author: Ralph Deane, David Wulff                                          */
/*  Description: Decal management                                             */
/*                                                                            */
/*  Copyright (c) 1999-2001, Battleaxe Studios; All rights reserved.          */
/*                                                                            */
/*  Project: Bravo One                                                        */
/*  Web-site: www.bttlxe.co.uk                                                */
/******************************************************************************/
#ifndef __DECAL_H_
#define __DECAL_H_

// The maximum number of decals in your world at any one time.  Note, I have
// .. tested with as many as 500 on screen at once, and not had any noticable
// .. performance drop.
#define NUM_DECALS		150

typedef enum
{
	DECALTYPE_BULLET,
	DECALTYPE_BULLETBIG,
	DECALTYPE_SCORCHSM,
	DECALTYPE_SCORCHMED,
	DECALTYPE_SCORCHBIG,
	DECALTYPE_BULLETMED,
	DECALTYPE_BULLETXL,
	DECALTYPE_BULLETXS,
	DECALTYPE_SHOTGUN
	
} E_DecalTypes;

typedef enum
{
	DECAL_BULLET1 = 0,
	DECAL_BULLET2,
	DECAL_BULLET3,
	DECAL_BULLET4,
	DECAL_BULLETBIG1,
	DECAL_BULLETBIG2,
	DECAL_BULLETBIG3,
	DECAL_SCORCHSM1,
	DECAL_SCORCHSM2,
	DECAL_SCORCHMED1,
	DECAL_SCORCHBIG1,
	DECAL_SCORCHBIG2,
	DECAL_SCORCHBIG3,
	DECAL_BULLETMED1,
	DECAL_BULLETMED2,
	DECAL_BULLETMED3,
	DECAL_BULLETMED4,
	DECAL_BULLETMED5,
	DECAL_BULLETXL1,
	DECAL_BULLETXL2,
	DECAL_BULLETXL3,
	DECAL_BULLETXL4,
	DECAL_BULLETXL5,
	DECAL_BULLETXS1,
	DECAL_BULLETXS2,
	DECAL_BULLETXS3,
	DECAL_BULLETXS4,
	DECAL_SHOTGUN1,
	DECAL_SHOTGUN2,
	DECAL_SHOTGUN3,
	DECAL_SHOTGUN4,
	DECAL_SHOTGUN5,
	
	NUM_DECAL_TYPES
	
} E_Decals;


// We have a fixed number of decals (1 x NUM_DECALS).  If the maximum is
// .. reached, the next add_decal will overwrite the oldest.  This ensures
// .. we never have too many decals present at a time (after all, they are
// .. only eye-candy).

typedef struct	Decal
{
	float			TimeToLive;
	GE_LVertex		vertex[4];
	long			Leaf;
	geBoolean		Visible;
	int				type;
	geBoolean		bAdded;
	gePoly			*Poly;
	
} Decal;

typedef struct T_DecalMgr
{
	Decal			*dcl[NUM_DECALS];
	geWorld			*World;
	geEngine		*Engine;
	geBitmap		*Bitmap[NUM_DECAL_TYPES];
	int				Current;
	
} DecalMgr;

int DecalMgr_GetRandomDecal(int nType);
DecalMgr *DecalMgr_Create(geEngine *Engine, geWorld *world);
void DecalMgr_Destroy(DecalMgr *dMgr);
geBoolean DecalMgr_AddDecal(DecalMgr *dMgr, int type, float timeToLive, GE_RGBA rgba, float percent,
					   geVec3d *impact, geVec3d *normal, geVec3d *In);
void DecalMgr_Tick(DecalMgr *dMgr, float dwTicks);
void DecalMgr_Clear(DecalMgr *dMgr);

#endif


