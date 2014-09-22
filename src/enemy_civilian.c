#include "enemy_civilian.h"
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
#include "globals.h"
#include "infection.h" // hande_Vector function
#include "fxbitmaps.h" // BloodExplosion function
#include "console.h" // so that we can execute commands when we are used
#include "inf_lua.h"
#include "pickup.h"

extern geFloat frand(geFloat, geFloat);

#define UPDATE_CLOSEST			getClosestPosition( c->XForm, MONSTER_FLAGS_OF_FRIENDLY, c->closest )
#define CAN_SEE_ENEMY			c->closest->used
#define FIFTY_PERCENTS_CHANCE	rand()%2 == 0
#define IN_RANGE_OF_ENEMY		(c->closest->distance < ZOMBE_SLASH_RANGE_SQ && c->closest->used)

geMotion			*CivilianIdle[NUMBER_OF_CIVILIANS]={0,};
geMotion			*CivilianWalk[NUMBER_OF_CIVILIANS]={0,};
geMotion			*CivilianRun[NUMBER_OF_CIVILIANS]={0,};
geMotion			*CivilianDead[NUMBER_OF_CIVILIANS]={0,};
geMotion			*CivilianScared[NUMBER_OF_CIVILIANS]={0,};

// standing
#define CIV_DEAD			0 // 1.00
#define CIV_IDLE			1 // 1.29
#define CIV_WALKING_FROM	2 // 1.00
#define CIV_SCARED			4 // 1.29
#define CIV_WAITING			5 // 1.29
#define CIV_TURN_R			6 // 1.29
#define CIV_TURN_L			7 // 1.29
#define CIV_WALKING_PLAYER	8 // 1.00
#define CIV_RUNNING_PLAYER	9 // 0.96
#define CIV_SCARED_FLEE		10

#define AI_STANDING			0
#define AI_WALKING			1
#define AI_FOLLOW			2
#define AI_WAITING			3

#define WALKING_DISTANCE 120.0f
#define RUNNING_DISTANCE 280.0f

#define WALKING_DISTANCE_SQ  14400.0f
#define RUNNING_DISTANCE_SQ  78400.0f

#define FLEE_RANGE			 144000.0f

#define WALK_SPEED 120.0f
#define RUN_SPEED 320.0f

soundsys_sound civilian_saving_sounds[NUMBER_OF_CIVILIANS][5];
soundsys_sound civilian_leave_sounds[NUMBER_OF_CIVILIANS][5];
soundsys_sound civilian_take_sounds[NUMBER_OF_CIVILIANS][5];
soundsys_sound civilian_hello_sounds[NUMBER_OF_CIVILIANS][5];
soundsys_sound civilian_death_sounds[NUMBER_OF_CIVILIANS][5];
soundsys_sound civilian_afraid_sounds[NUMBER_OF_CIVILIANS][6];


#define LOAD_SOUND_FILE(file,array,face,index)	soundsys_loadSound(file, 1, &(array[0][index]), 1, TYPE_3D); soundsys_loadSound(file, 1, &(array[1][index]), 1, TYPE_3D); soundsys_loadSound(file, 1, &(array[2][index]), 1, TYPE_3D)
#define SAY_SOMETHING(count, sounds, screem)\
int index = rand()%count;\
geVec3d loc = civ->Position; \
loc.Y += 10.0f; \
soundsys_play3dsound( &(sounds[civ->Look][index]), &loc, ((screem)?30:15.0f), GE_TRUE, GE_FALSE);

