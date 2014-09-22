/*
enemies.h
enemy manager, calls the different functions
*/

#include "genesis.h"
#include "save_file.h"
#include "sound_system.h"

#ifndef ENEMIES_H
#define ENEMIES_H

soundsys_sound e_splat;

// this file and it's implementation takes care of controlling the enemies in evry situation
int handle_enemy(SaveFile* file, geWorld* world);

// done at the beginning of the game
geBoolean enemy_init();
//Done at the end of the game
void enemy_erase();

void enemy_newWorld(geWorld* World);//Done when entering a anew level
void enemy_clearWorld(geWorld* World);//Done when leaveing a level

void enemy_iterate(geWorld* World, float time);//Done evry frame

/* Damage types */
#define DAMAGE_ARMORPIERCING	1 // is this needed???
#define DAMAGE_IMPALING			2
#define DAMAGE_CRUSHING			3
#define DAMAGE_NORMAL			4

geBoolean enemy_isAlive(geActor* enemy);

geBoolean enemy_damage(geActor* enemy, // the enemy that got hit
				  unsigned char damage, // how many point damage does this weaopon damage do?
				  char type, // damage type, se above
				  /* Location Based Damage Data */
				  geVec3d fromPos, //fromPos - location of the shooter
				  geVec3d toPos // toPos - location of the weapon max range
				  );

// AI functions
void Basic_MoveForward(geVec3d pos, geVec3d rot, float speed, geWorld* world, geExtBox *eb, geVec3d* newPos);
void Basic_ApplyGravity(geVec3d pos, float gravity, geWorld* world, geExtBox *eb, geVec3d *newPos);
float enemy_canAttack(geVec3d a, geVec3d b, geWorld* World);
#endif