#include "genesis.h"

#ifndef __MINIROCKET_MANAGER_H
#define __MINIROCKET_MANAGER_H

#define NUMBER_OF_MINIROCKETS					15
#define MINIROCKET_DAMAGE_RADIUS				62500.0f // 250

typedef struct _InfMiniRocket {
	geBoolean active;
	geActor* mini_rocket;
	geVec3d position;
	geVec3d velocity;
	geXForm3d xform; // miniRocket xform
	geExtBox box;
} InfMiniRocket;

geActor_Def* mini_rocketActor;
InfMiniRocket miniRockets[NUMBER_OF_MINIROCKETS];

char weapon_mini_rocket_init();
void weapon_mini_rocket_proccess(float timePassed);
void weapon_mini_rocket_newWorld(geWorld* world);
void weapon_mini_rocket_kill();
void weapon_mini_rocket_throw(geVec3d velocity, geVec3d position);
void weapon_mini_rocket_destroy(InfMiniRocket* miniRocket);

// users of this little library need to implement this:
void weapon_mini_rocket_bang(geVec3d position, float radius);

#endif