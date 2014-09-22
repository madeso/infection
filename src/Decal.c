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
#include <stdlib.h>
#include "Genesis.h"
#include "Ram.h"
//#include "gebmutil.h"
#include "Decal.h"
#include "useful_macros.h"
#include "log.h"


// File Systems
geVFile *PakFS = NULL; // Pak File
geVFile *MainFS= NULL; // App Folder

// Get a random number between 1 and the passed number
#define GET_RAND(nNum) (int)(nNum * ((float)rand() / (float)RAND_MAX)) + 1

geBitmap* LoadBmpWithountEngine(char *filename)
{
	//local variables
	geVFile *File;
	geBitmap *Bitmap = 0;

	printLog("Loading bitmap: ");
	printLog(filename);
	printLog("\n");

	//open the file:
	File = geVFile_OpenNewSystem(NULL, GE_VFILE_TYPE_DOS, filename, NULL, GE_VFILE_OPEN_READONLY);
	if( !File )
	{
		printLog("Couldn't open file.\n");
		return 0;
	}
	Bitmap = geBitmap_CreateFromFile(File);
	geVFile_Close(File);


	if( geBitmap_SetColorKey(Bitmap, GE_TRUE, 0, GE_FALSE) == GE_FALSE )
	{
		printLog("Failed to set color key.\n");
		geBitmap_Destroy(&Bitmap);
		return 0;
	}
	printLog("Loading ok.\n");

	return Bitmap;
}

geBitmap* geBitmapUtil_CreateFromFileAndAlphaNames(geVFile* PakFS, char* DecalBitmap, char* DecalABitmap)
{
	geBitmap *bmp;
	geBitmap *alpha;

	bmp = LoadBmpWithountEngine(DecalBitmap);
	alpha = LoadBmpWithountEngine(DecalABitmap);

	if( !bmp || !alpha)
	{
		if(bmp)
			geBitmap_Destroy(&bmp);
		if(alpha)
			geBitmap_Destroy(&alpha);
		return 0;
	}

	if( !geBitmap_SetAlpha(bmp, alpha) )
	{
		if(bmp)
			geBitmap_Destroy(&bmp);
		if(alpha)
			geBitmap_Destroy(&alpha);
		return 0;
	}

	geBitmap_Destroy(&alpha);

	return bmp;
}

geBoolean gCreated = GE_FALSE;
DecalMgr *gdMgr = NULL;

// Decal bitmaps
static char DecalBitmaps[NUM_DECAL_TYPES][50] = {
	".\\gfx\\Decals\\bullet_1.Bmp",	// DECAL_BULLET1
	".\\gfx\\Decals\\bullet_2.Bmp",	// DECAL_BULLET2
	".\\gfx\\Decals\\bullet_3.Bmp",	// DECAL_BULLET3
	".\\gfx\\Decals\\bullet_4.Bmp",	// DECAL_BULLET4
	".\\gfx\\Decals\\bullet_big1.Bmp",	// DECAL_BULLETBIG1
	".\\gfx\\Decals\\bullet_big2.Bmp",	// DECAL_BULLETBIG2
	".\\gfx\\Decals\\bullet_big3.Bmp",	// DECAL_BULLETBIG3
	".\\gfx\\Decals\\scorch_sm1.Bmp",	// DECAL_SCORCHSM1
	".\\gfx\\Decals\\scorch_sm2.Bmp",	// DECAL_SCORCHSM2
	".\\gfx\\Decals\\scorch_med1.Bmp",	// DECAL_SCORCHMED1
	".\\gfx\\Decals\\scorch_big1.Bmp",	// DECAL_SCORCHBIG1
	".\\gfx\\Decals\\scorch_big2.Bmp",	// DECAL_SCORCHBIG2
	".\\gfx\\Decals\\scorch_big3.Bmp",	// DECAL_SCORCHBIG3
	".\\gfx\\Decals\\bullet_med1.Bmp",	// DECAL_BULLETMED1
	".\\gfx\\Decals\\bullet_med2.Bmp",	// DECAL_BULLETMED2
	".\\gfx\\Decals\\bullet_med3.Bmp",	// DECAL_BULLETMED3
	".\\gfx\\Decals\\bullet_med4.Bmp",	// DECAL_BULLETMED4
	".\\gfx\\Decals\\bullet_med5.Bmp",	// DECAL_BULLETMED5
	".\\gfx\\Decals\\bullet_xl1.Bmp",	// DECAL_BULLETXL1
	".\\gfx\\Decals\\bullet_xl2.Bmp",	// DECAL_BULLETXL2
	".\\gfx\\Decals\\bullet_xl3.Bmp",	// DECAL_BULLETXL3
	".\\gfx\\Decals\\bullet_xl4.Bmp",	// DECAL_BULLETXL4
	".\\gfx\\Decals\\bullet_xl5.Bmp",	// DECAL_BULLETXL5
	".\\gfx\\Decals\\bullet_xs1.Bmp",	// DECAL_BULLETXS1
	".\\gfx\\Decals\\bullet_xs2.Bmp",	// DECAL_BULLETXS2
	".\\gfx\\Decals\\bullet_xs3.Bmp",	// DECAL_BULLETXS3
	".\\gfx\\Decals\\bullet_xs4.Bmp",	// DECAL_BULLETXS4
	".\\gfx\\Decals\\shotgun_1.Bmp",	// DECAL_SHOTGUN1
	".\\gfx\\Decals\\shotgun_2.Bmp",	// DECAL_SHOTGUN2
	".\\gfx\\Decals\\shotgun_3.Bmp",	// DECAL_SHOTGUN3
	".\\gfx\\Decals\\shotgun_4.Bmp",	// DECAL_SHOTGUN4
	".\\gfx\\Decals\\shotgun_5.Bmp"	// DECAL_SHOTGUN5
};

