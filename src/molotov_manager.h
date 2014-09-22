#include "genesis.h"

#ifndef __MOLOTOV_MANAGER_H
#define __MOLOTOV_MANAGER_H

#define NUMBER_OF_MOLOTOVS					5
#define MOLOTOV_DAMAGE_RADIUS				160000.0f // 400

typedef struct _InfMolotov {
	geBoolean active;
	geActor* molotov;
	geVec3d position;
	geVec3d velocity;
	geXForm3d xform; // molotov xform
	geExtBox box;
} InfMolotov;

geActor_Def* molotovActor;
InfMolotov molotovs[NUMBER_OF_MOLOTOVS];

char weapon_molotov_init();
void weapon_molotov_proccess(float timePassed);
void weapon_molotov_newWorld(geWorld* world);
void weapon_molotov_kill();
void weapon_molotov_throw(geVec3d velocity, geVec3d position);
void weapon_molotov_destroy(InfMolotov* molotov);

// users of this little library need to implement this:
void weapon_molotov_bang(geVec3d position, float radius);

#endif