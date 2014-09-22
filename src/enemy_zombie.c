#include "enemy_zombie.h"
#include "enemy_def.h"
#include "inf_actor.h"
#include "memory.h"
#include "malloc.h"
#include "stdlib.h"
#include "time_damage.h"
#include "extra_genesis.h"
#include "player.h"
#include "damage.h"
#include "log.h"
#include "jacktalks.h"
#include "globalGenesis.h"
#include "useful_macros.h"
#include "enemies.h"
#include <math.h>
#include "sound_system.h"
#include "explosions.h"
#include "useful_macros.h"
#include "inf_message_system.h"
#include "infection.h" // hande_Vector function
#include "fxbitmaps.h" // BloodExplosion function
#include "console.h"

#define UPDATE_CLOSEST			getClosestPosition( z->XForm, MONSTER_FLAGS_OF_ZOMBIE, z->closest )
#define CAN_SEE_ENEMY			z->closest->used
#define FIFTY_PERCENTS_CHANCE	rand()%2 == 0
#define IN_RANGE_OF_ENEMY		(z->closest->distance < ZOMBE_SLASH_RANGE_SQ && z->closest->used)

soundsys_sound zombie_damage[6];
soundsys_sound zombie_attack[3];
soundsys_sound zombie_idle[4];

geMotion			*ZombieIdle=0;
geMotion			*ZombieWalk=0;
geMotion			*ZombieDie=0;
geMotion			*ZombieSlash1=0;
geMotion			*ZombieSlash2=0;
geMotion			*ZombieDamage=0;
geMotion			*ZombieEating=0;

#define ZOMBE_SLASH_RANGE		120.0f
#define ZOMBE_SLASH_RANGE_SQ	14400.0f
#define ZOMBIE_SPEED			100.0f

#define ZOMBIE_STATE_IDLE							0	// standing still
#define ZOMBIE_STATE_DIE							1	// dying and waiting for resurection
#define ZOMBIE_STATE_RESURECTION					2	// resurection - rising from dead state
#define ZOMBIE_STATE_DEAD							3	// dead - won't rise again
#define ZOMBIE_STATE_DAMAGE							4	// the zombie is in pain
#define ZOMBIE_STATE_WALK_TOWARDS_ENEMY				5	// 
#define ZOMBIE_STATE_ELECTRICAL						6	// 
#define ZOMBIE_STATE_EAT							7	// 
#define ZOMBIE_STATE_SLASH1							8	// 
#define ZOMBIE_STATE_SLASH2							9	// 
#define ZOMBIE_STATE_WALK_TOWARDS_ENEMY_LAST_POS	10	// 
#define ZOMBIE_STATE_ATTACKMODE						11	// 
#define ZOMBIE_STATE_WAITING_FOR_RESURECTION		12	// 

void enemy_zombie_statePrint(Inf_Enemy_Zombie* zombie){
	char str[300];
	sprintf(str, "Zombie state is: %i / %f", zombie->state, zombie->counter);
	game_message(str);
}

void enemy_zombie_setHealth(Inf_Enemy_Zombie* zombie){
	switch(zombie->Type){
	case 1: // weak
		zombie->Health = 50;
		break;
	case 2: // standard
		zombie->Health = 100;
		break;
	case 3: // tough
		zombie->Health = 200;
		break;
	default:
		zombie->Type = (rand()%3) + 1;
		enemy_zombie_setHealth(zombie);
		break;
	}
}
zombie_setHealth(z);

void zombie_sayDamage(Inf_Enemy_Zombie* zombie){
	if( rand()%5 == 0 ) return;
	soundsys_play3dsound( &(zombie_damage[rand()%6]) , &(zombie->Position), 5.0f, GE_FALSE, GE_FALSE);
}
void zombie_sayAttack(Inf_Enemy_Zombie* zombie, char doit){
	if( !doit ) {
		if( rand()%4 == 0 ) return;
	}
	soundsys_play3dsound( &(zombie_attack[rand()%3]) , &(zombie->Position), 5.0f, GE_FALSE, GE_FALSE);
}
void zombie_sayIdle(Inf_Enemy_Zombie* zombie){
	if( rand()%4 == 0 ) return;
	soundsys_play3dsound( &(zombie_idle[rand()%4]) , &(zombie->Position), 5.0f, GE_TRUE, GE_FALSE);
}

