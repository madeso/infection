#ifndef __EXPLOSIONS_H
#define __EXPLOSIONS_H

int explosion_getDamage(geVec3d* explosionLocation, geVec3d* pos, float range, int damage);
void explosion_at(geVec3d* location, float range, int damage);

#endif