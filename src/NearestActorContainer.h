#include "genesis.h"

#ifndef __NAC_H
#define __NAC_H

#define COUNT_OF_NACNA		4

typedef struct _NacNa {
	geActor* actor;
	float sqDist;
	char used;
} NacNa;

NacNa gNacNa[COUNT_OF_NACNA];

void nac_addActor(geActor* act, geVec3d* pos, geVec3d* origin);
void nac_clear();
NacNa* nac_findAndRemoveClosest();


#endif