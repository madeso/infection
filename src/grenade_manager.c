#include "genesis.h"
#include "grenade_manager.h"
#include "inf_actor.h"
#include "log.h"
#include "proposeg3d.h"
#include "globals.h"

#define GRAVITY			500.0f
/*
#define NUMMBER_OF_GRENADES					10
#define GRENADE_DAMAGE_RADIUS_SQUARE		10000.0f 

typedef struct _InfGrenade {
	geBoolean active;
	float timeLeftToBang;
	geActor* grenade;
	geVec3d position;
	geVec3d velocity;
	geXForm3d xform; // grenade xform
} InfGrenade;

geActor_Def* grenadeActor;
InfGrenade grenades[NUMBER_OF_GRENADES];

void weapon_grenade_bang(geVec3d position, float radius);
*/

geWorld* grenadeCurrentWorld = 0;
int grenadeIndex = 0;

void hide_grenade(int i){
	geWorld_SetActorFlags(grenadeCurrentWorld, grenades[i].grenade, 0);
}
void hide_grenade_ex(InfGrenade* grenade){
	geWorld_SetActorFlags(grenadeCurrentWorld, grenade->grenade, 0);
}
void show_grenade(int i){
	geWorld_SetActorFlags(grenadeCurrentWorld, grenades[i].grenade, GE_ACTOR_RENDER_NORMAL | GE_ACTOR_COLLIDE);
}
void kill_all_grenades(){
	int i;
	InfGrenade* g;
	for(i=0; i<NUMBER_OF_GRENADES; i++) {
		g = &grenades[i];
		g->active=GE_FALSE;
		
		if( g->grenade ){
			if(! grenadeCurrentWorld ){
				error("No previous world present when clearing grenades");
			}
			KillActor(grenadeCurrentWorld, &(g->grenade));
			g->grenade = 0;
		}

		geVec3d_Clear(&(g->position));
		geVec3d_Clear(&(g->velocity));
		g->timeLeftToBang = 0.0f;
		geXForm3d_SetIdentity(&(g->xform));
	}
	grenadeCurrentWorld = 0;
	grenadeIndex = 0;
}

void weapon_grenade_destroy(InfGrenade* grenade){
	hide_grenade_ex(grenade);
	grenade->active = GE_FALSE;
	weapon_grenade_bang(grenade->position, GRENADE_DAMAGE_RADIUS);
}

char weapon_grenade_init(){
	int i;
	InfGrenade* g;
	for(i=0; i<NUMBER_OF_GRENADES; i++) {
		g = &grenades[i];
		g->active=GE_FALSE;
		g->grenade=0;
		//g->position
		geVec3d_Clear(&(g->position));
		geVec3d_Clear(&(g->velocity));
		g->timeLeftToBang = 0.0f;
		geXForm3d_SetIdentity(&(g->xform));
	}
	grenadeCurrentWorld = 0;
	grenadeActor = LoadActorDef(".\\actors\\grenade.act");
	if(! grenadeActor ) {
		error("Failed to load grenade actor");
		printLog("Failed to load grenade actor\n");
		return 0;
	}
	grenadeIndex = 0;
	return 1;
}

void weapon_grenade_proccess(float timePassed){
#define		COLLISION		geWorld_Collision(grenadeCurrentWorld,&(g->box.Min),&(g->box.Max),&(g->position),&newPos,GE_CONTENTS_SOLID_CLIP,GE_COLLIDE_ALL,0xffffffff, cb_move ,1, &lCol)
	int i;
	InfGrenade* g;
	geVec3d newPos;
	GE_Collision lCol;

	for( i=0; i<NUMBER_OF_GRENADES; i++){
		g = & grenades[i];
		if( g->active ){
			g->velocity.Y -= GRAVITY *timePassed;
			geVec3d_AddScaled(&(g->position), &(g->velocity), timePassed, &(newPos) );

			if( COLLISION ) {
				g->position = lCol.Impact;
				//geVec3d_Scale(&(g->velocity), -0.8f, &(g->velocity) );
				geVec3d_Reflect(&(g->velocity), &(lCol.Plane.Normal), &(g->velocity), 0.60f);
			}
			else {
				g->position = newPos;
			}

			g->timeLeftToBang -= timePassed;
			if( g->timeLeftToBang <= 0.0f ){
				weapon_grenade_destroy(g);
				return;
			}

			// update xform variable
			geXForm3d_SetIdentity(&(g->xform));
			g->xform.Translation = g->position;
			geActor_ClearPose(g->grenade, &(g->xform) );
		}
	}
#undef COLLISION
}

void weapon_grenade_throw(geVec3d velocity, geVec3d position, float time){
	InfGrenade* g = &( grenades[grenadeIndex] );
	if( g->active ){
		error("There is a need to expande the grenade array");
		return;
	}

	g->active = GE_TRUE;
	g->position = position;
	g->velocity = velocity;
	g->timeLeftToBang = time;
	show_grenade(grenadeIndex);

	grenadeIndex++;
	if( grenadeIndex >= NUMBER_OF_GRENADES ) grenadeIndex = 0;
}

void weapon_grenade_newWorld(geWorld* world){
	int i;
	InfGrenade* g = 0;
	kill_all_grenades();
	for(i=0; i<NUMBER_OF_GRENADES; i++){
		g = &grenades[i];
		g->grenade = LoadActor(grenadeActor, world,
		1.0f, 0, &(g->xform) );
		setup_box( &(g->box), g->grenade, g->position );
		g->active = GE_FALSE;
		geActor_SetUserData( g->grenade, g);
	}
	grenadeCurrentWorld = world;
}

void weapon_grenade_kill(){
	kill_all_grenades();
	if( grenadeActor ){
		geActor_DefDestroy( &grenadeActor );
		grenadeActor = 0;
	}
}