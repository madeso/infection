#include "enemies.h"
#include "genesis.h"
#include "enemy_rat.h"
#include "inf_actor.h"
#include "log.h"
#include "enemy_zombie.h"
#include "globalGenesis.h"
#include "damage.h"
#include "player.h" // player_is_alive
#include "math.h"
#include "inf_message_system.h"
#include "enemy_civilian.h"
#include "enemy_def.h"

// I really shouldn't include a gloabl function in something that should
// be self sufficient, but I do this now.
#include "globals.h" 

void apply_enemies(geWorld* world){
	apply_rats(world);
	apply_zombies(world);
	apply_civilian(world);
}

int handle_enemy(SaveFile* file, geWorld* world){
	if(! handle_rat_enemy(file, world) ) return 0;
	if(! handle_zombie_enemy(file, world) ) return 0;
	if(! handle_civilian_enemy(file, world) ) return 0;
	apply_enemies(world);
	return 1;
}

// done at the beginning of the game
geBoolean enemy_init()
{
	int i = 0;
	enemy_rat_def = 0;
	enemy_zombie_def = 0;

	soundsys_loadSound(".\\sfx\\enemies\\splat.wav", 1, &e_splat, 1, TYPE_3D);
	// load the actor rat def
	enemy_rat_def = LoadActorDef(".\\actors\\rat.act");
	printLog(enemy_rat_def ? "Rat loaded ok.\n":"Rat loaded bad.\n" );

	// return GE_FALSE if we didn't succeed
	if(! enemy_rat_def ) return GE_FALSE;

	enemy_zombie_def = LoadActorDef(".\\actors\\zombie.act");
	printLog(enemy_zombie_def ? "Zombie loaded ok.\n":"Zombie loaded bad.\n" );
	if(! enemy_zombie_def ) return GE_FALSE;

	enemy_civilian_def[0] = LoadActorDef(".\\actors\\CivilianS1.act");
	enemy_civilian_def[1] = LoadActorDef(".\\actors\\CivilianS2.act");
	enemy_civilian_def[2] = LoadActorDef(".\\actors\\CivilianS3.act");
	
	for(i=0; i<NUMBER_OF_CIVILIANS; i++){
		printLog(enemy_civilian_def[i] ? "Civilian loaded ok.\n":"Civilian loaded bad.\n" );
		if(! enemy_civilian_def[i] ) return GE_FALSE;
	}


	enemy_rat_init();
	enemy_zombie_init();
	enemy_civilian_init();

	return GE_TRUE;
}

//Done at the end of the game
void enemy_erase()
{
	int i = 0;
	if( enemy_rat_def ) {
		geActor_DefDestroy(& enemy_rat_def ); enemy_rat_def = 0;
	}
	if( enemy_zombie_def ) {
		geActor_DefDestroy(& enemy_zombie_def ); enemy_zombie_def = 0;
	}
	for(i=0; i<NUMBER_OF_CIVILIANS; i++){
		if( enemy_civilian_def[0] ) {
			geActor_DefDestroy(& enemy_civilian_def[0] );
			enemy_civilian_def[0] = 0;
		}
	}
}


//Done when entering a anew level
void enemy_newWorld(geWorld* World)
{
	enemy_rat_newWorld(World);
	enemy_zombie_newWorld(World);
	enemy_civilian_newWorld(World);
}

//Done when leaveing a level
void enemy_clearWorld(geWorld* World)
{
	enemy_rat_clearWorld(World);
	enemy_zombie_clearWorld(World);
	enemy_civilian_clearWorld(World);
}

//Done evry frame
void enemy_iterate(geWorld* World, float time)
{
	enemy_rat_iterate(World, time);
	enemy_zombie_iterate(World, time);
	enemy_civilian_iterate(World, time);
}

/* Damage types 
#define DAMAGE_ARMORPIERCING	1
#define DAMAGE_IMPALING			2
#define DAMAGE_CRUSHING			3
#define DAMAGE_NORMAL			4
*/