void enemy_zombie_doDamage(Inf_Enemy_Zombie* z, char type, int damage, geVec3d* damageLocation){
	//enemy_zombie_statePrint(z);
	if( !damage ) return;
	if( z->state == ZOMBIE_STATE_RESURECTION ) return;
	if( !damageLocation ) damageLocation = &(z->Position);
	BloodExplosion( *damageLocation );
	if( z->Health <= 0 ) return;
	zombie_sayDamage(z);
	talkPower_Change( damage/10.0f );
	if( type == DAMAGE_NORMAL ){
		z->Health -= damage;
	} else if( type == DAMAGE_ELECTRICAL ) {
		z->state = ZOMBIE_STATE_ELECTRICAL;
	}
	else if( type == DAMAGE_FIRE ) {
		// temporary
		//z->Health -= damage;
		z->burningTime = 0.0f;
		z->burningDamage += damage;
	}
	/*if( z->state != ZOMBIE_STATE_ELECTRICAL ){
		z->state = ZOMBIE_STATE_DAMAGE;
		z->counter = 0.0f;
	}*/
	if( z->Health <= 0 ){
		z->Health = 0;
		z->counter = 0.0f;
		if( z->Resurections > 0 ){
			z->state = ZOMBIE_STATE_DIE;
			z->Resurections -= 1;
			//game_message("Waiting for resurection");
		} else {
			z->state = ZOMBIE_STATE_DEAD;
			//game_message("Zombie died");
		}
	}
}


// may change state to eating
void enemy_zombie_attack(Inf_Enemy_Zombie* z){
	geVec3d to;
	geVec3d from;
	geXForm3d_GetIn(z->XForm, &to);
	from = z->Position;
	geVec3d_AddScaled(&from,&to, ZOMBE_SLASH_RANGE, &to);
	// remember: if the actor is null, we are kicking the player's ass
	enemy_damage(z->closest->actor, 10 + rand() % 10, DAMAGE_ZOMBIE, 0, from, to);

	if(! enemy_isAlive(z->closest->actor) ){
		UPDATE_CLOSEST;
		if(! z->closest->used ){
			z->state = ZOMBIE_STATE_EAT;
			z->counter = 0.0f;
		}
	}
}

int handle_zombie_enemy(SaveFile* file, geWorld* world){
	geEntity_EntitySet	*Set=0;
	geEntity			*Entity=0;
	
	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Zombie");
	if (Set == NULL) return 1;
	
	// get first rain entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_Enemy_Zombie* r;
	
		r = (Inf_Enemy_Zombie *)geEntity_GetUserData(Entity);
		
		INT( r->state, "Failed to handle zombie state");
		INT( r->Health, "Failed to handle zombie health");
		INT( r->Type, "Failed to handle zombie type");
		INT( r->Resurections, "Failed to handle zombie resurection");
		FLOAT( r->counter, "Failed to handle zombie counter");
		VECTOR( r->Position, "Failed to handle zombie position");
		VECTOR( r->Orientation, "Failed to handle zombie orientation");
		INT( r->closest->used , "Failed to handle zombie monster used");
		VECTORPTR( &(r->closest->position) , "Failed to handle zombie monster position" );
		FLOAT( r->closest->distance, "Failed to handle zombie monster distance");

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}

	return 1;
}

