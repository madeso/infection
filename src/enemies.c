#include "enemies.h"
#include "genesis.h"
#include "enemy_rat.h"
#include "inf_actor.h"
#include "log.h"

// I really shouldn't include a gloabl function in something that should
// be self sufficient, but I do this now.
#include "globals.h" 

void apply_enemies(geWorld* world){
	apply_rats(world);
}

int handle_enemy(SaveFile* file, geWorld* world){
	if(! handle_rat_enemy(file, world) ) return 0;
	apply_enemies(world);
	return 1;
}

// done at the beginning of the game
geBoolean enemy_init()
{
	soundsys_loadWaw(".\\sfx\\enemies\\splat.wav", &e_splat);
	// load the actor rat def
	enemy_rat_def = LoadActorDef(".\\actors\\rat.act");
	printLog(enemy_rat_def ? "Rat loaded ok.\n":"Rat loaded bad.\n" );

	// return GE_FALSE if we didn't succeed
	if(! enemy_rat_def ) return GE_FALSE;

	enemy_rat_init();

	return GE_TRUE;
}

//Done at the end of the game
void enemy_erase()
{
	geActor_DefDestroy(& enemy_rat_def ); enemy_rat_def = 0;
}


//Done when entering a anew level
void enemy_newWorld(geWorld* World)
{
	enemy_rat_newWorld(World);
}

//Done when leaveing a level
void enemy_clearWorld(geWorld* World)
{
	enemy_rat_clearWorld(World);
}

//Done evry frame
void enemy_iterate(geWorld* World, float time)
{
	enemy_rat_iterate(World, time);
}

/* Damage types 
#define DAMAGE_ARMORPIERCING	1
#define DAMAGE_IMPALING			2
#define DAMAGE_CRUSHING			3
#define DAMAGE_NORMAL			4
*/

geBoolean enemy_damage(geActor* enemy, // the enemy that got hit
				  unsigned char damage, // how many point damage does this weaopon damage do?
				  char type, // damage type, se above
				  /* Location Based Damage Data */
				  geVec3d fromPos, //fromPos - location of the shooter
				  geVec3d toPos // toPos - location of the weapon max range
				  )
{
	geActor_Def* def=0;
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
		return enemy_rat_damage(enemy, damage, type, fromPos, toPos);
	}
	else
	{
		error("Error in code in enemy_damage(): #1");
	}

	return GE_TRUE;
}

geBoolean enemy_isAlive(geActor* enemy)
{
	geActor_Def* def=0;
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
	}
	else
	{
		error("Error in code in enemy_isAlive(): #1");
	}

	return GE_TRUE;
}

void Basic_MoveForward(geVec3d pos, geVec3d rot, float speed, geWorld* World, geExtBox *eb, geVec3d* newPos)
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
		}
	}
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
		NULL,
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

	if( geWorld_Collision(
		World,
		NULL,
		NULL,
		&a,
		&b,
		GE_CONTENTS_SOLID_CLIP,
		GE_COLLIDE_ALL,
		0x00000000, //changed this from all f's to all zeros, so that this function worked, is the right thing to do? The reaseon why it didn't work was probably of the actor flags of the rat.
		NULL,
		NULL,
		&Col
		)//end of function call
		)//end of if
	{
		//system_message("A rat can't see you");
		return -1.0f;
	}

	// distance on the plane.
	a.Y = 0.0f;
	b.Y = 0.0f;

	geVec3d_Subtract(&a, &b, &a);
	return geVec3d_LengthSquared(&a);
}