geBoolean enemy_damage(geActor* enemy, // the enemy that got hit
				  int damage, // how many point damage does this weaopon damage do?
				  char type, // damage type, se above
				  char lbd,
				  /* Location Based Damage Data */
				  geVec3d fromPos, //fromPos - location of the shooter
				  geVec3d toPos // toPos - location of the weapon max range
				  )
{
	int i = 0;
	geActor_Def* def=0;

	if( ! enemy ) {
		damagex(damage, type);
		return GE_TRUE;
	}
	def = geActor_GetActorDef( enemy );

	if( !def ) {
		error("Failed to get the definition in enemy_damage()");
		return GE_FALSE;
	}

	if( def == enemy_rat_def ){
		return enemy_rat_damage(enemy, damage, type, lbd, fromPos, toPos);
	}
	else if( def== enemy_zombie_def) {
		return enemy_zombie_damage(enemy, damage, type, lbd, fromPos, toPos);
	}
	for(i=0; i<NUMBER_OF_CIVILIANS; i++) {
		if( def== enemy_civilian_def[i]) {
			return enemy_civilian_damage(enemy, damage, type, lbd, fromPos, toPos);
		}
	}

	return GE_TRUE;
}

geBoolean enemy_loopDamage(int damage, // how many point damage does this weaopon damage do?
				  char type, // damage type, se above
				  char lbd,
				  /* Location Based Damage Data */
				  geVec3d fromPos, //fromPos - location of the shooter
				  geVec3d toPos // toPos - location of the weapon max range
				  ){
	geBoolean ret = GE_FALSE;
	if( enemy_civilian_loopDamage	(damage, type, lbd, fromPos, toPos) )
		ret = GE_TRUE;
	if( enemy_zombie_loopDamage		(damage, type, lbd, fromPos, toPos) )
		ret = GE_TRUE;
	return GE_TRUE;
}

geBoolean enemy_isAlive(geActor* enemy)
{
	geActor_Def* def=0;
	int i;

	if(! enemy ){
		if( hero_hit_points > 0 ) return GE_TRUE;
		return GE_FALSE;
	}

	// get the definition
	def = geActor_GetActorDef( enemy );

	if( !def )
	{
		error("Failed to get the definition in enemy_damage()");
		return GE_FALSE;
	}

	if( def == enemy_rat_def )
	{
		// the user got the rat
		return enemy_rat_isAlive(enemy);
	} else if( def== enemy_zombie_def) {
		return enemy_zombie_isAlive(enemy);
	}

	for(i=0; i<NUMBER_OF_CIVILIANS; i++) {
		if( def== enemy_civilian_def[i])
			return enemy_civilian_isAlive(enemy);
	}

	return GE_TRUE;
}

geVec3d* enemy_getPosition(geActor* act){
	geActor_Def* def=0;
	int i=0;

	if(! act ){
		return &( XForm.Translation );
	}

	def = geActor_GetActorDef( act );
	if( !def ) {
		error("Failed to get the definition in enemy_damage()");
		return 0;
	}

	if( def == enemy_rat_def ) {
		return enemy_rat_getPosition(act);
	} else if( def== enemy_zombie_def) {
		return enemy_zombie_getPosition(act);
	}
	
	for(i=0; i<NUMBER_OF_CIVILIANS; i++) {
		if( def== enemy_civilian_def[i])
			return enemy_civilian_getPosition(act);
	}

	return 0;
}

