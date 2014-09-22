/*
timefx.h
Allows changes in time effects.
*/

#ifndef TIMEFX_H
#define TIMEFX_H

#include "genesis.h"

// TimeVariables
geFloat enemyTime;
geFloat heroTime;
geFloat gravityTime;

// fx is a positive floatingpoint greater than 1.0f
void timefx_slowmotion( geFloat fx );
void timefx_drug( geFloat fx );
void timefx_bite( geFloat fx );
void timefx_normal();

#endif