void civilian_say_saving(Inf_Enemy_Civilian* civ){
	//game_message("Civilian: Thanks for saving me");
	SAY_SOMETHING(5, civilian_saving_sounds, 0);
}
void civilian_say_leave(Inf_Enemy_Civilian* civ){
	//game_message("Civilian: I'll stay here");
	SAY_SOMETHING(5, civilian_leave_sounds, 0);
}
void civilian_say_take(Inf_Enemy_Civilian* civ){
	//game_message("Civilian: Take this");
	SAY_SOMETHING(5, civilian_take_sounds, 0);
}
void civilian_say_hello(Inf_Enemy_Civilian* civ){
	//game_message("Civilian: Hello friend");
	SAY_SOMETHING(5, civilian_hello_sounds, 0);
}
void civilian_say_death(Inf_Enemy_Civilian* civ){
	//game_message("Civilian: Aaaaaa");
	SAY_SOMETHING(5, civilian_death_sounds, 0);
}
void civilian_say_afraid(Inf_Enemy_Civilian* civ){
	//game_message("Civilian: Help");
	SAY_SOMETHING(6, civilian_afraid_sounds, 1);
}
void civilian_setAfraidTime(Inf_Enemy_Civilian* civ){
	civ->afraidTime = frand(2.0f, 10.0f);
}

geBoolean can_use_actor(geActor* act) {
	Inf_Enemy_Civilian* z;
	geActor_Def* def=0;
	int i=0;
	int doit = 0;

	def = geActor_GetActorDef( act );
	if( !def ) {
		error("Failed to get the definition in can_use_actor(geActor*)");
		return 0;
	}

	for(i=0;i<NUMBER_OF_CIVILIANS; i++) {
		if( def == enemy_civilian_def[i] ){
			doit = 1;
			break;
		}
	}
	if(!doit) return GE_FALSE;


	geActor_GetUserData(act);
	z = (Inf_Enemy_Civilian*) geActor_GetUserData(act);
	if( !z ){
		error("Convertion error in talkto_cvilian");
		return 0;
	}

	if( z->Give ) return GE_TRUE;

	if( z->Health <= 0 ) return GE_FALSE;

	if( z->ai == AI_FOLLOW ) {
		return GE_TRUE;
	}
	else {
		if( z->ai == AI_WAITING ) {
			return GE_TRUE;
		}
		else {
			if( !is_string_null(z->cmdUse) ) {
				return GE_TRUE;
			}
		}
	}

	return GE_FALSE;
}

void civilian_setAI(const char* name, int ai) {
	geEntity_EntitySet	*Set=0;
	geEntity			*Entity=0;
	
	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Civilian");
	if (Set == NULL) return;
	
	// get first entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_Enemy_Civilian* r;
	
		r = (Inf_Enemy_Civilian *)geEntity_GetUserData(Entity);
		
		if( strcmp(name, r->name)==0 ) {
			r->ai = ai;
			return;
		}

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
	console_message("Failed to find civilian entity");
}

LUA_FUNC(civilian_setAI) {
	int args;
	if( (args = lua_gettop(luaVM)) != 2 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}
	// does it work?
	civilian_setAI(lua_tostring(luaVM, -2), (int)lua_tonumber(luaVM, -1) );
	lua_pushnumber(luaVM, 1);
	return 1;
}

void civ_renderNameNow(geActor* act) {
	Inf_Enemy_Civilian* z;
	int x;
	int y;
	int i;
	int doit=0;
	geActor_Def* def=0;

	GE_RGBA color;

	color.r = color.g = color.b = 255.0f;
	color.a = 255.0f;

	def = geActor_GetActorDef( act );
	if( !def ) {
		error("Failed to get the definition in talkto_civilian()");
		return;
	}

	for(i=0;i<NUMBER_OF_CIVILIANS; i++) {
		if( def == enemy_civilian_def[i] ){
			doit = 1;
			break;
		}
	}
	if(!doit) return;

	geActor_GetUserData(act);
	z = (Inf_Enemy_Civilian*) geActor_GetUserData(act);
	if( !z ){
		error("Convertion error in talkto_cvilian");
		return;
	}

	// optimize me???
	if( is_string_null(z->displayName) ) return;

	// render name
	y = Height / 2 + 20;
	x = Width / 2;
	// modify x by string value
	x -= XFont_GetStringWidth(XFontMgr_GetFont(fntMgr,kFontSmall), z->displayName) / 2;
	y -= XFont_GetStringHeight(XFontMgr_GetFont(fntMgr,kFontSmall), z->displayName) / 2;
	XFontMgr_PrintAt(fntMgr, x, y, kFontSmall, color, Camera, "%s", z->displayName );

}

