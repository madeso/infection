#include "genesis.h"

#ifndef __TRANQ_MANAGER_H
#define __TRANQ_MANAGER_H

#define NUMBER_OF_TRANQS					5

typedef struct _InfTranq {
	geBoolean active;
	float timeLeftToBang;
	geActor* tranq;
	geVec3d position;
	geVec3d velocity;
	geXForm3d xform; // tranq xform
	geExtBox box;
} InfTranq;

geActor_Def* tranqActor;
InfTranq tranqs[NUMBER_OF_TRANQS];

char weapon_tranq_init();
void weapon_tranq_proccess(float timePassed);
void weapon_tranq_newWorld(geWorld* world);
void weapon_tranq_kill();
void weapon_tranq_throw(geVec3d velocity, geVec3d position, float time);
void weapon_tranq_destroy(InfTranq* tranq);

// users of this little library need to implement this:
void weapon_tranq_hit(geVec3d* position, geVec3d* to);

#endif