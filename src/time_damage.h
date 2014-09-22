/*
time_damage.h
takes care of damaging the player over time,
plus changing the fov and the timefx
*/

#include "genesis.h"
#include "save_file.h"

#ifndef TIME_DAMAGE_H
#define TIME_DAMAGE_H

// possible timedamages
#define TIMEDAMAGE_NONE				0
#define TIMEDAMAGE_INFECTED			1
#define TIMEDAMAGE_DRUG				2
#define TIMEDAMAGE_FIRE				3

typedef struct timeDamage_tag
{
	int type;
	float timeLeft;
	int damageLeft;
} TimeDamage;

void timedamage_init();
void timedamage_clear();
void timedamage_add( int type, int value );
void timedamage_iterate();
char timedamage_has( int type );
int handle_timeDamage(SaveFile* file);



//////////////////////////////////////////////
// General functions
int td_handleTimeDamage(TimeDamage* td, SaveFile* file);
void td_timedamageInit(TimeDamage* td);
void td_timedamageClear(TimeDamage* td);
void td_timedamageAdd(TimeDamage* td, int type, geActor* act, int value);
char td_timedamageHas(TimeDamage* td, int type);
void td_timedamageIterate(TimeDamage* td, geActor* act);

#endif