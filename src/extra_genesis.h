

/*
extra_genesis.h
Some functions that helps loading and rendering of bitmaps,
and some basic AI look at functions
*/

#include "genesis.h"

#ifndef EXTRA_GENESIS_H
#define EXTRA_GENESIS_H

// convinient functions found on the forum, copied from Sevens projectZ code
float AnglesToPoint( geVec3d *s, geVec3d *d, geVec3d *a );
void LookAt(geVec3d from, geVec3d point, geVec3d *Result);

// my wrapper for Jeffs code
// Loads a bitmap, scales it and adds it to the engine. Returns the bitmaps is successfull, Returns 0 otherwise.
geBitmap* loadBitmapEx(char* fileName, int width, int height, geEngine* Engine);

geBitmap* loadBitmapExColorKey(char* fileName, int width, int height, geEngine* Engine, int idex);

// Loads the bitmap, sets the transparent key to zero (0) and adds the bitmap to the engine.
geBitmap* LoadBmp(char *filename);
geBitmap* LoadBmpNoColorKey(char *filename);

void killBitmap(geBitmap* bitmap);

// renders the whole bitmap
geBoolean renderBitmap(geBitmap *bmp, int x, int y);

geBoolean isWithinSquaredRange( geVec3d* pos, geVec3d* pos2, float squaredRange );
#endif