void zombie_applyAndEvaluateZombieState(Inf_Enemy_Zombie* z, float time){
	geXForm3d_SetIdentity(z->XForm );
	geXForm3d_RotateX(z->XForm, z->Orientation.X );
	geXForm3d_RotateY(z->XForm, z->Orientation.Y + GE_PI);
	geXForm3d_RotateZ(z->XForm, z->Orientation.Z );
	z->XForm->Translation = z->Position;

	Basic_ApplyGravity(z->Position, 180.0f * time, World, z->ExtBox, &( z->Position ));

	//////////////////////////////////////////////////
	// state evaluate

	// damage evaluate
	if( z->burningDamage > 0 ) {
		z->burningTime -= TIME;
		if( z->burningTime <= 0.0f ) {
			geVec3d fire = z->Position;
			fire.Y += 30.0f;
			z->burningDamage -= 1;
			z->burningTime = 0.25f;
			enemy_zombie_doDamage(z, DAMAGE_NORMAL, 5, 0);
			fx_fire(&fire, GE_FALSE, 0.50f);
		}
	}

	switch( z->state ){
	case ZOMBIE_STATE_DEAD:
		geActor_SetPose(z->Actor, ZombieDie, z->counter, z->XForm );
		z->counter += time;
		if( z->counter > 1.0f ) z->counter = 1.0f;
		break;
	case ZOMBIE_STATE_RESURECTION:
		geActor_SetPose(z->Actor, ZombieDie, z->counter, z->XForm );
		z->counter -= time;
		if( z->counter <= 0.0f ){
			z->counter = 0.0f;
			z->state = ZOMBIE_STATE_IDLE;
			zombie_sayIdle(z);
			enemy_zombie_setHealth(z);
		}
		break;
	case ZOMBIE_STATE_DIE:
		z->counter += time;
		if( z->counter > 1.0f ) {
			geActor_SetPose(z->Actor, ZombieDie, 1.0f, z->XForm );
		}
		else {
			geActor_SetPose(z->Actor, ZombieDie, z->counter, z->XForm );
		}
		if( z->counter > 5.0f ){
			z->state = ZOMBIE_STATE_WAITING_FOR_RESURECTION;
			z->counter = RAND_FLOAT() * 10.0f;
		}
		break;
    case ZOMBIE_STATE_WAITING_FOR_RESURECTION:
		z->counter -= time;
		geActor_SetPose(z->Actor, ZombieDie, 1.0f, z->XForm );
		if( z->counter <= 0.0f ){
			z->state = ZOMBIE_STATE_RESURECTION;
			z->counter = 1.0f;
			zombie_sayAttack(z, 1);
			UPDATE_CLOSEST;
		}
		break;
	case ZOMBIE_STATE_IDLE:
		UPDATE_CLOSEST;
		geActor_SetPose(z->Actor, ZombieIdle, z->counter, z->XForm );
		z->counter += time;
		if( z->counter > 1.29f ) {
			z->counter = 0.0f;
			//zombie_sayIdle(z);
		};
		if( CAN_SEE_ENEMY ) {
			z->state = ZOMBIE_STATE_WALK_TOWARDS_ENEMY;
			z->counter = 0.0f;
			zombie_sayIdle(z);
		}
		break;
	case ZOMBIE_STATE_DAMAGE:
		geActor_SetPose(z->Actor, ZombieDamage, z->counter, z->XForm );
		z->counter += time;
		if( z->counter > 0.88f ) {
			z->counter = 0.0f;
			z->state = ZOMBIE_STATE_IDLE;
			zombie_sayIdle(z);
		}
		break;
	case ZOMBIE_STATE_WALK_TOWARDS_ENEMY:
		UPDATE_CLOSEST;
		geActor_SetPose(z->Actor, ZombieWalk, z->counter, z->XForm );
		z->counter += time;
		if( z->counter > 1.0f ) {
			z->counter = 0.0f;
		}

		enemy_lookAtPos(&(z->closest->position), z->XForm, &(z->Orientation), time);
		Basic_MoveForward(z->Position, z->Orientation, ZOMBIE_SPEED * time, World, z->ExtBox, &(z->Position) );

		if(! z->closest->used ){
			z->state = ZOMBIE_STATE_WALK_TOWARDS_ENEMY_LAST_POS;
			// ignore clearing counter since the next state will also use the walk anim and it's descision won't be based on time
		} else if( IN_RANGE_OF_ENEMY ) {
			z->state = ZOMBIE_STATE_ATTACKMODE;
			z->counter = 0.0f;
		}
		break;
	case ZOMBIE_STATE_ELECTRICAL: // this zombie can say goodby to this life, better luck next resurection :)
		geActor_SetPose(z->Actor, ZombieDamage, z->counter, z->XForm );
		z->counter += time;
		if( z->counter > 0.88f ) {
			z->counter = 0.0f;
			enemy_zombie_doDamage(z, DAMAGE_NORMAL, (rand()%10) + 10, 0);
		}
		break;
	case ZOMBIE_STATE_SLASH1:
		geActor_SetPose(z->Actor, ZombieSlash1, z->counter, z->XForm );
		z->counter += time;
		if( z->counter > 0.46f ) {
			z->counter = 0.0f;
			if( FIFTY_PERCENTS_CHANCE ){
				zombie_sayAttack(z, 0);
				z->state = ZOMBIE_STATE_SLASH2;
			}
			else {
				z->state = ZOMBIE_STATE_ATTACKMODE;
			}
			enemy_zombie_attack(z);
		}
		break;
	case ZOMBIE_STATE_SLASH2:
		geActor_SetPose(z->Actor, ZombieSlash2, z->counter, z->XForm );
		z->counter += time;
		if( z->counter > 0.46f ) {
			z->counter = 0.0f;
			z->state = ZOMBIE_STATE_ATTACKMODE;
			enemy_zombie_attack(z);
		}
		break;
	case ZOMBIE_STATE_WALK_TOWARDS_ENEMY_LAST_POS:
		UPDATE_CLOSEST;
		geActor_SetPose(z->Actor, ZombieWalk, z->counter, z->XForm );
		z->counter += time;
		if( z->counter > 1.0f ) {
			z->counter = 0.0f;
		}
		enemy_lookAtPos(&(z->closest->position), z->XForm, &(z->Orientation), time);
		// @@@ fix statechange when reached pos
		Basic_MoveForward(z->Position, z->Orientation, ZOMBIE_SPEED * time, World, z->ExtBox, &(z->Position) );
		if( z->closest->used ){
			z->state = ZOMBIE_STATE_WALK_TOWARDS_ENEMY;
			// ignore clearing counter since the next state will also use the walk anim and it's descision won't be based on time
			zombie_sayAttack(z, 0);
		}
		break;
	case ZOMBIE_STATE_EAT:
		geActor_SetPose(z->Actor, ZombieEating, z->counter, z->XForm );
		z->counter += time;
		if( z->counter > 1.0f ) {
			UPDATE_CLOSEST;
			z->counter = 0.0f;
			if( z->closest->used ){
				z->state = ZOMBIE_STATE_IDLE;
			}
		}
		break;
	case ZOMBIE_STATE_ATTACKMODE:
		{
			geActor* temp = z->closest->actor;
			UPDATE_CLOSEST;
			
			if(! z->closest->used ){
				if( enemy_isAlive(temp) ){
					z->state = ZOMBIE_STATE_EAT;
					z->counter = 0.0f;
				}
				else {
					z->state = ZOMBIE_STATE_WALK_TOWARDS_ENEMY_LAST_POS;
					z->counter = 0.0f;
				}
			}
			else {
				geActor_SetPose(z->Actor, ZombieIdle, z->counter, z->XForm );
				z->counter += time;
				if( z->counter > 0.7f ) {
					z->counter = 0.0f;
					zombie_sayAttack(z, 1);
					if( FIFTY_PERCENTS_CHANCE ){
						z->state = ZOMBIE_STATE_SLASH2;
					}
					else {
						z->state = ZOMBIE_STATE_SLASH1;
					}
				}
				if(! IN_RANGE_OF_ENEMY ) {
					z->state = ZOMBIE_STATE_WALK_TOWARDS_ENEMY;
					z->counter = 0.0f;
				}
			}
		}
		break;
	default:
		system_message("Bad FSM state in zombie switch");
		z->state = ZOMBIE_STATE_IDLE;
		z->counter = 0.0f;
	}
}

