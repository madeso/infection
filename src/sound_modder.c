#include "sound_modder.h"
#include "sound_system.h"

// soundsys_master

float sm_value;

void sm_init(){
	sm_value = 1.0f;
}
void sm_set(float data){
	if( data < 0.01f ) data = 0.01f;
	if( data > 1.0f ) data = 1.0f;
	sm_value = data;
}
void sm_iterate(float time){
	sm_value += time*time*1.5f;
	if( sm_value >= 1.0f ) sm_value = 1.0f;
	soundsys_master(sm_value);
}
