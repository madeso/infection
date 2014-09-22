#include "recoil.h"

float recoil_force = 0.0f;
float recoil_direction_x = 0.0f;
float recoil_direction_y = 0.0f;

void recoil_clear() {
	recoil_direction_x = 
		recoil_direction_y = 
		recoil_force = 0.0f;
}

void recoil_move(geVec3d* angles, float time) {
	if( recoil_force > 0.0f ) {
		recoil_force -= time;
		angles->X += recoil_direction_x * recoil_force * time;
		angles->Y += recoil_direction_y * recoil_force * time;
	}
}

extern geFloat frand(geFloat, geFloat);

void recoil_give(float force) {
	/*if( force > 0.0f )
		recoil_force += force;
	else*/
		recoil_force = force;
	recoil_direction_x = frand(-35.0f,90.0f);
	recoil_direction_y = frand(-20.0f,20.0f);
}