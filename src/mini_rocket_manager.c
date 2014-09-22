#include "genesis.h"
#include "mini_rocket_manager.h"
#include "inf_actor.h"
#include "log.h"
#include "proposeg3d.h"
#include "globals.h"
#include "extra_genesis.h"

geWorld* mini_rocketCurrentWorld = 0;
int mini_rocketIndex = 0;

void hide_mini_rocket(int i){
	geWorld_SetActorFlags(mini_rocketCurrentWorld, miniRockets[i].mini_rocket, 0);
}
void hide_mini_rocket_ex(InfMiniRocket* mini_rocket){
	geWorld_SetActorFlags(mini_rocketCurrentWorld, mini_rocket->mini_rocket, 0);
}
void show_mini_rocket(int i){
	geWorld_SetActorFlags(mini_rocketCurrentWorld, miniRockets[i].mini_rocket, GE_ACTOR_RENDER_MIRRORS | GE_ACTOR_RENDER_NORMAL | GE_ACTOR_COLLIDE);
}
void kill_all_miniRockets(){
	int i;
	InfMiniRocket* g;
	for(i=0; i<NUMBER_OF_MINIROCKETS; i++) {
		g = &miniRockets[i];
		g->active=GE_FALSE;
		
		if( g->mini_rocket ){
			if(! mini_rocketCurrentWorld ){
				error("No previous world present when clearing miniRockets");
			}
			KillActor(mini_rocketCurrentWorld, &(g->mini_rocket));
			g->mini_rocket = 0;
		}

		geVec3d_Clear(&(g->position));
		geVec3d_Clear(&(g->velocity));
		geXForm3d_SetIdentity(&(g->xform));
	}
	mini_rocketCurrentWorld = 0;
	mini_rocketIndex = 0;
}

void weapon_mini_rocket_destroy(InfMiniRocket* mini_rocket){
	hide_mini_rocket_ex(mini_rocket);
	mini_rocket->active = GE_FALSE;
	weapon_mini_rocket_bang(mini_rocket->position, MINIROCKET_DAMAGE_RADIUS);
}

char weapon_mini_rocket_init(){
	int i;
	InfMiniRocket* g;
	for(i=0; i<NUMBER_OF_MINIROCKETS; i++) {
		g = &miniRockets[i];
		g->active=GE_FALSE;
		g->mini_rocket=0;
		//g->position
		geVec3d_Clear(&(g->position));
		geVec3d_Clear(&(g->velocity));
		geXForm3d_SetIdentity(&(g->xform));
	}
	mini_rocketCurrentWorld = 0;
	mini_rocketActor = LoadActorDef(".\\actors\\mini_rocket.act");
	if(! mini_rocketActor ) {
		error("Failed to load mini_rocket actor");
		printLog("Failed to load mini_rocket actor\n");
		return 0;
	}
	mini_rocketIndex = 0;
	return 1;
}

void weapon_mini_rocket_proccess(float timePassed){
#define		COLLISION		geWorld_Collision(mini_rocketCurrentWorld,&(g->box.Min),&(g->box.Max),&(g->position),&newPos,GE_CONTENTS_SOLID_CLIP,GE_COLLIDE_ALL,0xffffffff, cb_move, 0, &lCol)
	int i;
	InfMiniRocket* g;
	geVec3d newPos;
	geVec3d to;
	geVec3d angles;
	GE_Collision lCol;

	for( i=0; i<NUMBER_OF_MINIROCKETS; i++){
		g = & miniRockets[i];
		if( g->active ){
			geVec3d_AddScaled(&(g->position), &(g->velocity), timePassed, &(newPos) );

			if( COLLISION ) {
				g->position = lCol.Impact;
				//geVec3d_Scale(&(g->velocity), -0.8f, &(g->velocity) );
				weapon_mini_rocket_destroy(g);
			}
			else {
				g->position = newPos;
			}

			geVec3d_Add(&(g->position), &(g->velocity), &to);
			LookAt(g->position, to, &angles);
			
			angles.X += GE_PI/2;

			// update xform variable
			geXForm3d_SetIdentity(&(g->xform));
			geXForm3d_RotateX(&(g->xform), angles.X);
			geXForm3d_RotateY(&(g->xform), angles.Y);
			geXForm3d_RotateZ(&(g->xform), angles.Z);
			g->xform.Translation = g->position;
			geActor_ClearPose(g->mini_rocket, &(g->xform) );
		}
	}
#undef COLLISION
}

void weapon_mini_rocket_throw(geVec3d velocity, geVec3d position){
	InfMiniRocket* g = &( miniRockets[mini_rocketIndex] );
	if( g->active ){
		error("There is a need to expande the mini_rocket array");
		return;
	}

	g->active = GE_TRUE;
	g->position = position;
	g->velocity = velocity;
	show_mini_rocket(mini_rocketIndex);

	mini_rocketIndex++;
	if( mini_rocketIndex >= NUMBER_OF_MINIROCKETS ) mini_rocketIndex = 0;
}

void weapon_mini_rocket_newWorld(geWorld* world){
	int i;
	InfMiniRocket* g = 0;
	kill_all_miniRockets();
	for(i=0; i<NUMBER_OF_MINIROCKETS; i++){
		g = &miniRockets[i];
		g->mini_rocket = LoadActor(mini_rocketActor, world,
		1.0f, 0, &(g->xform) );
		setup_box( &(g->box), g->mini_rocket, g->position );
		g->active = GE_FALSE;
		geActor_SetUserData( g->mini_rocket, g);
	}
	mini_rocketCurrentWorld = world;
}

void weapon_mini_rocket_kill(){
	kill_all_miniRockets();
	if( mini_rocketActor ){
		geActor_DefDestroy( &mini_rocketActor );
		mini_rocketActor = 0;
	}
}