char Basic_MoveForward(geVec3d pos, geVec3d rot, float speed, geWorld* World, geExtBox *eb, geVec3d* newPos)
{
	geXForm3d xf;
	geVec3d in;
	GE_Collision Col;
	
	geXForm3d_SetIdentity(&xf);
	geXForm3d_RotateX(&xf, rot.X);
	geXForm3d_RotateY(&xf, rot.Y);
	geXForm3d_RotateZ(&xf, rot.Z);
	geXForm3d_GetIn(&xf, &in);
	in.Y = 0.0f;//on the plane
	geVec3d_AddScaled(&pos, &in, speed, newPos);
	
	if( geWorld_Collision(
		World,
		&(eb->Min),
		&(eb->Max),
		&pos,
		newPos,
		GE_CONTENTS_SOLID_CLIP,
		GE_COLLIDE_ALL,
		0xffffffff,
		cb_move,
		NULL,
		&Col
		)//end of function call
		)//end of if
	{
		geFloat slide = 0.0f;

		slide = geVec3d_DotProduct (newPos, &Col.Plane.Normal) - Col.Plane.Dist;
		//re-position the player
		newPos->X -= Col.Plane.Normal.X * slide;
		newPos->Y -= Col.Plane.Normal.Y * slide;
		newPos->Z -= Col.Plane.Normal.Z * slide;
		
		if( geWorld_Collision(
			World,
			&(eb->Min),
			&(eb->Max),
			&pos,
			newPos,
			GE_CONTENTS_SOLID_CLIP,
			GE_COLLIDE_ALL,
			0xffffffff,
			cb_move,
			NULL,
			&Col
			)//end of function call
			)//end of if
		{
			*newPos = Col.Impact;
			/*if( Col.Actor ) return 1;
			return 0;*/
		}

		if( Col.Actor ) return 1;
		if( getAngleBetwen(&in, &(Col.Plane.Normal) ) > 2.9f ) return 0;
	}
	return 1;
}

void Basic_ApplyGravity(geVec3d pos, float gravity, geWorld* World, geExtBox *eb, geVec3d *newPos)
{
	GE_Collision Col;

	*newPos = pos;
	newPos->Y -= gravity;

	if( geWorld_Collision(
		World,
		&(eb->Min),
		&(eb->Max),
		&pos,
		newPos,
		GE_CONTENTS_SOLID_CLIP,
		GE_COLLIDE_ALL,
		0xffffffff,
		cb_move,
		NULL,
		&Col
		)//end of function call
		)//end of if
	{
		*newPos = Col.Impact;
	}
}

float enemy_canAttack(geVec3d a, geVec3d b, geWorld* World)
{
	GE_Collision Col;
	float length;
	a.Y += 10.0f;
	b.Y += 10.0f;

	if( geWorld_Collision(
		World,
		NULL,
		NULL,
		&a,
		&b,
		GE_CONTENTS_SOLID,
		(GE_COLLIDE_MESHES | GE_COLLIDE_MODELS),
		0x00000000, //changed this from all f's to all zeros, so that this function worked, is the right thing to do? The reaseon why it didn't work was probably of the actor flags of the rat.
		NULL,
		NULL,
		&Col
		)//end of function call
		)//end of if
	{
		//system_message("A rat can't see you");
		//if(! Col.Model )
		return -1.0f;
	}

	// distance on the plane.
	a.Y = 0.0f;
	b.Y = 0.0f;

	geVec3d_Subtract(&a, &b, &a);
	length = geVec3d_LengthSquared(&a);
	return length;
}

float enemy_distanceBetween(geVec3d a, geVec3d b, geWorld* World)
{
	// distance on the plane.
	a.Y = 0.0f;
	b.Y = 0.0f;

	geVec3d_Subtract(&a, &b, &a);
	return geVec3d_LengthSquared(&a);
}

void enemy_explosionDamage(geVec3d* location, float range, int damage){
	enemy_rat_explosionDamage(location, range, damage);
	enemy_zombie_explosionDamage(location, range, damage);
	enemy_civilian_explosionDamage(location, range, damage);
}
void enemy_fireExplosionDamage(geVec3d* location, float range, int damage){
	system_message("update fire explosion to include rat");
	enemy_rat_explosionDamage(location, range, damage);
	enemy_zombie_fireExplosionDamage(location, range, damage);
	enemy_civilian_fireExplosionDamage(location, range, damage);
}

