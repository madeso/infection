/*
infection.h
standard infection functions that deals with the game
*/

#ifndef INFECTION_H
#define INFECTION_H

#include "genesis.h"

//tittle and version of the game
#define INFECTION_NAME_AND_VERSION		"Infection v0.2 WIP"

void setPlayerName(char* playerName);
void new_game();
void save_game(int index);
void load_game(int index);
void quick_save();
void quick_load();
void level_save();
void level_load();

geVec3d* findLocationByName(geWorld* world, char* name);

void enableByName(geWorld* world, char* name);
void disableByName(geWorld* world, char* name);

#endif