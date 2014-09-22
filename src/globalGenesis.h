/*
globalGenesis.h

Global genesis variables, and a convinient function that takes a screenshot
*/

#ifndef GLOBAL_GENESIS_H
#define GLOBAL_GENESIS_H

#include <windows.h>
#include "genesis.h"
#include "xfont.h"

geEngine *Engine;//The engine
HWND hWnd;//The window handle
geCamera *Camera;//the camera, where you see from, and the fov
GE_Rect Rect;//Size of the camera
geFloat fov;//your fov
XFontMgr* fntMgr;

int run;//are we still in play
geBoolean keys[258];//temporary variable that will hold keys if they are pressed
//previous 256, changed to 258 to inlude scroller
//reason: easier key-change func later on

//mouse input variables
geBoolean mouse[3];
geBoolean mbnew[3];

//The driver info
geDriver_System *DrvSys;
geDriver *Driver;
geDriver_Mode *Mode;


char ourdriver;//Drivername
int Width;//width in pixels of the app
int Height;//Height in pixels of the app


//The XForm, holds your position and direction in the world
geXForm3d XForm;


//---------------
//Level Variables
//---------------

geWorld *World;//The world
geEntity_EntitySet *IEntitySet;//A EntitySet
geEntity *IEntity;//A Entity

//----------------------------
//Temporary Movement variables
//----------------------------

POINT MousePos;//To know if the mouse is moved

//How fast we wanna move
geFloat speed; // current speed
geFloat real_speed; // the base speed

/** Mouse sensitivity **/
// current sensitivity
geFloat mouse_y_sensitivity;
geFloat mouse_x_sensitivity;

// original sensitivity
geFloat real_mouse_y_sensitivity;
geFloat real_mouse_x_sensitivity;



geVec3d CurrentSpeed;//Temporary Speed

//Helping variables, when moving and strafing
geVec3d in;
geVec3d left;
geVec3d up;

//deltaTime, time variable - used when moving, updated in WinMain
geFloat delta_T;
#define TIME (delta_T)

//collision variables
geExtBox ExtBox;//Bounding box
GE_Collision Col;//A temporary collision variable.

geExtBox stateBox;//state Bounding box
geExtBox minBox;//state Bounding box

// current contents
GE_Contents contents;

float fps; // current fps as in update
float realFps; // fps as in render
char fullscreen; // 1 if fullscreen 0 if not


//current screenshot index
unsigned int shotIndex;
//does a screenshot and aves it to an file
geBoolean screenshot();

GE_RGBA colorDebug;

#endif