// Decal Alpha bitmaps
static char DecalABitmaps[NUM_DECAL_TYPES][50] = {
	".\\gfx\\Decals\\bullet_1a.Bmp",	// DECAL_BULLET1
	".\\gfx\\Decals\\bullet_2a.Bmp",	// DECAL_BULLET2
	".\\gfx\\Decals\\bullet_3a.Bmp",	// DECAL_BULLET3
	".\\gfx\\Decals\\bullet_4a.Bmp",	// DECAL_BULLET4
	".\\gfx\\Decals\\bullet_big1a.Bmp",// DECAL_BULLETBIG1
	".\\gfx\\Decals\\bullet_big2a.Bmp",// DECAL_BULLETBIG2
	".\\gfx\\Decals\\bullet_big3a.Bmp",// DECAL_BULLETBIG3
	".\\gfx\\Decals\\scorch_sm1a.Bmp",	// DECAL_SCORCHSM1
	".\\gfx\\Decals\\scorch_sm2a.Bmp",	// DECAL_SCORCHSM2
	".\\gfx\\Decals\\scorch_med1a.Bmp",// DECAL_SCORCHMED1
	".\\gfx\\Decals\\scorch_big1a.Bmp",// DECAL_SCORCHBIG1
	".\\gfx\\Decals\\scorch_big2a.Bmp",// DECAL_SCORCHBIG2
	".\\gfx\\Decals\\scorch_big3a.Bmp",// DECAL_SCORCHBIG3
	".\\gfx\\Decals\\bullet_med1a.Bmp",// DECAL_BULLETMED1
	".\\gfx\\Decals\\bullet_med2a.Bmp",// DECAL_BULLETMED2
	".\\gfx\\Decals\\bullet_med3a.Bmp",// DECAL_BULLETMED3
	".\\gfx\\Decals\\bullet_med4a.Bmp",// DECAL_BULLETMED4
	".\\gfx\\Decals\\bullet_med5a.Bmp",// DECAL_BULLETMED5
	".\\gfx\\Decals\\bullet_xl1a.Bmp",	// DECAL_BULLETXL1
	".\\gfx\\Decals\\bullet_xl2a.Bmp",	// DECAL_BULLETXL2
	".\\gfx\\Decals\\bullet_xl3a.Bmp",	// DECAL_BULLETXL3
	".\\gfx\\Decals\\bullet_xl4a.Bmp",	// DECAL_BULLETXL4
	".\\gfx\\Decals\\bullet_xl5a.Bmp",	// DECAL_BULLETXL5
	".\\gfx\\Decals\\bullet_xs1a.Bmp",	// DECAL_BULLETXS1
	".\\gfx\\Decals\\bullet_xs2a.Bmp",	// DECAL_BULLETXS2
	".\\gfx\\Decals\\bullet_xs3a.Bmp",	// DECAL_BULLETXS3
	".\\gfx\\Decals\\bullet_xs4a.Bmp",	// DECAL_BULLETXS4
	".\\gfx\\Decals\\shotgun_1a.Bmp",	// DECAL_SHOTGUN1
	".\\gfx\\Decals\\shotgun_2a.Bmp",	// DECAL_SHOTGUN2
	".\\gfx\\Decals\\shotgun_3a.Bmp",	// DECAL_SHOTGUN3
	".\\gfx\\Decals\\shotgun_4a.Bmp",	// DECAL_SHOTGUN4
	".\\gfx\\Decals\\shotgun_5a.Bmp"	// DECAL_SHOTGUN5
};

