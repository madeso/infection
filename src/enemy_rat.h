/*
enemy_rat.h
contains the control of the rats
enemy.h calls theese functions, nobody else should call them.
*/

#include "genesis.h"
#include "save_file.h"

#ifndef ENEMY_RAT_H
#define ENEMY_RAT_H

void apply_rats(geWorld* world);

int handle_rat_enemy(SaveFile* file, geWorld* world);

void enemy_rat_init();

void enemy_rat_newWorld(geWorld* World);//Done when entering a anew level
void enemy_rat_clearWorld(geWorld* World);//Done when leaveing a level

void enemy_rat_iterate(geWorld* World, float time);//Done evry frame

/* Damage types
#define DAMAGE_ARMORPIERCING	1
#define DAMAGE_IMPALING			2
#define DAMAGE_CRUSHING			3
#define DAMAGE_NORMAL			4
*/

geBoolean enemy_rat_isAlive(geActor* enemy);

geBoolean enemy_rat_damage(geActor* enemy, // the enemy that got hit
				  unsigned char damage, // how many point damage does this weaopon damage do?
				  char type, // damage type, se above
				  /* Location Based Damage Data */
				  geVec3d fromPos, //fromPos - location of the shooter
				  geVec3d toPos // toPos - location of the weapon max range
				  );
geBoolean enemy_rat_canDamage(geActor* enemy, // the enemy that got hit
				  geVec3d fromPos, //fromPos - location of the shooter
				  geVec3d toPos // toPos - location of the weapon max range
				  );
geVec3d* enemy_rat_getPosition(geActor* act);
void enemy_rat_explosionDamage(geVec3d* location, float range, int damage);

#endif