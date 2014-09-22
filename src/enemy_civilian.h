/*
enemy_civilian.h
contains the control of the civilians
enemy.h calls theese functions, nobody else should call them.
*/

#include "genesis.h"
#include "save_file.h"

#ifndef ENEMY_CIVILIAN_H
#define ENEMY_CIVILIAN_H

void apply_civilian(geWorld* world);

int handle_civilian_enemy(SaveFile* file, geWorld* world);

void enemy_civilian_init();

void enemy_civilian_newWorld(geWorld* World);//Done when entering a anew level
void enemy_civilian_clearWorld(geWorld* World);//Done when leaveing a level

void enemy_civilian_iterate(geWorld* World, float time);//Done evry frame

geBoolean enemy_civilian_isAlive(geActor* enemy);

geVec3d* enemy_civilian_getPosition(geActor* act);

geBoolean enemy_civilian_damage(geActor* enemy, // the enemy that got hit
				  int damage, // how many point damage does this weaopon damage do?
				  char type, // damage type, se above
				  char lbd,
				  /* Location Based Damage Data */
				  geVec3d fromPos, //fromPos - location of the shooter
				  geVec3d toPos // toPos - location of the weapon max range
				  );
geBoolean enemy_civilian_loopDamage(int damage, // how many point damage does this weaopon damage do?
				  char type, // damage type, se above
				  char lbd,
				  /* Location Based Damage Data */
				  geVec3d fromPos, //fromPos - location of the shooter
				  geVec3d toPos // toPos - location of the weapon max range
				  );
void enemy_civilian_explosionDamage(geVec3d* location, float range, int damage);
void enemy_civilian_fireExplosionDamage(geVec3d* location, float range, int damage);

char talkto_civilian(geActor* act);

void civilian_renderName();

#endif