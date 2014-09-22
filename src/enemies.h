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
#define DAMAGE_ARMORPIERCING	1
#define DAMAGE_IMPALING			2
#define DAMAGE_CRUSHING			3
#define DAMAGE_NORMAL			4
#define DAMAGE_ELECTRICAL		5
#define DAMAGE_TRANQUALICER		6
#define DAMAGE_FIRE				7
#define DAMAGE_ZOMBIE			8

geBoolean enemy_isAlive(geActor* enemy);

// Warning: if enemy is null the player is damaged
geBoolean enemy_damage(geActor* enemy, // the enemy that got hit
				  int damage, // how many point damage does this weaopon damage do?
				  char type, // damage type, se above
				  char lbd,
				  /* Location Based Damage Data */
				  geVec3d fromPos, //fromPos - location of the shooter
				  geVec3d toPos // toPos - location of the weapon max range
				  );

geBoolean enemy_loopDamage(int damage, // how many point damage does this weaopon damage do?
				  char type, // damage type, se above
				  char lbd,
				  /* Location Based Damage Data */
				  geVec3d fromPos, //fromPos - location of the shooter
				  geVec3d toPos // toPos - location of the weapon max range
				  );


// AI functions
char Basic_MoveForward(geVec3d pos, geVec3d rot, float speed, geWorld* world, geExtBox *eb, geVec3d* newPos);
void Basic_ApplyGravity(geVec3d pos, float gravity, geWorld* world, geExtBox *eb, geVec3d *newPos);
float enemy_canAttack(geVec3d a, geVec3d b, geWorld* World);

void enemy_explosionDamage(geVec3d* location, float range, int damage);
void enemy_fireExplosionDamage(geVec3d* location, float range, int damage);

// A monster struct is the closest MONSTER to an existing MONSTER. Every MONSTER has a monster struct
typedef struct _Monster {
	geBoolean used; // is this used
	geVec3d position; // reference to the position
	geActor* actor; // if zero/null it is refering to the null player - the hero - you
	float distance; // the distance from the owner
} Monster;

#define MONSTER_FLAGS_PLAYER			1
#define MONSTER_FLAGS_ZOMBIE			2
#define MONSTER_FLAGS_WEREWOLF			4
#define MONSTER_FLAGS_WAMPIRE			8
#define MONSTER_FLAGS_COMPANY			16
#define MONSTER_FLAGS_FRIENDLY			32
#define MONSTER_FLAGS_COMPUTER			64

#define MONSTER_FLAGS_OF_ZOMBIE		(MONSTER_FLAGS_PLAYER | MONSTER_FLAGS_WEREWOLF | MONSTER_FLAGS_WAMPIRE | MONSTER_FLAGS_COMPANY | MONSTER_FLAGS_FRIENDLY | MONSTER_FLAGS_COMPUTER )
#define MONSTER_FLAGS_OF_FRIENDLY	(MONSTER_FLAGS_WEREWOLF | MONSTER_FLAGS_WAMPIRE | MONSTER_FLAGS_COMPANY | MONSTER_FLAGS_ZOMBIE | MONSTER_FLAGS_COMPUTER )

void getClosestPosition(geXForm3d* enemyDirectionAndPosition, int flags, Monster* monster);


#define EN_NOT_TURNED	0
#define EN_TURNED		1

float getAngleBetwen(const geVec3d* a, const geVec3d* b);
void getDirectionVector(const geVec3d* from, const geVec3d* point, geVec3d* Result);
int enemy_lookAtPos(const geVec3d *pos, const geXForm3d* eXForm, geVec3d* eRotation, float time);
int enemy_fleeFromPos(const geVec3d *pos, const geXForm3d* eXForm, geVec3d* eRotation, float time);
float enemy_distanceBetween(geVec3d a, geVec3d b, geWorld* World);

geVec3d* enemy_getPosition(geActor* act);

#endif