#include "globals.h"
#include "enemies.h"
#include "useful_macros.h"
#include "globalGenesis.h"
#include "xtra_g3d.h"
#include "player.h"


geBoolean cb_move(geWorld_Model* Model, geActor* Actor, void * Context)
{
	// this should be updated to give more scheck
	if( Actor )
	{
		// if the actor is dead we can move through it, if it's alive we can't
		return enemy_isAlive(Actor);
	}

	return GE_TRUE; // stop against walls
}

void set_fog(float r, float g, float b, float meters, int fog)
{
	geFloat distance = METERS( meters ); // get the distance
	
	geCamera_SetFarClipPlane(Camera, GE_TRUE, distance); // screwed up things, maybee I should try it again?
	if( fog )
		geEngine_SetFogEnable(Engine, GE_TRUE, r, g, b, 0, distance + 2.0f); // create the fog
}

void kill_fog()
{
	geEngine_SetFogEnable(Engine, GE_FALSE, 0, 0, 0, 0, 50); // kill the fog
	geCamera_SetFarClipPlane(Camera, GE_FALSE, 50); // Since we're not enabeling it, we don't need to disable it :)
}

// NOTE: start David Wulff
geBoolean is_string_null(char *String )
{

	// first way
	if ( String == NULL )
	{
		return GE_TRUE;
	}

	// second way
	if ( strlen( String ) < 1 )
	{
		return GE_TRUE;
	}

	// third way
	if ( strnicmp( String, "<null>", 6 ) == 0 )
	{
		return GE_TRUE;
	}

	// fourth way
	if ( strnicmp( String, "NULL", 4 ) == 0 )
	{
		return GE_TRUE;
	}

	// if we got to here then the string is not null
	return GE_FALSE;

}
// NOTE: end David Wulff

void printTexture( GE_Collision* lCol ){
	char texture[300];
	geVec3d from;
	geVec3d to;
	char message[600];

	memset(texture, 0, 300);
	geVec3d_AddScaled(&(lCol->Impact), &(lCol->Plane.Normal), 5.0f, &from);
	geVec3d_AddScaled(&(lCol->Impact), &(lCol->Plane.Normal), -5.0f, &to);
	
	if( getSingleTextureNameByTrace(World, &from, &to, texture) ){
		sprintf(message, "The texture is: %s", texture);
		system_message(message);
	}
}

int getMaterial(GE_Collision* lCol){
	char texture[300];
	geVec3d from;
	geVec3d to;

	memset(texture, 0, 300);
	geVec3d_AddScaled(&(lCol->Impact), &(lCol->Plane.Normal), 5.0f, &from);
	geVec3d_AddScaled(&(lCol->Impact), &(lCol->Plane.Normal), -5.0f, &to);
	
	if( getSingleTextureNameByTrace(World, &from, &to, texture) ){
		int data;
		if( cheats.texturePrint )
			system_message(texture);

		return HT_Find(materialStorage, texture, &data);
	}

	system_message("No textures found");
	return MATERIAL_ERROR;
}

/*

//#define GET_MATERIAL(content)	(( content & GE_CONTENTS_USER4 & GE_CONTENTS_USER5 & GE_CONTENTS_USER6 & GE_CONTENTS_USER7 ) >>19 )
//#define GET_MATERIAL(content)	( ( content & GE_CONTENTS_USER4 & GE_CONTENTS_USER5 & GE_CONTENTS_USER6 & GE_CONTENTS_USER7 ) )
#define GET_MATERIAL(content) ((content& 0x780000) >> 19)
//*
int getMaterial(GE_Collision* lCol){
	geVec3d mins;
	geVec3d maxs;
	GE_Contents conts;

	geVec3d_Set(&mins, -1.0f, -1.0f, -1.0f); 
	geVec3d_Set(&maxs, 1.0f, 1.0f, 1.0f);

	printTexture(lCol);

	if( geWorld_GetContents(World, &(lCol->Impact), &mins, &maxs, GE_COLLIDE_ALL, 0xffffffff, NULL, NULL, &conts) ){
		return GET_MATERIAL(conts.Contents);
	}

	return MATERIAL_ERROR;
}
//

#define HALF_TRACE_DISTANCE 3.0f
#define ADD_IF_COLLISION(flags)	if(geWorld_Collision( World, NULL, NULL, \
		&from, &to, GE_CONTENTS_SOLID_CLIP, flags, \
		!flags, NULL, NULL, &Col )){ \
		Result |= flags; }

int getMaterial(GE_Collision* lCol){
	GE_Collision Col;
	int Result = 0;

	geVec3d from, to;

	geVec3d_AddScaled( &(lCol->Impact), &(lCol->Plane.Normal), HALF_TRACE_DISTANCE, &from);
	geVec3d_AddScaled( &(lCol->Impact), &(lCol->Plane.Normal), -1.0f * HALF_TRACE_DISTANCE, &to);	

	

	ADD_IF_COLLISION( 0x00080000 );
	ADD_IF_COLLISION( 0x00100000 );
	ADD_IF_COLLISION( 0x00200000 );
	ADD_IF_COLLISION( 0x00400000 );

	return GET_MATERIAL(Result);
}*/