#ifndef PLAYER_H
#define PLAYER_H

#include "genesis.h"

#define EXTBOX_SET(x)	geExtBox_Set(&ExtBox, -40.0f, x, -40.0f, 40.0f, 40.0f, 40.0f)
#define MAX_Y_SPEED						2900.0f
#define MAX_H_SPEED						200.0f


//------------------------------------------------------------------------
// Typedefs
//------------------------------------------------------------------------

typedef struct cheater_tag
{
	geBoolean god;
	geBoolean ghost;
	geBoolean unlimited_ammo;
	geBoolean unlimited_mag;
	geBoolean doPrint; // prints out the name of the current weapon to the user - I classify this as a cheat
	geBoolean materialPrint;
	geBoolean texturePrint;
	geBoolean bouncy;
	geBoolean printFps;
	geBoolean debug;
	geBoolean bbox;
}cheater;

typedef struct options_tag
{
	char enable_effects;
	char enable_decals;
	char log;
	char extended_log;
	geFloat gamma;
	geBoolean clearScreen;
	char fogInWater;
	char startLevel[200];
	float meterfog;
	geBoolean weaponFlash;
	int crosshairNormal;
	int crosshairOver;
	char invertSpeakers;
}Options;


//------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------

// contents defines
#define CONTENTS_WATER		GE_CONTENTS_USER1
#define CONTENTS_LADDER		GE_CONTENTS_USER2
#define CONTENTS_BOUNCY		GE_CONTENTS_USER3
#define CONTENTS_NOMARK		GE_CONTENTS_USER4
#define CONTENTS_DEATH		GE_CONTENTS_USER5

// state defines
#define STATE_NORMAL	0
#define STATE_WATER		1
#define STATE_LADDER	2

//-----------------------------------------------------
// Player variables
//-------------------------------------------------------

unsigned char player_state;// current state
geFloat ySpeed;// ySpeed
unsigned char hero_hit_points;
unsigned char hero_armor_points;
geFloat PLAYER_JUMP_SPEED;
geBoolean player_is_alive;
geFloat air;
geBoolean landed; //true only one round - when the player has landed - second thought do we need this?
geFloat gravity; //the gravity
char moved;//true if player has moved, false otherwise
geBoolean dojump;
geVec3d Angle;//In wich way are we looking?
geVec3d Pos;//Our temporary pos

int currentCrosshair;
cheater cheats; // current cheats
Options options; // current options

//-----------------------------------------------------------
//Function prototypes
//-----------------------------------------------------------

unsigned char player_getState();

void update_player_state();
void player_crouchOrStand(); // note: the player needs to be in a "free" position, ie the player shouldn't have moved without collision detection
void apply_gravity();
void user_interaction();
void set_speed(); // updates the variable speed used when moving

void reset_cursor();
void player_end();
void player_start();
void player_frame();

void player_getPos(geVec3d *pos);// gets the floor position of the player
float player_getPlayerFloor();// floor = the y Value of the players boundingbox bottom

void play_walksound_right();
void play_walksound_left();

geBoolean player_is_on_ground();
void player_is_on_ground_setup();

void enter_water();
void leave_water();

#endif