void civilian_renderName() {
	geVec3d to;
	geVec3d in;
	GE_Collision Col;
	geExtBox eb;
	const float MIN = 0.1f;
	geExtBox_Set(&eb, -1.0f * MIN , -1.0f * MIN , -1.0f * MIN , MIN, MIN, MIN);

	geXForm3d_GetIn(&XForm, &in);
	geVec3d_AddScaled(&(XForm.Translation), &in, 80.0f, &to);
	
	if( geWorld_Collision(
		World,
		&(eb.Min),
		&(eb.Max),
		&(XForm.Translation),
		&to,
		GE_CONTENTS_SOLID_CLIP,
		GE_COLLIDE_ALL,
		0xffffffff,
		NULL,
		NULL,
		&Col
		)//end of function call
		)//end of if
	{
		if( Col.Actor ) {
			civ_renderNameNow(Col.Actor);
		}
	}
}

void civilian_applyAndEvaluateState(Inf_Enemy_Civilian* c, float time) {
	geXForm3d_SetIdentity(c->XForm );
	geXForm3d_RotateX(c->XForm, c->Orientation.X );
	geXForm3d_RotateY(c->XForm, c->Orientation.Y + GE_PI);
	geXForm3d_RotateZ(c->XForm, c->Orientation.Z );
	c->XForm->Translation = c->Position;

	Basic_ApplyGravity(c->Position, 180.0f * time, World, c->ExtBox, &( c->Position ));

	c->counter += time;

	// test for "scare"
	switch(c->state) {
	case CIV_DEAD:
	case CIV_SCARED:
	case CIV_SCARED_FLEE:
		break;
	default:
		UPDATE_CLOSEST;
		if( c->closest->used ) {
			if( c->closest->distance < FLEE_RANGE ) {
				c->state = CIV_SCARED_FLEE;
				c->counter = 0.0f;
			}
			else {
				c->state = CIV_SCARED;
				c->counter = 0.0f;
			}
		}
		break;
	}

	switch (c->state) {
		case CIV_DEAD:
			if( c->counter > 1.0f ) c->counter = 1.0f;
			geActor_SetPose(c->Actor, CivilianDead[c->Look], c->counter, c->XForm );
			break;
		case CIV_IDLE:
			if( c->ai == AI_WAITING || 
				c->ai == AI_FOLLOW ){
				// turn towards player
				enemy_lookAtPos(&(XForm.Translation), c->XForm, &(c->Orientation), time);
			}
			if( c->ai == AI_FOLLOW ) {
				float distance;

				// check player pos vs this pos
				distance = enemy_distanceBetween(c->Position, XForm.Translation, World);

				// if beyond accepted area goto walk
				if( distance > WALKING_DISTANCE_SQ ){
					c->counter = 0.0f;
					c->state = CIV_WALKING_PLAYER;
				}
			}
			if( c->counter > 1.29f ){
				if( c->ai == AI_WALKING ) {
					if( c->time ) {
						c->time --;
						c->counter -= 1.29f;
						if( FIFTY_PERCENTS_CHANCE ) {
							if( FIFTY_PERCENTS_CHANCE ) {
								c->state = CIV_TURN_R;
							}
							else {
								c->state = CIV_TURN_L;
							}
							c->counter = 0.0f;
						}
					}
					else {
						c->state = CIV_WALKING_FROM;
						c->counter = 0.0f;
						c->time = rand() % 5;
					}
				}
				else {
					c->counter -= 1.29f;
				}
			}
			geActor_SetPose(c->Actor, CivilianIdle[c->Look], c->counter, c->XForm );
			break;
		case CIV_WALKING_FROM:
			if( c->counter > 1.0f ) {
				 if( c->time == 0 ) {
					 c->state = CIV_IDLE;
					 c->counter = 0.0f;
					 c->time = rand() % 5;
				 }
				 else {
					 c->counter -= 1.0f;
					 c->time --;
				 }
			}
			else {
				if(! Basic_MoveForward(c->Position, c->Orientation, WALK_SPEED*time, World, c->ExtBox, &(c->Position)) ) {
					// entered wall
					if( FIFTY_PERCENTS_CHANCE ) {
						c->state = CIV_TURN_L;
					}
					else {
						c->state = CIV_TURN_R;
					}
					c->counter = 0.0f;
				}
			}
			geActor_SetPose(c->Actor, CivilianWalk[c->Look], c->counter, c->XForm );
			break;
		case CIV_WALKING_PLAYER:
			if( c->counter > 1.0f ) {
				 c->counter -= 1.0f;
			}

			// turn civilian if neededmove
			enemy_lookAtPos(&(XForm.Translation), c->XForm, &(c->Orientation), 2*time);

			// move
			if( !Basic_MoveForward(c->Position, c->Orientation, WALK_SPEED*time, World, c->ExtBox, &(c->Position)) ){
				// hit the wall, dont bother following
				c->ai = AI_WAITING;
				c->state = CIV_IDLE;
				c->counter = 0.0f;
			}

			
			{	
				// test range of player
				float distance;
				distance = enemy_distanceBetween(c->Position, XForm.Translation, World);
				// change state to run, same or idle
				if( distance > WALKING_DISTANCE_SQ ) {
					if( distance > RUNNING_DISTANCE_SQ ) {
						// running
						c->state = CIV_RUNNING_PLAYER;
						c->counter = 0.0f;
					}
					else {
						// same
					}
				}
				else {
					// walking
					c->state = CIV_IDLE;
				}
			}
			geActor_SetPose(c->Actor, CivilianWalk[c->Look], c->counter, c->XForm );
			break;
		case CIV_RUNNING_PLAYER:
			if( c->counter > 0.96f ) {
				 c->counter -= 0.96f;
			}

			// turn civilian if needed
			enemy_lookAtPos(&(XForm.Translation), c->XForm, &(c->Orientation), 3*time);

			// move
			if( !Basic_MoveForward(c->Position, c->Orientation, RUN_SPEED*time, World, c->ExtBox, &(c->Position)) ){
				// hit the wall, dont bother following
				c->ai = AI_WAITING;
				c->state = CIV_IDLE;
				c->counter = 0.0f;
			}

			
			{
				// test range of player
				float distance;
				distance = enemy_distanceBetween(c->Position, XForm.Translation, World);
				// change state to run, same or idle
				if( distance > WALKING_DISTANCE_SQ ) {
					if( distance > RUNNING_DISTANCE_SQ ) {
						// same
					}
					else {
						// walking
						c->state = CIV_WALKING_PLAYER;
					}
				}
				else {
					// walking
					c->state = CIV_IDLE;
				}
			}
			// change to civilian run
			geActor_SetPose(c->Actor, CivilianRun[c->Look], c->counter, c->XForm );
			break;
		case CIV_SCARED:
			UPDATE_CLOSEST;
			if( c->closest->used ) {
				if( c->closest->distance < FLEE_RANGE ) {
					c->state = CIV_SCARED_FLEE;
					c->counter = 0.0f;
				}
				else {
					// current
				}
			}
			else {
				c->state = CIV_IDLE;
				c->counter = 0.0f;
			}

			if( c->counter > 1.29f ){
				c->counter = 0.0f;
			}
			geActor_SetPose(c->Actor, CivilianScared[c->Look], c->counter, c->XForm );
			break;
		case CIV_SCARED_FLEE:
			UPDATE_CLOSEST;
			if( c->closest->used ) {
				if( c->closest->distance < FLEE_RANGE ) {
					// current
				}
				else {
					c->state = CIV_SCARED;
					c->counter = 0.0f;
				}
			}
			else {
				c->state = CIV_IDLE;
				c->counter = 0.0f;
			}

			if( c->counter > 0.96f ) {
				 c->counter -= 0.96f;
			}

			// turn civilian if needed
			enemy_fleeFromPos(&(XForm.Translation), c->XForm, &(c->Orientation), 3*time);

			// adjust direction
			// move
			Basic_MoveForward(c->Position, c->Orientation, RUN_SPEED*time, World, c->ExtBox, &(c->Position));
			
			geActor_SetPose(c->Actor, CivilianRun[c->Look], c->counter, c->XForm );
			break;
		case CIV_TURN_L:
			if( c->counter > 1.29f ){
				c->state = CIV_IDLE;
				c->counter = 0.0f;
			}
			c->Orientation.Y -= time;
			geActor_SetPose(c->Actor, CivilianIdle[c->Look], c->counter, c->XForm );
			break;
		case CIV_TURN_R:
			if( c->counter > 1.29f ){
				c->state = CIV_IDLE;
				c->counter = 0.0f;
			}
			c->Orientation.Y += time;
			geActor_SetPose(c->Actor, CivilianIdle[c->Look], c->counter, c->XForm );
			break;
		default:
			system_message("Bad state in civilian ai");
			c->state = CIV_IDLE;
		}

		if( c->closest->used && c->Health > 0) {
			// we can se an enemy
			c->afraidTime -= time;
			if( c->afraidTime < 0.0f ) {
				civilian_say_afraid(c);
				civilian_setAfraidTime(c);
			}
		}
}
void enemy_civilian_doDamage(Inf_Enemy_Civilian* c, char type, int damage, geVec3d* damageLocation){
	if( damageLocation ) {
		BloodExplosion(*damageLocation);
	}
	else {
		BloodExplosion(c->Position);
	}

	if( c->Health <= 0 ) return;
	if( type == DAMAGE_NORMAL || DAMAGE_ZOMBIE )
		c->Health -= damage;
	if( c->Health <= 0 ) {
		c->state = CIV_DEAD;
		execute_command( c->cmdKill );
		c->Health = 0;
		civilian_say_death(c);
	}
}