void getClosestPosition(geXForm3d* enemy, int flags, Monster* monster){
	float distance;
	geVec3d* pos;
	geEntity_EntitySet	*Set=0;
	geEntity			*Entity=0;

	monster->used = GE_FALSE;
	monster->distance = 0.0f;
	//monster->position;
	monster->actor = 0;

//#define TEST_BEST_VALUE(pos, value, actor) { if(*value>bestValue || bestPos==0) { bestPos=pos; bestValue=*value; } }
#define TEST_BEST_VALUE(act)	 { if( distance > 0.0f ) { if(distance<monster->distance || !monster->used) { monster->position = *pos; monster->distance = distance; monster->used = GE_TRUE; monster->actor = act; } } }


	if( flags & MONSTER_FLAGS_PLAYER ) {
		if( hero_hit_points > 0 ) {
			pos = &XForm.Translation;
			distance = enemy_canAttack(enemy->Translation, *pos, World);
			TEST_BEST_VALUE(0);
		}
	}

	if( flags & MONSTER_FLAGS_ZOMBIE ) {
		Set = geWorld_GetEntitySet(World, "Inf_Enemy_Zombie");
		if (Set != NULL) {
			Entity = geEntity_EntitySetGetNextEntity(Set, NULL);
			while( Entity ) {
				Inf_Enemy_Zombie* r;
				r = (Inf_Enemy_Zombie *)geEntity_GetUserData(Entity);
				// do stuff
				if( r->Health > 0 ) {
					pos = &(r->Position);
					distance = enemy_canAttack(enemy->Translation, *pos, World);
					TEST_BEST_VALUE(r->Actor);
				}
				Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
			}
		}
		Set = 0;
		Entity = 0;
	}

	if( flags & MONSTER_FLAGS_FRIENDLY ) {
		Set = geWorld_GetEntitySet(World, "Inf_Enemy_Civilian");
		if (Set != NULL) {
			Entity = geEntity_EntitySetGetNextEntity(Set, NULL);
			while( Entity ) {
				Inf_Enemy_Civilian* r;
				r = (Inf_Enemy_Civilian *)geEntity_GetUserData(Entity);
				// do stuff
				if( r->Health > 0 ) {
					pos = &(r->Position);
					distance = enemy_canAttack(enemy->Translation, r->Position, World);
					TEST_BEST_VALUE(r->Actor);
					if( monster->used ) {
						int i=0;
						i++;
					}
				}
				Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
			}
		}
		Set = 0;
		Entity = 0;
	}
}


float getAngleBetwen(const geVec3d* pa, const geVec3d* pb)
{
	float cosangle = 0.0f;
	geVec3d a = *pa;
	geVec3d b = *pb;

	geVec3d_Normalize(&a);
	geVec3d_Normalize(&b);
	
	cosangle = geVec3d_DotProduct(&a, &b);

	return (float)acos((double)
		(cosangle)
		);
}

void getDirectionVector(const geVec3d* from, const geVec3d* point, geVec3d* Result)
{
	geVec3d_Subtract( from, point, Result );
}

