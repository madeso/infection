/*
crosshair.h
Functions that takes care of loading rendering and deleting the crosshairs.
*/

#ifndef CROSSHAIR_H
#define CROSSHAIR_H

#include "genesis.h"

#define NUM_CROSSHAIRS 5

//load ups all the bitmaps the game uses
geBoolean LoadCrossHair();

//draw all the bitmaps the game uses
geBoolean DrawCrossHair();

//destroy all the bitmaps the game uses
void DestroyCrosshair();

void Crosshair_onLevelLoad();
void Crosshair_onLevelLeave();

#endif