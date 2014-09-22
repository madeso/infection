#include "genesis.h"

#ifndef __GRENADE_MANAGER_H
#define __GRENADE_MANAGER_H

#define NUMBER_OF_GRENADES					5
#define GRENADE_DAMAGE_RADIUS				160000.0f

typedef struct _InfGrenade {
	geBoolean active;
	float timeLeftToBang;
	geActor* grenade;
	geVec3d position;
	geVec3d velocity;
	geXForm3d xform; // grenade xform
	geExtBox box;
} InfGrenade;

geActor_Def* grenadeActor;
InfGrenade grenades[NUMBER_OF_GRENADES];

char weapon_grenade_init();
void weapon_grenade_proccess(float timePassed);
void weapon_grenade_newWorld(geWorld* world);
void weapon_grenade_kill();
void weapon_grenade_throw(geVec3d velocity, geVec3d position, float time);
void weapon_grenade_destroy(InfGrenade* grenade);

// users of this little library need to implement this:
void weapon_grenade_bang(geVec3d position, float radius);

#endif