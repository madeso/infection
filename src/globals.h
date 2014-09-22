/***************************************************
*               globals.h                          *
*      written by Gustav 'sirGustav' Jansson       *
*         for his first 3d game 'Infection'        *
*             started: 2002-07-18                  *
*             ended:                               *
*              mail: sir_gustav@passagen.se        *
*    URL: http://www.geocities.com/sir_gustavs     *
****************************************************/

/*
Some global functions that everybody could use
*/

#ifndef GLOBALS_H
#define GLOBALS_H

#include "genesis.h"
#include "hash_table.h"
//Includes

// move callback function, evry one should use this when moving - we dont want dead bodies to stop us when we are walking
geBoolean cb_move(geWorld_Model* Model, geActor* Actor, void * Context);
void set_fog(float r, float g, float b, float meters, int fog); // creates a fog
void kill_fog(); // kills the fog
geBoolean is_string_null(char *String ); // returns GE_TRUE if the string is considered null, GE_FALSE otherwise

int getMaterial(GE_Collision* lCol);

HashTable* materialStorage;

#define MATERIAL_ERROR				-1

#define MATERIAL_NO_MATERIAL		0
#define MATERIAL_WOOD				1
#define MATERIAL_METAL				2
#define MATERIAL_STONE				3
#define MATERIAL_ELECTRIC			4
#define MATERIAL_GLASS				5
#define MATERIAL_GRASS				6
#define MATERIAL_MARBLE				7
#define MATERIAL_SNOW				8
#define MATERIAL_LEAVES				9
#define MATERIAL_SOFA				10
#define MATERIAL_SMALL_ROCKS		11
#define MATERIAL_SAND				12
#define MATERIAL_MUD				13
#define MATERIAL_GRAVEL				14
#define MATERIAL_PIPE				15

#endif