void apply_civilian(geWorld* world){
	geEntity_EntitySet	*Set=0;
	geEntity			*Entity=0;
	
	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Civilian");
	if (Set == NULL) return;
	
	// get first entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_Enemy_Civilian* r;
	
		r = (Inf_Enemy_Civilian *)geEntity_GetUserData(Entity);
		
		civilian_applyAndEvaluateState(r, 0.0f);
		civilian_setAfraidTime(r);

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
}
int handle_civilian_enemy(SaveFile* file, geWorld* world){
	geEntity_EntitySet	*Set=0;
	geEntity			*Entity=0;
	
	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Civilian");
	if (Set == NULL) return 1;
	
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	while( Entity )
	{
		Inf_Enemy_Civilian* r;
	
		r = (Inf_Enemy_Civilian *)geEntity_GetUserData(Entity);
		
		INT( r->state, "Failed to handle civilian state");
		INT( r->Health, "Failed to handle civilian health");
		FLOAT( r->counter, "Failed to handle civilian counter");
		VECTOR( r->Position, "Failed to handle civilian position");
		VECTOR( r->Orientation, "Failed to handle civilian orientation");
		INT( r->closest->used , "Failed to handle civilian monster used");
		VECTORPTR( &(r->closest->position) , "Failed to handle civilian monster position" );
		FLOAT( r->closest->distance, "Failed to handle civilian monster distance");

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}

	return 1;
}
void enemy_civilian_init(){
	//soundsys_loadSound(".\\sfx\\enemies\\zombie\\idle3.wav", 1, &(zombie_idle[3]), 1, TYPE_3D);
	int i;
	for(i=0; i<NUMBER_OF_CIVILIANS; i++) {
		CivilianIdle[i] = geActor_GetMotionByName(enemy_civilian_def[i], "Idle" );
		CivilianWalk[i] = geActor_GetMotionByName(enemy_civilian_def[i], "Walk" );
		CivilianDead[i] = geActor_GetMotionByName(enemy_civilian_def[i], "Die1" );
		CivilianRun[i] = geActor_GetMotionByName(enemy_civilian_def[i], "Run" );
		CivilianScared[i] = geActor_GetMotionByName(enemy_civilian_def[i], "C_Idle" );
	}

	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\saving_0_0.wav", civilian_saving_sounds, 0, 0);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\saving_0_1.wav", civilian_saving_sounds, 0, 1);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\saving_0_2.wav", civilian_saving_sounds, 0, 2);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\saving_0_3.wav", civilian_saving_sounds, 0, 3);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\saving_0_4.wav", civilian_saving_sounds, 0, 4);

	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\leave_0_0.wav", civilian_leave_sounds, 0, 0);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\leave_0_1.wav", civilian_leave_sounds, 0, 1);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\leave_0_2.wav", civilian_leave_sounds, 0, 2);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\leave_0_3.wav", civilian_leave_sounds, 0, 3);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\leave_0_4.wav", civilian_leave_sounds, 0, 4);
	
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\take_0_0.wav", civilian_take_sounds, 0, 0);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\take_0_1.wav", civilian_take_sounds, 0, 1);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\take_0_2.wav", civilian_take_sounds, 0, 2);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\take_0_3.wav", civilian_take_sounds, 0, 3);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\take_0_4.wav", civilian_take_sounds, 0, 4);

	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\hello_0_0.wav", civilian_hello_sounds, 0, 0);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\hello_0_1.wav", civilian_hello_sounds, 0, 1);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\hello_0_2.wav", civilian_hello_sounds, 0, 2);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\hello_0_3.wav", civilian_hello_sounds, 0, 3);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\hello_0_4.wav", civilian_hello_sounds, 0, 4);
	
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\death_0_0.wav", civilian_death_sounds, 0, 0);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\death_0_1.wav", civilian_death_sounds, 0, 1);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\death_0_2.wav", civilian_death_sounds, 0, 2);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\death_0_3.wav", civilian_death_sounds, 0, 3);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\death_0_4.wav", civilian_death_sounds, 0, 4);

	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\afraid_0_0.wav", civilian_afraid_sounds, 0, 0);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\afraid_0_1.wav", civilian_afraid_sounds, 0, 1);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\afraid_0_2.wav", civilian_afraid_sounds, 0, 2);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\afraid_0_3.wav", civilian_afraid_sounds, 0, 3);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\afraid_0_4.wav", civilian_afraid_sounds, 0, 4);
	LOAD_SOUND_FILE(".\\sfx\\enemies\\civilian\\afraid_0_5.wav", civilian_afraid_sounds, 0, 5);

	// @@@todo@@@ temporary till we get the right animations
	i=2;
	CivilianIdle[i] = geActor_GetMotionByName(enemy_civilian_def[i], "recon_idle" );
		CivilianWalk[i] = geActor_GetMotionByName(enemy_civilian_def[i], "recon_walk" );
		CivilianDead[i] = geActor_GetMotionByName(enemy_civilian_def[i], "recon_die" );
		CivilianRun[i] = geActor_GetMotionByName(enemy_civilian_def[i], "recon_walk" );
		CivilianScared[i] = geActor_GetMotionByName(enemy_civilian_def[i], "recon_crouch" );
}
void enemy_civilian_newWorld(geWorld* World){
	geEntity_EntitySet	*Set=0;
	geEntity			*Entity=0;

	// load lua functions
	LOAD_FUNC(civilian_setAI);
	
	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Civilian");
	if (Set == NULL) return;
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);
	while( Entity )
	{
		Inf_Enemy_Civilian* z;
	
		z = (Inf_Enemy_Civilian *)geEntity_GetUserData(Entity);

		z->Actor = 0;
		z->XForm = 0;
		z->ExtBox = 0;
		z->closest = 0;
		z->Health = 10;
		z->state = CIV_IDLE;
		z->time = rand() % 5;
		civilian_setAfraidTime(z);

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}

	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);
	while( Entity )
	{
		Inf_Enemy_Civilian* z;
	
		z = (Inf_Enemy_Civilian *)geEntity_GetUserData(Entity);
		
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

		// debug
		if( 1 || z->Look >= NUMBER_OF_CIVILIANS || z->Look < 0 ) {
			z->Look = rand() % NUMBER_OF_CIVILIANS;
		}

		// udate to choose different actor
		z->Actor = LoadActor(enemy_civilian_def[z->Look], World, 2.0f, GE_ACTOR_RENDER_MIRRORS | GE_ACTOR_RENDER_NORMAL | GE_ACTOR_COLLIDE, z->XForm);
		if( !z->Actor ) {
			run = 0;
			error("Failed to create the civilian actor");
			continue;
		}
		// enable shaddows
		geActor_SetStencilShadow(z->Actor, GE_TRUE);

		geActor_SetUserData(z->Actor, (void*) z);
		setup_box(z->ExtBox, z->Actor, z->Position);

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
}
void enemy_civilian_clearWorld(geWorld* World){
	geEntity_EntitySet	*Set=0;
	geEntity			*Entity=0;
	
	if( !World ) return;
	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Civilian");
	if (Set == NULL) return;
	
	// get first rain entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_Enemy_Civilian* z;
	
		z = (Inf_Enemy_Civilian *)geEntity_GetUserData(Entity);

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
void enemy_civilian_iterate(geWorld* World, float time){
	geEntity_EntitySet	*Set=0;
	geEntity			*Entity=0;
	
	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Civilian");
	if (Set == NULL) return;
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);
	while( Entity )
	{
		Inf_Enemy_Civilian* z;
	
		z = (Inf_Enemy_Civilian *)geEntity_GetUserData(Entity);
		
		civilian_applyAndEvaluateState(z, time);

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
}
geBoolean enemy_civilian_isAlive(geActor* enemy){
	Inf_Enemy_Civilian* z;

	z = (Inf_Enemy_Civilian*) geActor_GetUserData(enemy);
	if (z->Health > 0){
		return GE_TRUE;
	}
	return GE_FALSE;
}
geVec3d* enemy_civilian_getPosition(geActor* act){
	Inf_Enemy_Civilian* z;

	z = (Inf_Enemy_Civilian*) geActor_GetUserData(act);
	if( !z ){
		error("Convertion error in enemy_civilian_getPosition");
		return 0;
	}

	return &(z->Position);
}

char talkto_civilian(geActor* act) {
	Inf_Enemy_Civilian* z;
	geActor_Def* def=0;
	char doit=0;
	int i;

	def = geActor_GetActorDef( act );
	if( !def ) {
		error("Failed to get the definition in talkto_civilian()");
		return 0;
	}

	for(i=0;i<NUMBER_OF_CIVILIANS; i++) {
		if( def == enemy_civilian_def[i] ){
			doit = 1;
			break;
		}
	}
	if(!doit) return 0;

	geActor_GetUserData(act);
	z = (Inf_Enemy_Civilian*) geActor_GetUserData(act);
	if( !z ){
		error("Convertion error in talkto_cvilian");
		return 0;
	}

	if( z->Give ) {
		// are we able to use this?
		if( pickup_handlePickup(z->Give) ) {
			// replace with sfx
			if( z->Health > 0 ) {
				civilian_say_take(z);
			}
			else {
				game_message("You found something useful");
			}
			if( z->numberOfGives!=0 ){
				if( z->numberOfGives > 0 ) {
					z->numberOfGives -= 1;
				}
				// else inifinite
			}
			else {
				z->Give = 0; // we can't use the item again
			}
			return 1; //we have used the item are done
		}
		// if not we continue
	}

	if( z->Health <= 0 ) return 0;

	if( z->ai == AI_FOLLOW ) {
		//z->ai = AI_STANDING;
		z->ai = AI_WAITING;
		civilian_say_leave(z);
	}
	else {
		if( z->ai == AI_WAITING ) {
			// z->ai = AI_WALKING;
			//game_message("Thanks for saving me");
			civilian_say_saving(z);
			z->ai = AI_FOLLOW;
		}
		else {

			if( z->execute_command ) {
				execute_command(z->cmdUse);
				if( !z->infinite_commands ) z->execute_command = GE_FALSE;
			}
			else {
				civilian_say_hello(z);
			}
		}
	}

	return 1;
}

geBoolean enemy_civilian_damage(geActor* enemy,
				  int damage,
				  char type,
				  char lbd,
				  geVec3d fromPos,
				  geVec3d toPos
				  ){
	Inf_Enemy_Civilian* z;
	geExtBox eb;
	float t;
	geVec3d pos;

	z = (Inf_Enemy_Civilian*) geActor_GetUserData(enemy);

	if( lbd){
		geActor_GetDynamicExtBox(z->Actor, &eb);

		if(! geExtBox_RayCollision( &eb, &fromPos, &toPos, 
									&t, NULL ) // don't save the data
									)
		{
			return GE_TRUE;
		}


		geVec3d_Subtract(&toPos, &fromPos, &pos);
		geVec3d_AddScaled(&fromPos, &pos, t, &pos);

		enemy_civilian_doDamage(z, type, damage, &pos);
	}
	else {
		enemy_civilian_doDamage(z, type, damage, 0);
	}

	return GE_FALSE;
}

geBoolean enemy_civilian_loopDamage(int damage, // how many point damage does this weaopon damage do?
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
	
	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Civilian");
	if (Set == NULL) return GE_FALSE;
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);
	while( Entity )
	{
		Inf_Enemy_Civilian* z;
	
		z = (Inf_Enemy_Civilian *)geEntity_GetUserData(Entity);
		geActor_GetDynamicExtBox(z->Actor, &eb);
		if( geExtBox_RayCollision( &eb, &fromPos, &toPos, 
								&t, NULL ) // don't save the data
								) {
			GE_Collision lCol;
			geVec3d loc;
			geVec3d dir;
			geVec3d_Subtract(&toPos, &fromPos, &dir);
			geVec3d_AddScaled(&fromPos, &dir, t, &loc);
			if( !geWorld_Collision(World, NULL, NULL, &fromPos, &loc , GE_CONTENTS_SOLID, GE_COLLIDE_MESHES | GE_COLLIDE_MODELS, 0x00000000, 0 ,NULL, &lCol) ) {
				value = GE_TRUE;
				enemy_civilian_doDamage(z, type, damage, &loc);
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
void enemy_civilian_explosionDamage(geVec3d* location, float range, int damage){
	geEntity_EntitySet	*Set=0;
	geEntity			*Entity=0;
	
	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Civilian");
	if (Set == NULL) return;
	
	// get first rain entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_Enemy_Civilian* z;
	
		z = (Inf_Enemy_Civilian *)geEntity_GetUserData(Entity);
		
		enemy_civilian_doDamage(z, DAMAGE_NORMAL, explosion_getDamage(location, &(z->Position), range, damage), 0);

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
}
void enemy_civilian_fireExplosionDamage(geVec3d* location, float range, int damage){
	geEntity_EntitySet	*Set=0;
	geEntity			*Entity=0;
	
	Set = geWorld_GetEntitySet(World, "Inf_Enemy_Civilian");
	if (Set == NULL) return;
	
	// get first rain entity
	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	// wade thru them all
	while( Entity )
	{
		Inf_Enemy_Civilian* z;
	
		z = (Inf_Enemy_Civilian *)geEntity_GetUserData(Entity);
		
		enemy_civilian_doDamage(z, DAMAGE_FIRE, explosion_getDamage(location, &(z->Position), range, damage), 0);

		// get next entity
		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}
}