void enemy_zombie_init(){
	soundsys_loadSound(".\\sfx\\enemies\\zombie\\damage0.wav", 1, &(zombie_damage[0]), 1, TYPE_3D);
	soundsys_loadSound(".\\sfx\\enemies\\zombie\\damage1.wav", 1, &(zombie_damage[1]), 1, TYPE_3D);
	soundsys_loadSound(".\\sfx\\enemies\\zombie\\damage2.wav", 1, &(zombie_damage[2]), 1, TYPE_3D);
	soundsys_loadSound(".\\sfx\\enemies\\zombie\\damage3.wav", 1, &(zombie_damage[3]), 1, TYPE_3D);
	soundsys_loadSound(".\\sfx\\enemies\\zombie\\damage4.wav", 1, &(zombie_damage[4]), 1, TYPE_3D);
	soundsys_loadSound(".\\sfx\\enemies\\zombie\\damage5.wav", 1, &(zombie_damage[5]), 1, TYPE_3D);

	soundsys_loadSound(".\\sfx\\enemies\\zombie\\attack0.wav", 1, &(zombie_attack[0]), 1, TYPE_3D);
	soundsys_loadSound(".\\sfx\\enemies\\zombie\\attack1.wav", 1, &(zombie_attack[1]), 1, TYPE_3D);
	soundsys_loadSound(".\\sfx\\enemies\\zombie\\attack2.wav", 1, &(zombie_attack[2]), 1, TYPE_3D);

	soundsys_loadSound(".\\sfx\\enemies\\zombie\\idle0.wav", 1, &(zombie_idle[0]), 1, TYPE_3D);
	soundsys_loadSound(".\\sfx\\enemies\\zombie\\idle1.wav", 1, &(zombie_idle[1]), 1, TYPE_3D);
	soundsys_loadSound(".\\sfx\\enemies\\zombie\\idle2.wav", 1, &(zombie_idle[2]), 1, TYPE_3D);
	soundsys_loadSound(".\\sfx\\enemies\\zombie\\idle3.wav", 1, &(zombie_idle[3]), 1, TYPE_3D);

	ZombieIdle = geActor_GetMotionByName(enemy_zombie_def, "Idle" );
	ZombieWalk = geActor_GetMotionByName(enemy_zombie_def, "Walk" );
	ZombieDie = geActor_GetMotionByName(enemy_zombie_def, "Die1" );
	ZombieSlash1 = geActor_GetMotionByName(enemy_zombie_def, "Slash1" );
	ZombieSlash2 = geActor_GetMotionByName(enemy_zombie_def, "Slash2" );
	ZombieEating = geActor_GetMotionByName(enemy_zombie_def, "C_Idle" );
	ZombieDamage = geActor_GetMotionByName(enemy_zombie_def, "Injury" );
}