int enemy_lookAtPos(const geVec3d *pos, const geXForm3d* eXForm, geVec3d* eRotation, float time)
{
	// the angle values
	float a = 0.0f;
	float b = 0.0f;
	//float c = 0.0f;
	//float d = 0.0f;
	geVec3d dirPos;// direction from pos to enemy, not normalized
	geVec3d temp;// temporary vector
	
	// Get the wanted direction i a vector
	getDirectionVector(&( eXForm->Translation ), pos, &dirPos);
	dirPos.Y = 0; // make it on the plane



	geXForm3d_GetIn(eXForm, &temp); // Get in
	temp.Y = 0.0f; // on the plane
	b = getAngleBetwen(&dirPos, &temp); // get the angles

	// This check(ie the return) will cause a slight performance hit
	// if the enemy is facing towards the point it doesn't need to steer
	// right now, perhaps later, but since this code is called each frame
	// we don't take care of that.
	// idea: if we don't want the enemy to fire while it's turning and the direction 
	if( b < GE_PI/12 )
	{
		//game_message("Don't");
		return EN_NOT_TURNED;
	}

	geXForm3d_GetLeft(eXForm, &temp); // Get right
	geVec3d_Scale(&temp, -1.0f, &temp); // make it the inverse
	temp.Y = 0.0f; // on the plane
	a = getAngleBetwen(&dirPos, &temp); // get the angles


	// from the rat code:
	// old and is here only because it may be needed later.
	// note that it needs to be converterted when used.
	// the names and structs are specific to the rat monster.

	// we only need a, then we don't need to calculate theese 
	//geXForm3d_GetLeft(r->XForm, &temp); // Get left
	//temp.Y = 0.0f; // on the plane
	//c = getAngleBetwen(dirPosRat, temp); // get the angles

	//geXForm3d_GetIn(r->XForm, &temp); // get backwards
	//geVec3d_Scale(&temp, -1.0f, &temp); // make it the inverse
	//temp.Y = 0.0f; // on the plane
	//d = getAngleBetwen(dirPosRat, temp); // get the angles	

	if( a < GE_PI/2.0f )
		eRotation->Y -= time * b;// rotate right
	else
		eRotation->Y += time * b;// rotate left
	return EN_TURNED;
}

int enemy_fleeFromPos(const geVec3d *pos, const geXForm3d* eXForm, geVec3d* eRotation, float time)
{
	// the angle values
	float a = 0.0f;
	float b = 0.0f;
	//float c = 0.0f;
	//float d = 0.0f;
	geVec3d dirPos;// direction from pos to enemy, not normalized
	geVec3d temp;// temporary vector
	
	// Get the wanted direction i a vector
	getDirectionVector(&( eXForm->Translation ), pos, &dirPos);
	dirPos.Y = 0; // make it on the plane



	geXForm3d_GetIn(eXForm, &temp); // Get in
	temp.Y = 0.0f; // on the plane
	b = getAngleBetwen(&dirPos, &temp); // get the angles

	// This check(ie the return) will cause a slight performance hit
	// if the enemy is facing towards the point it doesn't need to steer
	// right now, perhaps later, but since this code is called each frame
	// we don't take care of that.
	// idea: if we don't want the enemy to fire while it's turning and the direction 
	if( b > (GE_PI - GE_PI/12) )
	{
		//game_message("Don't");
		return EN_NOT_TURNED;
	}

	geXForm3d_GetLeft(eXForm, &temp); // Get right
	geVec3d_Scale(&temp, -1.0f, &temp); // make it the inverse
	temp.Y = 0.0f; // on the plane
	a = getAngleBetwen(&dirPos, &temp); // get the angles


	// from the rat code:
	// old and is here only because it may be needed later.
	// note that it needs to be converterted when used.
	// the names and structs are specific to the rat monster.

	// we only need a, then we don't need to calculate theese 
	//geXForm3d_GetLeft(r->XForm, &temp); // Get left
	//temp.Y = 0.0f; // on the plane
	//c = getAngleBetwen(dirPosRat, temp); // get the angles

	//geXForm3d_GetIn(r->XForm, &temp); // get backwards
	//geVec3d_Scale(&temp, -1.0f, &temp); // make it the inverse
	//temp.Y = 0.0f; // on the plane
	//d = getAngleBetwen(dirPosRat, temp); // get the angles	

	if( a < GE_PI/2.0f )
		eRotation->Y += time * b;// rotate right
	else
		eRotation->Y -= time * b;// rotate left
	return EN_TURNED;
}