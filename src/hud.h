/*
hud.h
takes care of drawing the hud. 
the hud is here classified as the bottom right bitmap with all
the numbers that represent health, armor, magazines and ammo
*/

#include "genesis.h"

#ifndef HUD_H
#define HUD_H

//the real hud
geBitmap *Hud;
geBitmap *nf1[11];
geBitmap *nf2[11];

//load the hud
geBoolean load_hud();

//destroy the hud
void destroy_hud();

//render the hud
geBoolean render_hud();

#endif