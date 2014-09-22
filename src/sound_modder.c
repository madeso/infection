#include "sound_modder.h"
#include "sound_system.h"

// soundsys_master

// private, sort of
float sm_value = 1.0f;
char sm_enable = 0;
float sm_add = 0.0f;
soundsys_sound sm_bangsound;

void sm_init(){
	sm_value = 1.0f;
	sm_enable = 1;
	sm_add = 0.0f;
	soundsys_loadSound(".\\sfx\\player\\bangbeep.wav", 1, &sm_bangsound, 0, TYPE_UNAFFECTED);
}
void sm_set(float data){
	if( data < 0.01f ) data = 0.01f;
	if( data > 1.0f ) data = 1.0f;
	sm_value = data;
	sm_enable = 1;
	sm_add = 0.0f;
	soundsys_play_sound(&sm_bangsound, GE_TRUE, TYPE_STOP, 0.0f, 0.1f);
}

float getBeepValue(float x){
	float pow = 2 * ( x - 0.5f );
	return - pow*pow+1;
}

void sm_iterate(float time){
	float val;
	if(! sm_enable ) return;
	sm_add += time * 0.07f; // previous: 0.03f
	sm_value += time*sm_add;
	val = getBeepValue(sm_value);
	
	if( sm_value >= 1.0f ){
		sm_value = 1.0f;
		sm_enable = 0;
		soundsys_stop(&sm_bangsound);
	}
	else {
		soundsys_setVolume(&sm_bangsound, val );
	}
	soundsys_setGameVolume(1.0f - val);
}
