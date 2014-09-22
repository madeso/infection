#include "time_damage.h" // we will define theese functions declared in this file

#include "memory.h"
#include "genesis.h"
#include "player.h" // for the moved variable
#include "console.h" // debug and error printing
#include <stdlib.h>
#include "timefx.h"
#include "globalGenesis.h"
#include "damage.h"
#include "log.h"
#include "sound_system.h"
#include "enemies.h"
#include "fxbitmaps.h"
#include "inf_message_system.h"

int td_handleTimeDamage(TimeDamage* td, SaveFile* file){
	FLOAT( td->timeLeft, "Failed to handle timedamage timeLeft" );
	INT( td->damageLeft , "Failed to handle damage left");
	INT( td->type, "Failed to handle tmedamage type");

	return 1;
}
void td_timedamageInit(TimeDamage* td){
	td_timedamageClear(td);
}
void td_timedamageClear(TimeDamage* td){
	td->type = TIMEDAMAGE_NONE;
	td->damageLeft = 0;
	td->timeLeft = 0.0f;
}

float getTimeOfTimedamage(int type){
	switch(type){
	case TIMEDAMAGE_NONE:
		break;
	case TIMEDAMAGE_INFECTED:
		return 4.0f;
	case TIMEDAMAGE_DRUG:
		return 8.0f;
	case TIMEDAMAGE_FIRE:
		return 0.2f;
	}
	return 0.0f;
}

void disableTimedamageType(int type, geActor* act){
	if( act ) return;
	soundsys_updateSound(1.0f);
	timefx_normal();
}

void td_timedamageAdd(TimeDamage* td, int type, geActor* act, int time){
	if( type == TIMEDAMAGE_NONE ) return;
	if( td->type == type ){
		td->damageLeft += time;
	}
	else {
		if( td->type < type ){
			disableTimedamageType(td->type,act);
			td->type = type;
			td->timeLeft = getTimeOfTimedamage(type);
			td->damageLeft = time;
		}
	}
}
char td_timedamageHas(TimeDamage* td, int type){
	return td->type == type;
}
void td_timedamageIterate(TimeDamage* td, geActor* act) {
	float time = TIME;
	switch(td->type){
	case TIMEDAMAGE_NONE:
		if(! act ) {
			timefx_normal();
		}
		return;
	case TIMEDAMAGE_INFECTED:
		if( !act ){
			soundsys_updateSound(2.0);
			timefx_bite(3.0f);
		}

		td->timeLeft-=time;
		if( td->timeLeft <= 0.0f ){
			if( td->damageLeft <= 0 ){
				disableTimedamageType( td->type, act );
				td_timedamageClear(td);
			}
			else {
				geVec3d zeroVector;
				geVec3d_Clear(&zeroVector);
				td->damageLeft -= 1;
				// cause damage
				enemy_damage(act, 1 + rand() % 5, DAMAGE_ARMORPIERCING, 0, zeroVector, zeroVector);
				td->timeLeft += getTimeOfTimedamage(td->type);
			}
		}
		break;
	case TIMEDAMAGE_DRUG:
		td->timeLeft-=time;
		if( !act ){
			soundsys_updateSound(0.33f);
			timefx_drug(3.0f);
		}
		if( td->timeLeft <= 0.0f ){
			if( td->damageLeft <= 0 ){
				disableTimedamageType( td->type, act );
				td_timedamageClear(td);
			}
			else {
				geVec3d zeroVector;
				geVec3d_Clear(&zeroVector);
				td->damageLeft -= 1;
				// cause damage
				enemy_damage(act, 1 + rand() % 5, DAMAGE_ARMORPIERCING, 0, zeroVector, zeroVector);
				td->timeLeft += getTimeOfTimedamage(td->type);
			}
		}
		break;
	case TIMEDAMAGE_FIRE:
		if( act == 0 && moved ) {
			td->timeLeft+=time;
			//system_message("adding fire time");
		}
		else
			td->timeLeft-=time;
		if( td->timeLeft <= 0.0f ){
			if( td->damageLeft <= 0 ){
				disableTimedamageType( td->type, act );
				td_timedamageClear(td);
			}
			else {
				geVec3d zeroVector;
				geVec3d_Clear(&zeroVector);
				td->damageLeft -= 1;
				// cause damage
				enemy_damage(act, 1 + rand() % 6, DAMAGE_FIRE, 0, zeroVector, zeroVector);
				td->timeLeft += getTimeOfTimedamage(td->type);
				fx_fire(&Pos , GE_FALSE, 0.50f);
			}
		}
		else if(td->timeLeft > 0.4f) {
			// effect
			geVec3d zeroVector;
			geVec3d_Clear(&zeroVector);
			td->timeLeft -= getTimeOfTimedamage(td->type);
			td->damageLeft -= 2;
			enemy_damage(act, 1 + rand() % 2, DAMAGE_FIRE, 0, zeroVector, zeroVector);
			if( !act ) {
				fx_fire(&Pos , GE_FALSE, 0.50f);
			}
			if( td->damageLeft <= 0 ){
				td_timedamageClear(td);
			}
		}
		break;
	}
}


///////////////////////////////////////////////////////
// Player specific - call the general
TimeDamage playerTd;

int handle_timeDamage(SaveFile* file){
	return td_handleTimeDamage(&playerTd, file);
}
void timedamage_init(){
	td_timedamageInit(&playerTd);
}
void timedamage_clear(){
	td_timedamageClear(&playerTd);
}
void timedamage_add( int type, int value ){
	td_timedamageAdd(&playerTd, type, 0, value);
}
char timedamage_has( int type ){
	return td_timedamageHas(&playerTd, type);
}
void timedamage_iterate() {
	td_timedamageIterate(&playerTd, 0);
}