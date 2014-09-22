#ifndef __PICKUP_H
#define __PICKUP_H

#include "genesis.h"

geActor* pickup_getActorByIndex(geWorld* world, int type, geVec3d pos);
char pickup_loadActorDefinitions();
void pickup_clearActorDefinitions();
char pickup_handleActor(geActor* actor, geWorld* world, geVec3d* from, geVec3d* to);
char pickup_handlePickup(int index);

#endif // __PICKUP_H