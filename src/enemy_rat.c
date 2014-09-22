#include "enemy_rat.h"
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

#define IN_DEG(x) (x)*GE_PI/180.0f

#define ENEMY_RAT_DETECT_RANGE	922694.0f
#define ENEMY_RAT_EAT_RANGE		12000.0f
#define ENEMY_RAT_DETECT_HIGHT	45.0f

geBoolean rat_created;

soundsys_sound rat_die;

void enemy_rat_init()
{
	rat_created = GE_FALSE;

	soundsys_loadWaw(".\\sfx\\enemies\\rat\\die.wav", &rat_die);
}

//Done when entering a anew level
void enemy_rat_newWorld(geWorld* World)
{
	geEntity_EntitySet	*Set;
	geEntity				*Entity;

	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Rat");
	if (Set == NULL) return;

	if( rat_created ) return;
	rat_created = GE_TRUE;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Make sure all the data is cleared when we alloc the actor data later on.
	// If the allocation somehow failed we don't want our app crashing when cleaning anything
	// we didn't allocate
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// get first entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);
	// wade thru them all

	while( Entity )
	{
		Inf_Enemy_Rat* r;
		
		// get data
		r = (Inf_Enemy_Rat *)geEntity_GetUserData(Entity);
		if( !r )
		{
			continue;
		}

		r->Actor = 0; // clear the only data that we will alocate later on
		r->XForm = 0;
		r->ExtBox = 0;

		// no memset, this will clear all the data from the editor, which is bad
		//memset(r, 0, sizeof(Inf_Enemy_Rat) );

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////


	// get first entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_Enemy_Rat* r;
		
		// get data
		r = (Inf_Enemy_Rat *)geEntity_GetUserData(Entity);
		if( !r )
		{
			continue;
		}

		r->ExtBox = (geExtBox*) malloc( sizeof(geExtBox) );
		if(! r->ExtBox )
		{
			run = 0;
			error("failed to allocate ExtBox");
			r->ExtBox = 0;
			continue;
		}
		// this is done later in the init code
		//geExtBox_Set(r->ExtBox, -20.0f, -40.0f, -20.0f, 20.0f, 20.0f, 20.0f); 
		//geExtBox_Set(r->ExtBox, 10.0f, 15.0f, 20.0f, -10.0f, 0.0f, -10.0f); 

		r->XForm = (geXForm3d*) malloc( sizeof(geXForm3d) );
		if(! r->XForm )
		{
			run = 0;
			error("failed to allocate xform");
			r->XForm = 0;
			continue;
		}
		memset(r->XForm, 0, sizeof(geXForm3d) );
		geXForm3d_SetIdentity(r->XForm );
		
		r->Orientation.X = IN_DEG(r->Orientation.X);
		r->Orientation.Y = IN_DEG(r->Orientation.Y);
		r->Orientation.Z = IN_DEG(r->Orientation.Z);

		geXForm3d_RotateX(r->XForm, r->Orientation.X );
		geXForm3d_RotateY(r->XForm, r->Orientation.Y );
		geXForm3d_RotateZ(r->XForm, r->Orientation.Z );
		r->XForm->Translation = r->Position;

		r->Actor = LoadActor(enemy_rat_def, World, 2.0f, GE_ACTOR_RENDER_NORMAL | GE_ACTOR_COLLIDE, r->XForm );
		r->health = 10;
		r->counter = 0.0f;
		r->preState = r->state = 0;

		if( ! r->Actor )
		{
			run = 0;
			error("Failed to create rat actor");
			continue;
		}

		// make sure we can acces this data later when we only got the actor
		geActor_SetUserData(r->Actor, (void*)r);

		setup_box(r->ExtBox, r->Actor, r->Position);
		Basic_ApplyGravity(r->Position, 150.0f, World, r->ExtBox, &(r->Position));

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}

}

//Done when leaveing a level
void enemy_rat_clearWorld(geWorld* World)
{
	geEntity_EntitySet	*Set;
	geEntity			*Entity;

	if( !rat_created ) return;
	rat_created = GE_FALSE;

	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Rat");
	if (Set == NULL) return;
	
	// get first rain entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_Enemy_Rat* r;
		
		// get data
		r = (Inf_Enemy_Rat *)geEntity_GetUserData(Entity);
		if( !r )
		{
			continue;
		}

		KillActor(World, &( r->Actor ));
		r->Actor = 0; // Just to make sure - KillActor() is supposed to take care of this

		if( r->XForm )
		{
			free(r->XForm );
			r->XForm = 0;
		}

		if( r->ExtBox )
		{
			free(r->ExtBox );
			r->ExtBox=0;
		}

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
}
/*
void enemy_rat_lookFromPos(geVec3d *pos, Inf_Enemy_Rat* r, float time, float speed)
{
	geVec3d rot;
	LookAt(r->Position, *pos, &rot);//get wanted angles
	geVec3d_Subtract(&rot, &(r->Orientation), &rot); // the change is stored in rot now
	rot.X = 0.0f;
	rot.Z = 0.0f;

	rot.Y -= GE_PI;
	r->Orientation.Y += rot.Y * time * speed;
}
*/

