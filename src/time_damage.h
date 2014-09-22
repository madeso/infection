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
#define TIMEDAMAGE_ZOMBIEBITE 0
#define TIMEDAMAGE_LAVADAMAGE 1
#define TIMEDAMAGE_INFECTEDBITE 2
#define TIMEDAMAGE_FIRE 3
#define TIMEDAMAGE_DRUG 4

// Number of timedamages
#define TIMEDAMAGE_SIZE 5

void timedamage_init();
void timedamage_clear();
void timedamage_add( unsigned char type );
void timedamage_iterate();
char timedamage_has( unsigned char type );

int handle_timeDamage(SaveFile* file);

#endif