// Decal sizes
static int DecalSizes[NUM_DECAL_TYPES] = {
	16,		// DECAL_BULLET1
	16,		// DECAL_BULLET2
	16,		// DECAL_BULLET3
	16,		// DECAL_BULLET4
	16,		// DECAL_BULLETBIG1
	16,		// DECAL_BULLETBIG2
	16,		// DECAL_BULLETBIG3
	32,		// DECAL_SCORCHSM1
	32,		// DECAL_SCORCHSM2
	64,		// DECAL_SCORCHMED1
	128,	// DECAL_SCORCHBIG1
	128,	// DECAL_SCORCHBIG2
	128,	// DECAL_SCORCHBIG3
	16,		// DECAL_BULLETMED1
	16,		// DECAL_BULLETMED2
	16,		// DECAL_BULLETMED3
	16,		// DECAL_BULLETMED4
	16,		// DECAL_BULLETMED5
	16,		// DECAL_BULLETXL1
	16,		// DECAL_BULLETXL2
	16,		// DECAL_BULLETXL3
	16,		// DECAL_BULLETXL4
	16,		// DECAL_BULLETXL5
	16,		// DECAL_BULLETXS1
	16,		// DECAL_BULLETXS2
	16,		// DECAL_BULLETXS3
	16,		// DECAL_BULLETXS4
	32,		// DECAL_SHOTGUN1
	32,		// DECAL_SHOTGUN2
	32,		// DECAL_SHOTGUN3
	32,		// DECAL_SHOTGUN4
	32		// DECAL_SHOTGUN5
};

// Return a random decal number based on the decal type
// .. very big and hard to follow, but then again i'm very lazy :)
int DecalMgr_GetRandomDecal(int nType)
{
	switch(nType)
	{
		default:
		case DECALTYPE_BULLET:
		{
			switch (GET_RAND(4))
			{
				default:
				case 1:
					return DECAL_BULLET1;
				case 2:
					return DECAL_BULLET2;
				case 3:
					return DECAL_BULLET3;
				case 4:
					return DECAL_BULLET4;
			}
		}
		case DECALTYPE_BULLETBIG:
		{
			switch (GET_RAND(3))
			{
			default:
			case 1:
				return DECAL_BULLETBIG1;
			case 2:
				return DECAL_BULLETBIG2;
			case 3:
				return DECAL_BULLETBIG3;
			}
		}
		case DECALTYPE_SCORCHSM:
		{
			switch (GET_RAND(2))
			{
			default:
			case 1:
				return DECAL_SCORCHSM1;
			case 2:
				return DECAL_SCORCHSM2;
			}
		}
		case DECALTYPE_SCORCHMED:
			return DECAL_SCORCHMED1;
		case DECALTYPE_SCORCHBIG:
		{
			switch (GET_RAND(3))
			{
			default:
			case 1:
				return DECAL_SCORCHBIG1;
			case 2:
				return DECAL_SCORCHBIG2;
			case 3:
				return DECAL_SCORCHBIG3;
			}
		}
		case DECALTYPE_BULLETMED:
		{
			switch (GET_RAND(5))
			{
			default:
			case 1:
				return DECAL_BULLETMED1;
			case 2:
				return DECAL_BULLETMED2;
			case 3:
				return DECAL_BULLETMED3;
			case 4:
				return DECAL_BULLETMED4;
			case 5:
				return DECAL_BULLETMED5;
			}
		}
		case DECALTYPE_BULLETXL:
		{
			switch (GET_RAND(5))
			{
			default:
			case 1:
				return DECAL_BULLETXL1;
			case 2:
				return DECAL_BULLETXL2;
			case 3:
				return DECAL_BULLETXL3;
			case 4:
				return DECAL_BULLETXL4;
			case 5:
				return DECAL_BULLETXL5;
			}
		}
		case DECALTYPE_BULLETXS:
		{
			switch (GET_RAND(4))
			{
			default:
			case 1:
				return DECAL_BULLETXS1;
			case 2:
				return DECAL_BULLETXS2;
			case 3:
				return DECAL_BULLETXS3;
			case 4:
				return DECAL_BULLETXS4;
			}
		}
		case DECALTYPE_SHOTGUN:
		{
			switch (GET_RAND(5))
			{
			default:
			case 1:
				return DECAL_SHOTGUN1;
			case 2:
				return DECAL_SHOTGUN2;
			case 3:
				return DECAL_SHOTGUN3;
			case 5:
				return DECAL_SHOTGUN4;
			case 4:
				return DECAL_SHOTGUN5;
			}
		}
	}
	
	return 0;
}