float getAngleBetwen(geVec3d a, geVec3d b)
{
	float cosangle = 0.0f;

	geVec3d_Normalize(&a);
	geVec3d_Normalize(&b);
	
	cosangle = geVec3d_DotProduct(&a, &b);

	return (float)acos((double)
		(cosangle)
		);
}

void getDirectionVector(geVec3d* from, geVec3d* point, geVec3d* Result)
{
	geVec3d_Subtract( from, point, Result );
}

// @@@ move this function to some smarter place
void enemy_rat_lookAtPos(geVec3d *pos, Inf_Enemy_Rat* r, float time)
{
	// the angle values
	float a = 0.0f;
	float b = 0.0f;
	//float c = 0.0f;
	//float d = 0.0f;
	geVec3d dirPosRat;// direction from pos to rat, not normalized
	geVec3d temp;// temporary vector
	
	// Get the wanted direction i a vector
	getDirectionVector(&r->Position, pos, &dirPosRat);
	dirPosRat.Y = 0; // make it on the plane



	geXForm3d_GetIn(r->XForm, &temp); // Get in
	temp.Y = 0.0f; // on the plane
	b = getAngleBetwen(dirPosRat, temp); // get the angles

	// This check(ie the return) will cause a slight performance hit
	// if the enemy is facing towards the point it doesn't need to steer
	// right now, perhaps later, but since this code is called each frame
	// we don't take care of that.
	// idea: if we don't want the enemy to fire while it's turning and the direction 
	if( b < GE_PI/12 )
	{
		//game_message("Don't");
		return;
	}

	geXForm3d_GetLeft(r->XForm, &temp); // Get right
	geVec3d_Scale(&temp, -1.0f, &temp); // make it the inverse
	temp.Y = 0.0f; // on the plane
	a = getAngleBetwen(dirPosRat, temp); // gwet the angles


	// we only need a, then we don't need to calculate theese 
	//geXForm3d_GetLeft(r->XForm, &temp); // Get left
	//temp.Y = 0.0f; // on the plane
	//c = getAngleBetwen(dirPosRat, temp); // get the angles

	//geXForm3d_GetIn(r->XForm, &temp); // get backwards
	//geVec3d_Scale(&temp, -1.0f, &temp); // make it the inverse
	//temp.Y = 0.0f; // on the plane
	//d = getAngleBetwen(dirPosRat, temp); // get the angles
	

	if( a < GE_PI/2.0f )
		r->Orientation.Y -= time * b;// rotate right
	else
		r->Orientation.Y += time * b;// rotate left
}

void apply_rats(geWorld* world){
	geEntity_EntitySet	*Set=0;
	geEntity			*Entity=0;
	geMotion			*Idle=0;
	geMotion			*Walk=0;
	geMotion			*Eat=0;

	if( !rat_created ) return;

	Idle = geActor_GetMotionByName(enemy_rat_def, "idle1" );
	Walk = geActor_GetMotionByName(enemy_rat_def, "walk" );
	Eat = geActor_GetMotionByName(enemy_rat_def, "eat" );

	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Rat");
	if (Set == NULL) return;
	
	// get first rain entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_Enemy_Rat* r;
		float floorDiff=0.0f;
		
		// get data
		r = (Inf_Enemy_Rat *)geEntity_GetUserData(Entity);
		if( !r )
		{
			continue;
		}

		
		///////////////////////////////////////////////////////
		// set the xform to the right pos
		geXForm3d_SetIdentity(r->XForm );
		geXForm3d_RotateX(r->XForm, r->Orientation.X );
		geXForm3d_RotateY(r->XForm, r->Orientation.Y + GE_PI);
		geXForm3d_RotateZ(r->XForm, r->Orientation.Z );
		r->XForm->Translation = r->Position;


		//////////////////////////////////////////////////
		// state evaluate
		switch( r->state )
		{
		case -1://dead
			r->XForm->Translation.Y += 10.0f;
			geActor_SetPose(r->Actor, Idle, 0.0f, r->XForm );
			break;
		case 0:
			while( r->counter > 1.58f )
				r->counter -= 1.58f;
			geActor_SetPose(r->Actor, Idle, r->counter, r->XForm );
			break;
		case 1:
			while( r->counter > 0.83f )
				r->counter -= 0.83f;
			geActor_SetPose(r->Actor, Walk, r->counter, r->XForm );
			break;
		case 2:
			while( r->counter > 1.08f )
			{
				r->counter -= 0.83f;
				damage( rand()%10+1 );
				//timedamage_add( TIMEDAMAGE_INFECTEDBITE );
			}
			geActor_SetPose(r->Actor, Eat, r->counter, r->XForm );
			break;
		}

		

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
}

