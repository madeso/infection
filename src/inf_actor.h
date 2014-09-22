/*
inf_actor.h
enemy actor definitions holder and convinient functions for load ing and destroying actors
*/

#include "genesis.h"

#ifndef INF_ACTOR_H
#define INF_ACTOR_H

#define NUMBER_OF_CIVILIANS		3

geActor_Def *enemy_rat_def;
geActor_Def *enemy_zombie_def;
geActor_Def *enemy_civilian_def[NUMBER_OF_CIVILIANS];

geActor_Def* LoadActorDef(const char* fileName);
geActor* LoadActor(geActor_Def* def, geWorld* World, float scale, uint32 flags, geXForm3d* xf);
void KillActor(geWorld* World, geActor** act);
void setup_box(geExtBox* eb, geActor* act, geVec3d pos);
void setup_box_ex(geActor* act);

#endif