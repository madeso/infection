#include "genesis.h"

#ifndef __FX_BITMAPS_H
#define __FX_BITMAPS_H

geBitmap	*stoneSmoke,
			*snowSmoke,
			*electricSmoke,
			*pipeSmoke,
			*gravelSmoke,
			*mudSmoke;

geBitmap	*marbleSprite,
			*grassSprite,
			*woodSprite,
			*glassSprite,
			*leafSprite,
			*sofaSprite,
			*rockSprite,
			*sandSprite;

geBitmap	*flameSprite,
			*smokeSprite,
			*gemSprite,
			*fireSprite;

typedef enum _smokeEmitterStyle{
	SES_NORMAL,
	SES_LARGE,
	SES_LONG_AND_LARGE
} SmokeEmitterStyle;

void fxbitmaps_init();
geBoolean fxbitmaps_load();
void fxbitmaps_free();
void createSmokeEmitter(geBitmap* bitmap, geVec3d from, geVec3d direction, SmokeEmitterStyle style);
void createExplosionSmokeEmitter(geBitmap* bitmap, geVec3d from, float waitTime);
void createElectricSparkle(geVec3d at);

void createSimpleSparkle(geVec3d at);
void ParticleExplosion(geBitmap* sprite, geVec3d location, geVec3d direction);

void BloodExplosion(geVec3d position);
void fx_blast(geVec3d from, geVec3d to);

void fx_grenadeExplosion(geVec3d at);
void fx_fire(geVec3d* at, geBoolean reference);
void fx_shootFire(geVec3d at, geVec3d direction);
#endif