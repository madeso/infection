#include "NearestActorContainer.h"

void nac_addActor(geActor* act, geVec3d* pos, geVec3d* origin){
	int i = 0;
	float sqDist = 0.0f;
	geVec3d res;

	geVec3d_Subtract(pos, origin, &res);
	sqDist = geVec3d_LengthSquared(&res);

	for( i=0; i< COUNT_OF_NACNA; i++){
		if( !gNacNa[i].used || gNacNa[i].sqDist > sqDist ){
			gNacNa[i].used = 1;
			gNacNa[i].sqDist = sqDist;
			gNacNa[i].actor = act;
			break;
		}
	}
}

void nac_clear(){
	int i = 0;
	for( i=0; i< COUNT_OF_NACNA; i++){
		gNacNa[i].used = 0;
		gNacNa[i].actor = 0;
		gNacNa[i].sqDist = 0.0f;
	}
}

NacNa* nac_findAndRemoveClosest(){
	int min = -1;
	int i = 0;
	float minDist;

	for( i=0; i< COUNT_OF_NACNA; i++){
		if( gNacNa[i].used && ( min == -1 || minDist > gNacNa[i].sqDist ) ){
			minDist = gNacNa[i].sqDist;
			min = i;
		}
	}

	if( min == -1 ) return 0;
	gNacNa[min].used = 0;
	return &(gNacNa[min]);
}