// Called to create the manager, and allocate the momory for the decals
DecalMgr *DecalMgr_Create(geEngine *Engine, geWorld *world)
{
	DecalMgr *dMgr = ALLOCATE_STRUCT(DecalMgr);
	int i;
	
	dMgr->World = world;
	dMgr->Engine = Engine;
	dMgr->Current = 0;

	// if we are already created, destroy our old self before continuiung
	if (gCreated == GE_TRUE)
		DecalMgr_Destroy(gdMgr);
	
	// allocate memory for each decal
	for (i = 0; i < NUM_DECALS; i++)
	{
		dMgr->dcl[i] = ALLOCATE_STRUCT(Decal);
		dMgr->dcl[i]->Visible = GE_FALSE; // so we don't draw it
	}
	
	for (i = 0; i < NUM_DECAL_TYPES; i++)
	{
		dMgr->Bitmap[i] = geBitmapUtil_CreateFromFileAndAlphaNames(PakFS, DecalBitmaps[i], DecalABitmaps[i]);
		
		if (!dMgr->Bitmap[i])
		{
			printLog("DecalMgr_Create: CreateFromFileAndAlphaNames failed: ");
			printLog(DecalBitmaps[i]);
			printLog(", ");
			printLog(DecalABitmaps[i]);
			printLog("\n");
		}
		
		geWorld_AddBitmap(world, dMgr->Bitmap[i]);
	}
	
	gdMgr = dMgr;
	gCreated = GE_TRUE;
	
	return dMgr;
}

// Called to destory the manager and all attached decals
void DecalMgr_Destroy(DecalMgr *dMgr)
{
	int i;
	
	// free each bitmap
	for (i = 0; i < NUM_DECAL_TYPES; i++)
	{
		geBitmap_Destroy(&dMgr->Bitmap[i]);
		dMgr->Bitmap[i] = NULL;
	}
	
	// free each decal
	for (i = 0; i < NUM_DECALS; i++)
		free(dMgr->dcl[i]);
	
	// now free the manager
	free(dMgr);
	
	gCreated = GE_FALSE;
}

