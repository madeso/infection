#include "infection.h"
#include "weapons.h"
#include "player.h"
#include "init.h"
#include "entity_iterate.h"
#include "save_file.h"
#include "log.h"
#include "console.h"
#include "globalGenesis.h"
#include "time_damage.h"
#include "decals.h"
#include "enemies.h"


#define APPLICATION_NAME		"Infection"
#define SAVE_KEY				"TheInfectionGameKey"
#define SAVE_VERSION			3
void setPlayerName(char *playerName);

float redFlashState = 0.0f;
float maxRedFlashState = 0.0f;
float blueFlashState = 0.0f;
float maxBlueFlashState = 0.0f;

void add_redFlash(float power){
	if( power < 0.01f ) return;
	if( maxRedFlashState > 4.0f ) return;
	redFlashState += power;
	maxRedFlashState += power;
}
void add_blueFlash(float power){
	if( power < 0.01f ) return;
	if( maxBlueFlashState > 3.0f ) return;
	blueFlashState += power;
	maxBlueFlashState += power;
}
void renderFlash(){
	GE_RGBA rgba;
	if( redFlashState > 0.0f){
		rgba.r = 255.0f;
		rgba.g = 0.0f;
		rgba.b = 0.0f;
		rgba.a = redFlashState / maxRedFlashState * 255.0f;
		geEngine_FillRect(Engine, &Rect, &rgba);
	}
	if( blueFlashState > 0.0f){
		rgba.r = 0.0f;
		rgba.g = 0.0f;
		rgba.b = 255.0f;
		rgba.a = blueFlashState / maxBlueFlashState * 255.0f;
		geEngine_FillRect(Engine, &Rect, &rgba);
	}
}
void iterateFlash(){
	if( redFlashState > 0.0f ){
		redFlashState -= TIME;
		if( redFlashState <= 0.0f ){
			redFlashState = 0.0f;
			maxRedFlashState = 0.0f;
		}
	}
	if( blueFlashState > 0.0f ){
		blueFlashState -= TIME;
		if( blueFlashState <= 0.0f ){
			blueFlashState = 0.0f;
			maxBlueFlashState = 0.0f;

		}
	}
}
void initFlash(){
	redFlashState= maxRedFlashState = 0.0f;
	blueFlashState= maxBlueFlashState = 0.0f;
}

void new_game()
{
	setPlayerName("player1");
	weapon_strip(); // no weapons on a new game
	hero_hit_points = 100; // full health
	hero_armor_points = 0; // no armor at the beginning
	cleanUpStateBox();
	if(! LoadLevel(options.startLevel, 0) ){
		run = GE_FALSE;
	}
	player_is_alive = GE_TRUE; // we need to be alive when the game starts
	Angle.Z = 0.0f; // no tilting
	// todo free time damage
	timedamage_clear();
}

int makeSureDirectoryExist(char* directory){
	geVFile* Directory = 0;
	Directory = geVFile_OpenNewSystem(NULL, GE_VFILE_TYPE_DOS, directory, NULL, GE_VFILE_OPEN_CREATE|GE_VFILE_OPEN_DIRECTORY); 

	// The directory is null when the directory exists.
	// Let's comment out this piece of code to avoid
	// a mass of stupid error messages.
	/*
	// @@@ why don't we get a directory
	if(! Directory ){
		error("Failed to create player directory");
		return 0;
	}*/
	return 1;
}

char glPlayerName[100];
char playerSavePath[200];
char* getPlayerDirectory(){
	sprintf(playerSavePath, ".\\players\\%s", glPlayerName);
	return playerSavePath;
}

void setPlayerName(char *playerName){
	strcpy(glPlayerName, playerName);
	if(! makeSureDirectoryExist(getPlayerDirectory()) ) return;
	//makeSureDirectoryExist(".\\players");
}

int handle_Vector(SaveFile* file, geVec3d* value){
	FLOAT(value->X, "Failed to handle X");
	FLOAT(value->Y, "Failed to handle Y");
	FLOAT(value->Z, "Failed to handle Z");
	return 1;
}

int handle_SaveFile(SaveFile* file){
	UCHAR(hero_hit_points, "Failed to handle hero_hit_points");
	UCHAR(hero_armor_points, "Failed to handle hero_armor_points");
	INT(player_is_alive, "Failed to handle player_is_alive");
	FLOAT(air, "Failed to handle air");
	VECTOR(Angle, "Failed to handle angle");
	VECTOR(Pos, "Failed to handle Pos");
	FLOAT( ExtBox.Min.Y, "Failed to handle stateBox.Min.Y" );
	if(! handle_Level(file) ) return 0;
	if(! handle_weapon(file) ) return 0;
	if(! handle_timeDamage(file) ) return 0;
	FLOAT(ySpeed, "Failed to handle ySpeed");

	// variable output - depends of the eneties, and most important
	// THE LEVEL LOADED!!!
	if(! handle_entities(file, World ) ) return 0;
	if(! handle_enemy(file, World ) ) return 0;
	return 1;
}

void do_save(char* fileName){
	char temp[200];
	SaveFile* file=0;
	sprintf(temp, "Saving file %s", fileName);
	console_message(temp);

	file = sf_open(fileName, APPLICATION_NAME, SAVE_VERSION, SAVE_KEY, 1);
	if(! file ){
		errorx( "Failed to open file for saving", sf_getLastError() );
		return;
	}
	if(! handle_SaveFile(file) ){
		return;
	}
	if( file ){
		if(! sf_closeFile(file) ){
			errorx( "Failed to close file for saving", sf_getLastError() );
		}
		file = 0;
	}
}
void do_load(char* fileName){
	char temp[200];
	SaveFile* file=0;
	sprintf(temp, "Loading file %s", fileName);
	console_message(temp);

	file = sf_open(fileName, APPLICATION_NAME, SAVE_VERSION, SAVE_KEY, 0);
	if(! file ){
		errorx( "Failed to open file for loading", sf_getLastError() );
		return;
	}
	if(! handle_SaveFile(file) ){
		return;
	}
	if( file ){
		if(! sf_closeFile(file) ){
			errorx( "Failed to close file for loading", sf_getLastError() );
		}
		file = 0;
	}

	DecalMgr_Clear(dMgr);
}

void save_game(int index)
{
	char temp[200];
	sprintf(temp, "%s\\save%i.sav", getPlayerDirectory(), index);
	do_save(temp);
}

void load_game(int index)
{
	char temp[200];
	sprintf(temp, "%s\\save%i.sav", getPlayerDirectory(), index);
	do_load(temp);
}

void quick_save(){
	char temp[200];
	sprintf(temp, "%s\\quick.sav", getPlayerDirectory());
	do_save(temp);
}

void quick_load(){
	char temp[200];
	sprintf(temp, "%s\\quick.sav", getPlayerDirectory());
	do_load(temp);
}

void level_save(){
	/*char temp[200];
	sprintf(temp, "%s\\level.sav", getPlayerDirectory());
	do_save(temp);*/
}

void level_load(){
	/*char temp[200];
	sprintf(temp, "%s\\level.sav", getPlayerDirectory());
	do_load(temp);*/
}

geVec3d* findLocationByName(geWorld* world, char* name){
	return findPositionByName(world, name);
	// this one will call other functions like find enemy positions
	// and find boxes and such things.
}

void enableByName(geWorld* world, char* name){
	entity_enableByName(world, name);
}
void disableByName(geWorld* world, char* name){
	entity_disableByName(world, name);
}