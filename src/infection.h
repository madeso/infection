/*
infection.h
standard infection functions that deals with the game
*/

#ifndef INFECTION_H
#define INFECTION_H

#include "genesis.h"
#include "save_file.h"

//tittle and version of the game
#define INFECTION_NAME_AND_VERSION		"Infection v0.2 WIP"

void setPlayerName(char* playerName);
char* getPlayerName();
void new_game();
void save_game(int index);
void load_game(int index);
void quick_save();
void quick_load();
void level_save();
void level_load();
// ok to save at
char test_saveIndex(int index);

geVec3d* findLocationByName(geWorld* world, const char* name);
void enableByName(geWorld* world, const char* name);
void disableByName(geWorld* world, const char* name);
int handle_Vector(SaveFile* file, geVec3d* value);

void add_redFlash(float power);
void add_blueFlash(float power);
void renderFlash();
void iterateFlash();

#endif