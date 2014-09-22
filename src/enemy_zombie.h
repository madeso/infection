/*
enemy_zombie.h
contains the control of the zombies
enemy.h calls theese functions, nobody else should call them.
*/

#include "genesis.h"
#include "save_file.h"

#ifndef ENEMY_ZOMBIE_H
#define ENEMY_ZOMBIE_H

void apply_zombies(geWorld* world);

int handle_zombie_enemy(SaveFile* file, geWorld* world);

void enemy_zombie_init();

void enemy_zombie_newWorld(geWorld* World);//Done when entering a anew level
void enemy_zombie_clearWorld(geWorld* World);//Done when leaveing a level

void enemy_zombie_iterate(geWorld* World, float time);//Done evry frame

/* Damage types
#define DAMAGE_ARMORPIERCING	1
#define DAMAGE_IMPALING			2
#define DAMAGE_CRUSHING			3
#define DAMAGE_NORMAL			4
*/

geBoolean enemy_zombie_isAlive(geActor* enemy);

geVec3d* enemy_zombie_getPosition(geActor* act);

geBoolean enemy_zombie_damage(geActor* enemy, // the enemy that got hit
				  int damage, // how many point damage does this weaopon damage do?
				  char type, // damage type, se above
				  char lbd,
				  /* Location Based Damage Data */
				  geVec3d fromPos, //fromPos - location of the shooter
				  geVec3d toPos // toPos - location of the weapon max range
				  );
geBoolean enemy_zombie_loopDamage(int damage, // how many point damage does this weaopon damage do?
				  char type, // damage type, se above
				  char lbd,
				  /* Location Based Damage Data */
				  geVec3d fromPos, //fromPos - location of the shooter
				  geVec3d toPos // toPos - location of the weapon max range
				  );
void enemy_zombie_explosionDamage(geVec3d* location, float range, int damage);
void enemy_zombie_fireExplosionDamage(geVec3d* location, float range, int damage);

#endif