// Called to add a new decal to the manager
geBoolean DecalMgr_AddDecal(DecalMgr *dMgr, int type, float timeToLive, GE_RGBA rgba, float percent,
					   geVec3d *impact, geVec3d *normal, geVec3d *In)
{
	int i, major = 0;
	geVec3d right, up;
	geVec3d Axis[3] =
	{
		{1.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 1.0f}
	};
	
	// return true (successfull) since we don't want to cause any problem when the manager isn't created
	// when the manager is zero it means that we don't want to use the decals
	if( dMgr== 0 ) return GE_TRUE;

	// see if we need to overwrite the oldest decal
	dMgr->Current++;
	
	if (dMgr->Current >= NUM_DECALS)
		dMgr->Current = 0;
	
	if (dMgr->dcl[dMgr->Current]->bAdded == GE_TRUE)
	{/*
		geBoolean res;
		res = geWorld_PolyIsValid(dMgr->dcl[dMgr->Current]->Poly);*/
		dMgr->dcl[dMgr->Current]->bAdded = GE_FALSE;
		// sg addon
		if( dMgr->dcl[dMgr->Current]->Poly )
		geWorld_RemovePoly(dMgr->World, dMgr->dcl[dMgr->Current]->Poly);
	}
	
	if (!dMgr->Bitmap[type])
		return GE_FALSE;
	
	dMgr->dcl[dMgr->Current]->TimeToLive = timeToLive;
	
	// Note:  The following code is based on code by Ralph Deane
	// Setup vertex 1,2,3,4
	for(i = 0; i < 4; i++)
	{
		// texture coordinates
		dMgr->dcl[dMgr->Current]->vertex[i].u = 0.0f;
		dMgr->dcl[dMgr->Current]->vertex[i].v = 0.0f;
		// color
		dMgr->dcl[dMgr->Current]->vertex[i].r = rgba.r;
		dMgr->dcl[dMgr->Current]->vertex[i].g = rgba.g;
		dMgr->dcl[dMgr->Current]->vertex[i].b = rgba.b;
		dMgr->dcl[dMgr->Current]->vertex[i].a = rgba.a;
	}
	
	dMgr->dcl[dMgr->Current]->vertex[3].u = 1.0f;
	dMgr->dcl[dMgr->Current]->vertex[2].u = 1.0f;
	dMgr->dcl[dMgr->Current]->vertex[2].v = 1.0f;
	dMgr->dcl[dMgr->Current]->vertex[1].v = 1.0f;

	
	#define fab(a) (a > 0 ? a : -a)
	
	if(fab(normal->Y) > fab(normal->X))
	{
		major = 1;
		if(fab(normal->Z) > fab(normal->Y))
			major = 2;
	}
	else
	{
		if(fab(normal->Z) > fab(normal->X))
			major = 2;
	}
	
	if(fab(normal->X)==1.0f || fab(normal->Y)==1.0f || fab(normal->Z)==1.0f)
	{
		if ((major == 0 && normal->X > 0) || major == 1)
		{
			right.X = 0.0f;
			right.Y = 0.0f;
			right.Z = -1.0f;
		}
		else if (major == 0)
		{
			right.X = 0.0f;
			right.Y = 0.0f;
			right.Z = 1.0f;
		}
		else 
		{
			right.X = normal->Z;
			right.Y = 0.0f;
			right.Z = 0.0f;
		}
	}
	else 
		geVec3d_CrossProduct(&Axis[major], normal, &right);
	
	geVec3d_CrossProduct(normal, &right, &up);
	geVec3d_Normalize(&up);
	geVec3d_Normalize(&right);
	
	geVec3d_Scale(&right, (DecalSizes[type] * percent) / 2.0f, &right); 
	geVec3d_Scale(&up, (DecalSizes[type] * percent) / 2.0f, &up);
	
	geVec3d_MA(impact, 0.1f, normal, impact);
	
	//calculate vertices from corners
	dMgr->dcl[dMgr->Current]->vertex[1].X = impact->X + ((-right.X - up.X));
	dMgr->dcl[dMgr->Current]->vertex[1].Y = impact->Y + ((-right.Y - up.Y));
	dMgr->dcl[dMgr->Current]->vertex[1].Z = impact->Z + ((-right.Z - up.Z));
	
	dMgr->dcl[dMgr->Current]->vertex[2].X = impact->X + ((right.X - up.X));
	dMgr->dcl[dMgr->Current]->vertex[2].Y = impact->Y + ((right.Y - up.Y));
	dMgr->dcl[dMgr->Current]->vertex[2].Z = impact->Z + ((right.Z - up.Z));
	
	dMgr->dcl[dMgr->Current]->vertex[3].X = impact->X + ((right.X + up.X));
	dMgr->dcl[dMgr->Current]->vertex[3].Y = impact->Y + ((right.Y + up.Y));
	dMgr->dcl[dMgr->Current]->vertex[3].Z = impact->Z + ((right.Z + up.Z));
	
	dMgr->dcl[dMgr->Current]->vertex[0].X = impact->X + ((-right.X + up.X));
	dMgr->dcl[dMgr->Current]->vertex[0].Y = impact->Y + ((-right.Y + up.Y));
	dMgr->dcl[dMgr->Current]->vertex[0].Z = impact->Z + ((-right.Z + up.Z));
	
	geWorld_GetLeaf(dMgr->World, impact, &dMgr->dcl[dMgr->Current]->Leaf);
	// End Ralph Deane's code
	
	dMgr->dcl[dMgr->Current]->type = type;
	
	// Ok, now we hit a problem.  If the impact point (the center of the
	// .. four verticies is on the very edge of (for example) a doorway,
	// .. the decal will currently be drawn overhanging into the doorway
	// .. - not very realistic!  To overcome this, we could use complicated
	// .. clipping techniques, or we could simply not draw the decal if it
	// .. won't fit completely against the passed collision plane.  I have
	// .. chose the latter approach - it's a hack, but it works, and for all
	// .. intents and purposes it really doesn't matter.
	{
		// I really didn't know how to approach this, so I am doing collision
		// .. tests for all four corners to see if they lie on the same
		// .. impact plane.  I have no idea what effect this will have on
		// .. performance if you have more than 500 decals on screen at once.
		// Please, if you find a better way, tell me [dwulff@bttlxe.co.uk]
		GE_Collision Collision[4];
		geVec3d Plane[4], Back, Corner;
		
		for (i = 0; i < 4; i++)
		{
			// fill a geVec3d with the current corner
			Corner.X = dMgr->dcl[dMgr->Current]->vertex[i].X;
			Corner.Y = dMgr->dcl[dMgr->Current]->vertex[i].Y;
			Corner.Z = dMgr->dcl[dMgr->Current]->vertex[i].Z;
			
			geVec3d_AddScaled(&Corner, In, 100.0f, &Back);
			
			if (geWorld_Collision(dMgr->World, NULL, NULL, &Corner, &Back, GE_CONTENTS_CANNOT_OCCUPY,
				GE_COLLIDE_MODELS, 0xffffffff, NULL, NULL, &Collision[i]))
				Plane[i] = Collision[i].Plane.Normal;
		}
		
		// Check to see if all four verticies lie on the same plane, and if so
		// .. mark the decal as valid
		if (geVec3d_Compare(&Plane[0], normal, 0.0f) &&
			geVec3d_Compare(&Plane[1], normal, 0.0f) &&
			geVec3d_Compare(&Plane[2], normal, 0.0f) &&
			geVec3d_Compare(&Plane[3], normal, 0.0f))
			dMgr->dcl[dMgr->Current]->Visible = GE_TRUE;
	}
	
	return GE_TRUE;
}