void apply_zombies(geWorld* world){
	geEntity_EntitySet	*Set=0;
	geEntity			*Entity=0;
	
	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Zombie");
	if (Set == NULL) return;
	
	// get first entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_Enemy_Zombie* r;
	
		r = (Inf_Enemy_Zombie *)geEntity_GetUserData(Entity);
		
		zombie_applyAndEvaluateZombieState(r, 0.0f);

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
}

void enemy_zombie_newWorld(geWorld* World){
	geEntity_EntitySet	*Set=0;
	geEntity			*Entity=0;
	
	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Zombie");
	if (Set == NULL) return;
	
	// get first rain entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_Enemy_Zombie* z;
	
		z = (Inf_Enemy_Zombie *)geEntity_GetUserData(Entity);

		z->Actor = 0;
		z->XForm = 0;
		z->ExtBox = 0;
		z->closest = 0;

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}

	// get first rain entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_Enemy_Zombie* z;
	
		z = (Inf_Enemy_Zombie *)geEntity_GetUserData(Entity);
		
		z->ExtBox = (geExtBox*) malloc( sizeof(geExtBox) );
		if( !z->ExtBox ) {
			run = 0;
			error("Failed to allocate extbox for zombie");
			continue;
		}

		z->XForm = (geXForm3d*) malloc( sizeof(geXForm3d) );
		if( !z->XForm ){
			run = 0;
			error("Failed to allocate xform for zombie");
			continue;
		}
		z->closest = (Monster*) malloc( sizeof(Monster) );
		if( !z->closest ){
			run = 0;
			error("Failed allocate monster for zombie");
			continue;
		}
		z->closest->used = GE_FALSE;

		geXForm3d_SetIdentity(z->XForm );

		IN_DEG_VECTOR(z->Orientation);

		geXForm3d_RotateX(z->XForm, z->Orientation.X );
		geXForm3d_RotateY(z->XForm, z->Orientation.Y );
		geXForm3d_RotateZ(z->XForm, z->Orientation.Z );
		z->XForm->Translation = z->Position;
		z->burningDamage = 0;
		z->burningTime = 0.0f;

		z->Actor = LoadActor(enemy_zombie_def, World, 2.0f, GE_ACTOR_RENDER_MIRRORS | GE_ACTOR_RENDER_NORMAL | GE_ACTOR_COLLIDE, z->XForm);
		if( !z->Actor ) {
			run = 0;
			error("Failed to create the zombie actor");
			continue;
		}
		// enable shaddows
		geActor_SetStencilShadow(z->Actor, GE_TRUE);

		enemy_zombie_setHealth(z);

		geActor_SetUserData(z->Actor, (void*) z);
		setup_box(z->ExtBox, z->Actor, z->Position);

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
}


