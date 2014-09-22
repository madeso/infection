#include "genesis.h"
#include "molotov_manager.h"
#include "inf_actor.h"
#include "log.h"
#include "proposeg3d.h"
#include "globals.h"

#define GRAVITY			500.0f

geWorld* molotovCurrentWorld = 0;
int molotovIndex = 0;

void hide_molotov(int i){
	geWorld_SetActorFlags(molotovCurrentWorld, molotovs[i].molotov, 0);
}
void hide_molotov_ex(InfMolotov* molotov){
	geWorld_SetActorFlags(molotovCurrentWorld, molotov->molotov, 0);
}
void show_molotov(int i){
	geWorld_SetActorFlags(molotovCurrentWorld, molotovs[i].molotov, GE_ACTOR_RENDER_MIRRORS | GE_ACTOR_RENDER_NORMAL | GE_ACTOR_COLLIDE);
}
void kill_all_molotovs(){
	int i;
	InfMolotov* m;
	for(i=0; i<NUMBER_OF_MOLOTOVS; i++) {
		m = &molotovs[i];
		m->active=GE_FALSE;
		
		if( m->molotov ){
			if(! molotovCurrentWorld ){
				error("No previous world present when clearing molotovs");
			}
			KillActor(molotovCurrentWorld, &(m->molotov));
			m->molotov = 0;
		}

		geVec3d_Clear(&(m->position));
		geVec3d_Clear(&(m->velocity));
		geXForm3d_SetIdentity(&(m->xform));
	}
	molotovCurrentWorld = 0;
	molotovIndex = 0;
}

void weapon_molotov_destroy(InfMolotov* molotov){
	hide_molotov_ex(molotov);
	molotov->active = GE_FALSE;
	weapon_molotov_bang(molotov->position, MOLOTOV_DAMAGE_RADIUS);
}

char weapon_molotov_init(){
	int i;
	InfMolotov* m;
	for(i=0; i<NUMBER_OF_MOLOTOVS; i++) {
		m = &molotovs[i];
		m->active=GE_FALSE;
		m->molotov=0;
		//m->position
		geVec3d_Clear(&(m->position));
		geVec3d_Clear(&(m->velocity));
		geXForm3d_SetIdentity(&(m->xform));
	}
	molotovCurrentWorld = 0;
	molotovActor = LoadActorDef(".\\actors\\molotov.act");
	if(! molotovActor ) {
		error("Failed to load molotov actor");
		printLog("Failed to load molotov actor\n");
		return 0;
	}
	molotovIndex = 0;
	return 1;
}

void weapon_molotov_proccess(float timePassed){
#define		COLLISION		geWorld_Collision(molotovCurrentWorld,&(m->box.Min),&(m->box.Max),&(m->position),&newPos,GE_CONTENTS_SOLID_CLIP,GE_COLLIDE_ALL,0xffffffff, cb_move, 0, &lCol)
	int i;
	InfMolotov* m;
	geVec3d newPos;
	GE_Collision lCol;

	for( i=0; i<NUMBER_OF_MOLOTOVS; i++){
		m = & molotovs[i];
		if( m->active ){
			m->velocity.Y -= GRAVITY *timePassed;
			geVec3d_AddScaled(&(m->position), &(m->velocity), timePassed, &(newPos) );

			if( COLLISION ) {
				m->position = lCol.Impact;
				weapon_molotov_destroy(m);
				//geVec3d_Scale(&(m->velocity), -0.8f, &(m->velocity) );
				//geVec3d_Reflect(&(m->velocity), &(lCol.Plane.Normal), &(m->velocity), 0.60f);
			}
			else {
				m->position = newPos;
			}

			/*
			if( m->timeLeftToBang <= 0.0f ){
				weapon_molotov_destroy(m);
				return;
			}*/

			// update xform variable
			geXForm3d_SetIdentity(&(m->xform));
			m->xform.Translation = m->position;
			geActor_ClearPose(m->molotov, &(m->xform) );
		}
	}
#undef COLLISION
}

void weapon_molotov_throw(geVec3d velocity, geVec3d position){
	InfMolotov* m = &( molotovs[molotovIndex] );
	if( m->active ){
		error("There is a need to expande the molotov array");
		return;
	}

	m->active = GE_TRUE;
	m->position = position;
	m->velocity = velocity;
	show_molotov(molotovIndex);

	molotovIndex++;
	if( molotovIndex >= NUMBER_OF_MOLOTOVS ) molotovIndex = 0;
}

void weapon_molotov_newWorld(geWorld* world){
	int i;
	InfMolotov* m = 0;
	kill_all_molotovs();
	for(i=0; i<NUMBER_OF_MOLOTOVS; i++){
		m = &molotovs[i];
		m->molotov = LoadActor(molotovActor, world,
		1.0f, 0, &(m->xform) );
		setup_box( &(m->box), m->molotov, m->position );
		m->active = GE_FALSE;
		geActor_SetUserData( m->molotov, m);
	}
	molotovCurrentWorld = world;
}

void weapon_molotov_kill(){
	kill_all_molotovs();
	if( molotovActor ){
		geActor_DefDestroy( &molotovActor );
		molotovActor = 0;
	}
}