// Called each frame to add the decal as a polygon (AddPolyOnce)
void DecalMgr_Tick(DecalMgr *dMgr, float dwTicks)
{
	int i;
		
	for (i = 0; i < NUM_DECALS; i++)
	{
		if (dMgr->dcl[i]->Visible != GE_TRUE)
			continue;
		
		if (dMgr->dcl[i]->TimeToLive != -1 && dMgr->dcl[i]->TimeToLive - dwTicks != -1)
			dMgr->dcl[i]->TimeToLive -= dwTicks;
		
		// only draw if we aren't expired or 'permanent'
		if(dMgr->dcl[i]->TimeToLive > 0.0 || dMgr->dcl[i]->TimeToLive == -1)
		{
			if(geWorld_MightSeeLeaf(dMgr->World, dMgr->dcl[i]->Leaf) == GE_TRUE)
			{
				// if we are not already added, add the poly to the engine
				if (dMgr->dcl[i]->bAdded != GE_TRUE)
				{
					dMgr->dcl[i]->Poly = geWorld_AddPoly(dMgr->World,
						dMgr->dcl[i]->vertex,
						4,
						dMgr->Bitmap[dMgr->dcl[i]->type],
						GE_TEXTURED_POLY,
						GE_RENDER_DEPTH_SORT_BF ,
						1.0f);
					dMgr->dcl[i]->bAdded = GE_TRUE;
				}
			}
		}
		else
		{
			// The decal has expired, so flag it as hidden so it can be re-used
			dMgr->dcl[i]->Visible = GE_FALSE;
			
			// remove the poly from the world
			// sg addon
			if( dMgr->dcl[i]->Poly )
			geWorld_RemovePoly(dMgr->World, dMgr->dcl[i]->Poly);
		}
	}
}

// Called each frame to add the decal as a polygon (AddPolyOnce)
void DecalMgr_Clear(DecalMgr *dMgr)
{
	int i;
		
	for (i = 0; i < NUM_DECALS; i++)
	{
		//if (dMgr->dcl[i]->Visible != GE_TRUE)
		if (dMgr->dcl[i]->bAdded != GE_TRUE)
			continue;
		
		{
			// The decal has expired, so flag it as hidden so it can be re-used
			dMgr->dcl[i]->Visible = GE_FALSE;
			
			// remove the poly from the world
			geWorld_RemovePoly(dMgr->World, dMgr->dcl[i]->Poly);
			dMgr->dcl[i]->Poly = 0;
			dMgr->dcl[i]->bAdded = GE_FALSE;
		}
	}
}