void enemy_zombie_clearWorld(geWorld* World){
	geEntity_EntitySet	*Set=0;
	geEntity			*Entity=0;
	
	if( !World ) return;
	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Zombie");
	if (Set == NULL) return;
	
	// get first rain entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_Enemy_Zombie* z;
	
		z = (Inf_Enemy_Zombie *)geEntity_GetUserData(Entity);

		KillActor(World, &(z->Actor) );
		z->Actor = 0;

		if( z->XForm ) {
			free( z->XForm);
			z->XForm = 0;
		}
		if( z->ExtBox ){
			free(z->ExtBox);
			z->ExtBox = 0;
		}
		if( z->closest ){
			free(z->closest);
			z->closest = 0;
		}

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
}

void enemy_zombie_iterate(geWorld* World, float time){
	geEntity_EntitySet	*Set=0;
	geEntity			*Entity=0;
	
	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Zombie");
	if (Set == NULL) return;
	
	// get first rain entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_Enemy_Zombie* z;
	
		z = (Inf_Enemy_Zombie *)geEntity_GetUserData(Entity);
		
		zombie_applyAndEvaluateZombieState(z, time);

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
}


/* Damage types
#define DAMAGE_ARMORPIERCING	1
#define DAMAGE_IMPALING			2
#define DAMAGE_CRUSHING			3
#define DAMAGE_NORMAL			4
*/

geVec3d* enemy_zombie_getPosition(geActor* act){
	Inf_Enemy_Zombie* z;

	z = (Inf_Enemy_Zombie*) geActor_GetUserData(act);
	if( !z ){
		error("Convertion error in enemy_zombie_getPosition");
		return 0;
	}

	return &(z->Position);
}

geBoolean enemy_zombie_isAlive(geActor* enemy){
	Inf_Enemy_Zombie* z;

	z = (Inf_Enemy_Zombie*) geActor_GetUserData(enemy);
	if (z->Health > 0){
		//system_message("not moving through");
		return GE_TRUE;
	}
	//system_message("moving through");
	return GE_FALSE;
}

geBoolean enemy_zombie_canDamage(geActor* enemy, // the enemy that got hit
				  geVec3d fromPos, //fromPos - location of the shooter
				  geVec3d toPos // toPos - location of the weapon max range
				  ){
	Inf_Enemy_Zombie* z;
	geExtBox eb;
	float t;

	z = (Inf_Enemy_Zombie*) geActor_GetUserData(enemy);
	geActor_GetDynamicExtBox(z->Actor, &eb);

	if(! geExtBox_RayCollision( &eb, &fromPos, &toPos, 
								&t, NULL ) // don't save the data
								)
	{
		return GE_FALSE;
	}

	return GE_TRUE;
}

