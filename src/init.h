/*
Initializing functions and cleanup functions
*/

#ifndef INIT_H
#define INIT_H

#include <windows.h>
#include "player.h"
#include "save_file.h"
//-------------------------------------------------------
//                  Function prototypes
//-------------------------------------------------------

//setup the engine
geBoolean Init(HWND hWnd);

//load driver information from file
geBoolean FindDriver();

//Load a driver with a given resolution
geBoolean LoadPrefs(char *drv, int *width, int *height, char* fullscreen, Options *options);

//Loads up a level
geBoolean LoadLevel(const char *Filename, int fromLoad);

//Free up all memory, and close file pointer.
void ShutDown();

void exit_application();

geBoolean SavePrefs();

char* getLevelFileName();

int handle_Level(SaveFile* file);

void LeaveLevel();

// implemented in main.c
void timing_reset();
#endif