#define DO_UPDATE Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
//Done evry frame
void enemy_rat_iterate(geWorld* World, float time)
{
	geEntity_EntitySet	*Set=0;
	geEntity			*Entity=0;
	geMotion			*Idle=0;
	geMotion			*Walk=0;
	geMotion			*Eat=0;
	geFloat				ds;

	if( !rat_created ) return;

	Idle = geActor_GetMotionByName(enemy_rat_def, "idle1" );
	Walk = geActor_GetMotionByName(enemy_rat_def, "walk" );
	Eat = geActor_GetMotionByName(enemy_rat_def, "eat" );

	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Rat");
	if (Set == NULL) return;
	
	// get first rain entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_Enemy_Rat* r;
		float floorDiff=0.0f;
		
		// get data
		r = (Inf_Enemy_Rat *)geEntity_GetUserData(Entity);
		if( !r )
		{
			continue;
		}

		r->counter += time;

		//////////////////////////////////////////////////
		// gravity takes hold

		//Basic_ApplyGravity(r->Position, 90.0f * time, World, r->ExtBox, &(r->Position));

		//////////////////////////////////////////////////
		// state change

		switch( r->state )
		{
		case -1: // dead
			break;
		case 0: // idle
			{
				ds = enemy_canAttack(Pos, r->Position, World);
				if( ds > 0.0f ){
					floorDiff = player_getPlayerFloor() - r->Position.Y; floorDiff = FLOAT_ABS(floorDiff);
					if( ds < ENEMY_RAT_DETECT_RANGE && floorDiff < ENEMY_RAT_DETECT_HIGHT )
					{
						//game_message("Rat changed state to 1");
						r->state = 1;
						r->counter = 0.0f;
					}
				} else {
					DO_UPDATE;
					continue;
				}

			}
			break;
		case 1: // walking towards target
			{
				ds = enemy_canAttack(Pos, r->Position, World);

				enemy_rat_lookAtPos(&Pos, r, time);
				Basic_MoveForward(r->Position, r->Orientation, 40.0f * time, World, r->ExtBox, &(r->Position) );

				if( ds < 0.0f )
				{
					// can't see
					r->state = 0;
					r->counter = 0.0f;
					//game_message("Rat changed state to 0");
				}
				else
				{
					floorDiff = player_getPlayerFloor() - r->Position.Y; floorDiff = FLOAT_ABS(floorDiff);
					if( floorDiff < ENEMY_RAT_DETECT_HIGHT )
					{
						if( ds < ENEMY_RAT_EAT_RANGE)
						{
							//game_message("Rat changed state to 2");
							r->state = 2;
							r->counter = RAND_FLOAT()*1.08f;
						}
						else
						{
							if( ds > ENEMY_RAT_DETECT_RANGE )
							{
								r->state = 0;
								r->counter = 0.0f;
								//game_message("Rat changed state to 0");
							}
						}
					}
					else
					{
						r->state = 0;
						r->counter = 0.0f;
					}	
				}
			}
			break;
		case 2: // eating
			{
				ds = enemy_canAttack(Pos, r->Position, World);

				enemy_rat_lookAtPos(&Pos, r, time);

				if( ds < 0.0f )
				{
					// can't see
					r->state = 0;
					r->counter = 0.0f;
					//game_message("Rat changed state to 0");
				}
				else
				{
					floorDiff = player_getPlayerFloor() - r->Position.Y; floorDiff = FLOAT_ABS(floorDiff);
					if( floorDiff < ENEMY_RAT_DETECT_HIGHT )
					{
						if( ds > ENEMY_RAT_EAT_RANGE )
						{
							if( ds < ENEMY_RAT_DETECT_RANGE )
							{
								r->state = 1;
								r->counter = 0.0f;
								//game_message("Rat changed state to 1");
							}
							else
							{
								r->state = 0;
								r->counter = 0.0f;
								//game_message("Rat changed state to 0");
							}
						}
					}
					else
					{
						r->state = 0;
						r->counter = 0.0f;
					}
				}
			}
			break;
		}

		if( r->health <= 0) r->state = -1;


		///////////////////////////////////////////////////////
		// set the xform to the right pos
		geXForm3d_SetIdentity(r->XForm );
		geXForm3d_RotateX(r->XForm, r->Orientation.X );
		geXForm3d_RotateY(r->XForm, r->Orientation.Y + GE_PI);
		geXForm3d_RotateZ(r->XForm, r->Orientation.Z );
		r->XForm->Translation = r->Position;


		//////////////////////////////////////////////////
		// state evaluate
		switch( r->state )
		{
		case -1://dead
			r->XForm->Translation.Y += 10.0f;
			geActor_SetPose(r->Actor, Idle, 0.0f, r->XForm );
			break;
		case 0:
			while( r->counter > 1.58f )
				r->counter -= 1.58f;
			geActor_SetPose(r->Actor, Idle, r->counter, r->XForm );
			break;
		case 1:
			while( r->counter > 0.83f )
				r->counter -= 0.83f;
			geActor_SetPose(r->Actor, Walk, r->counter, r->XForm );
			break;
		case 2:
			while( r->counter > 1.08f )
			{
				r->counter -= 0.83f;
				damage( rand()%10+1 );
				//timedamage_add( TIMEDAMAGE_INFECTEDBITE );
			}
			geActor_SetPose(r->Actor, Eat, r->counter, r->XForm );
			break;
		}

		

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

geBoolean enemy_rat_isAlive(geActor* enemy)
{
	Inf_Enemy_Rat* r;

	r = (Inf_Enemy_Rat*) geActor_GetUserData(enemy);
	if( !r )
	{
		error("Convertion error in enemy_rat_isAlive");
		return GE_FALSE;
	}

	if( r->health > 0 )
	{
		return GE_TRUE;
	}
	
	return GE_FALSE;
}


geBoolean enemy_rat_damage(geActor* enemy, // the enemy that got hit
				  unsigned char damage, // how many point damage does this weaopon damage do?
				  char type, // damage type, se above
				  /* Location Based Damage Data */
				  geVec3d fromPos, //fromPos - location of the shooter
				  geVec3d toPos // toPos - location of the weapon max range
				  )
{
	Inf_Enemy_Rat* r;
	geExtBox eb;

	r = (Inf_Enemy_Rat*) geActor_GetUserData(enemy);
	if( !r )
	{
		error("Convertion error in enemy_rat_damage");
		return GE_FALSE;
	}

	
	//eb.Min = r->ExtBox->Min;
	//eb.Max = r->ExtBox->Max;

	//geVec3d_Add(&r->ExtBox->Min, &r->Position, &eb.Min);
	//geVec3d_Add(&r->ExtBox->Max, &r->Position, &eb.Max);
	geActor_GetDynamicExtBox(r->Actor, &eb);


/*
// Collides a ray with box B.  The ray is directed, from Start to End.  
//   Only returns a ray hitting the outside of the box.  
//     on success, GE_TRUE is returned, and 
//       if T is non-NULL, T is returned as 0..1 where 0 is a collision at Start, and 1 is a collision at End
//       if Normal is non-NULL, Normal is the surface normal of the box where the collision occured.
geBoolean GENESISCC geExtBox_RayCollision( const geExtBox *B, const geVec3d *Start, const geVec3d *End, 
								geFloat *T, geVec3d *Normal );*/
	if(! geExtBox_RayCollision( &eb, &fromPos, &toPos, 
								NULL, NULL ) // don't save the data
								)
	{
		return GE_TRUE;
	}

	{
		BloodExplosion(r->Position);
	}

	if( r->health > 0 )
	{
		r->health -= damage;

		if( r->health <= 0)
		{
			//game_message("The rat died");
			r->Orientation.X = GE_PI;
			r->Position.Y += 10.0f;
			r->state = -1;
			talkPower_Change( 5.0f );
			soundsys_play3dsound(&rat_die, &r->Position, 3.0f, GE_FALSE);
		}
		else
		{
			//game_message("The rat was damaged");
		}
	}
	else
	{
		//game_message("The rat is dead");
		soundsys_play3dsound(&e_splat, &r->Position, 5.0f, GE_FALSE);
	}

	return GE_FALSE;
}

int handle_Vector(SaveFile* file, geVec3d* value);/*{
	FLOAT(value->X, "Failed to handle X");
	FLOAT(value->Y, "Failed to handle Y");
	FLOAT(value->Z, "Failed to handle Z");
	return 1;
}*/

#define VECTOR(value, message)		if(! handle_Vector(file, &value) )	{error(message); return 0;}

int handle_rat_enemy(SaveFile* file, geWorld* world){
	geEntity_EntitySet	*Set;
	geEntity			*Entity;

	INT( rat_created, "Failed to save rat creation");
	if( !rat_created ) return 1;

	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Rat");
	if (Set == NULL) return 1;
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_Enemy_Rat* r;
		
		// get data
		r = (Inf_Enemy_Rat *)geEntity_GetUserData(Entity);

		INT( r->state, "Failed to handle rat state")
		INT( r->health, "Failed to handle rat health");
		INT( r->preState, "Failed to handle rat preState");
		FLOAT( r->counter, "Failed to handle rat counter");
		VECTOR( r->Position, "Failed to handle rat postion");
		VECTOR( r->Orientation, "Failed to handle rat orientation");

		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}

	return 1;
}