geBoolean enemy_zombie_damage(geActor* enemy, // the enemy that got hit
				  int damage, // how many point damage does this weaopon damage do?
				  char type, // damage type, se above
				  char lbd,
				  /* Location Based Damage Data */
				  geVec3d fromPos, //fromPos - location of the shooter
				  geVec3d toPos // toPos - location of the weapon max range
				  ){
	Inf_Enemy_Zombie* z;
	geExtBox eb;
	float t;
	geVec3d pos;

	z = (Inf_Enemy_Zombie*) geActor_GetUserData(enemy);

	if( lbd){
		geActor_GetDynamicExtBox(z->Actor, &eb);

		if(! geExtBox_RayCollision( &eb, &fromPos, &toPos, 
									&t, NULL ) // don't save the data
									)
		{
			return GE_TRUE;
		}


		geVec3d_Subtract(&toPos, &fromPos, &pos);
		//geVec3d_Scale(&pos, t, &pos);
		geVec3d_AddScaled(&fromPos, &pos, t, &pos);

		enemy_zombie_doDamage(z, type, damage, &pos);
	}
	else {
		enemy_zombie_doDamage(z, type, damage, 0);
	}

	return GE_FALSE;
}

void enemy_zombie_explosionDamage(geVec3d* location, float range, int damage){
	geEntity_EntitySet	*Set=0;
	geEntity			*Entity=0;
	
	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Zombie");
	if (Set == NULL) return;
	
	// get first rain entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_Enemy_Zombie* z;
	
		z = (Inf_Enemy_Zombie *)geEntity_GetUserData(Entity);
		
		enemy_zombie_doDamage(z, DAMAGE_NORMAL, explosion_getDamage(location, &(z->Position), range, damage), 0);

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
}

void enemy_zombie_fireExplosionDamage(geVec3d* location, float range, int damage){
	geEntity_EntitySet	*Set=0;
	geEntity			*Entity=0;
	
	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Zombie");
	if (Set == NULL) return;
	
	// get first rain entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_Enemy_Zombie* z;
	
		z = (Inf_Enemy_Zombie *)geEntity_GetUserData(Entity);
		
		enemy_zombie_doDamage(z, DAMAGE_FIRE, explosion_getDamage(location, &(z->Position), range, damage), 0);

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
}

geBoolean enemy_zombie_loopDamage(int damage, // how many point damage does this weaopon damage do?
				  char type, // damage type, se above
				  char lbd,
				  /* Location Based Damage Data */
				  geVec3d fromPos, //fromPos - location of the shooter
				  geVec3d toPos // toPos - location of the weapon max range
				  ){
	geExtBox eb;
	float t;
	geEntity_EntitySet	*Set=0;
	geEntity			*Entity=0;
	geBoolean value = GE_FALSE;
	
	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Zombie");
	if (Set == NULL) return GE_FALSE;
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);
	while( Entity )
	{
		Inf_Enemy_Zombie* z;
	
		z = (Inf_Enemy_Zombie *)geEntity_GetUserData(Entity);
		geActor_GetDynamicExtBox(z->Actor, &eb);
		if(geExtBox_RayCollision( &eb, &fromPos, &toPos, 
								&t, NULL ) // don't save the data
								) {
			GE_Collision lCol;
			geVec3d loc;
			geVec3d dir;
			geVec3d_Subtract(&toPos, &fromPos, &dir);
			geVec3d_AddScaled(&fromPos, &dir, t, &loc);
			if( !geWorld_Collision(World, NULL, NULL, &fromPos, &loc , GE_CONTENTS_SOLID, GE_COLLIDE_MESHES | GE_COLLIDE_MODELS, 0x00000000, 0 ,NULL, &lCol) ) {
				value = GE_TRUE;
				//enemy_civilian_doDamage(z, type, damage, &loc);
				enemy_zombie_doDamage(z, type, damage, &loc);
			}
			else {
				if( cheats.debug )
					console_message("stuff in the way of bullet");
			}
			// continue
		}

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}

	return value;
}