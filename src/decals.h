/*
decals.h
Basic variables of the decal and effect manager
*/

#ifndef DECALS_H
#define DECALS_H

// Decal includes
#include "Decal.h"

// Effect manager includes
#include "EffManager.h" 
#include "e_rain.h" 
#include "e_spout.h" 
#include "e_sprite.h"


//DecalManager
DecalMgr *dMgr;
//Bullet decal color
GE_RGBA RGBA_Array;
//effect manager
Eff_Manager *EM;

#endif