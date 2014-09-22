#ifndef __RECOIL_H
#define __RECOIL_H

#include "genesis.h"

void recoil_clear();
void recoil_move(geVec3d* angles, float time);
void recoil_give(float force);

#endif