/*
weapons.h
Contains the functionality of the weapons.
*/

#include "genesis.h"
#include "save_file.h"

#ifndef WEAPONS_H
#define WEAPONS_H

//fire current selected weapon
void weapon_fire(geBoolean mbnew);//primary
void weapon_sfire(geBoolean mbnew);//secondary
//void weapon_sfireUp();//secondary up

//init structures and load bitmaps
geBoolean weapon_init();

//reload current selected weapon
void weapon_reload();

//select a weapon number
void weapon_select(unsigned char number);

//update current weapon - should be called evry frame
void weapon_tick();

//unload all loaded icons
void weapon_destroy();

void weapon_newWorld(geWorld* world);

//call this evry sec
void evry_sec();

//render weapon icons
geBoolean weapon_render();

////call this evry sec, detemines if we should draw the selectpos image or not if the user changed a weapon selection
void weapon_evry_sec();

//scrool funcs
void weapon_next();
void weapon_previous();

geBoolean weapon_hasUnlimitedAmmo();
char* weapon_getName();
int weapon_getMagasines();
int weapon_getAmmonution();
geBoolean weapon_ok();

geBoolean weapon_hasBar();

geBoolean weapon_drawBar(int x, int y);

void weapon_bob(char bob);

geBoolean weapon_water(); //returns true if the weapon functions under water

void weapon_strip();
void weapon_give_all();

/*
@@@todo@@@
Make this inyo a smarter way to do theese functions, perhaps an integer specifing the index.
*/

/* Give function */

// the type parameter means
// 0 = weapon with little ammo
// 1 = ammo
// 2 = everything

// hand
void weapon_give_knife();

void weapon_give_glock(int type);
void weapon_give_deagle(int type);

void weapon_give_shotgun(int type);
void weapon_give_sniper(int type);

void weapon_give_smg(int type);
void weapon_give_uzi(int type);

void weapon_give_lft(int type);
void weapon_give_molotov(int type);

void weapon_give_grenade(int type);
void weapon_give_minirocket(int type);

/* Loose function */
void weapon_loose_knife();

void weapon_loose_glock();
void weapon_loose_deagle();

void weapon_loose_shotgun();
void weapon_loose_sniper();

void weapon_loose_smg();
void weapon_loose_uzi();

void weapon_loose_lft();
void weapon_loose_molotov();

void weapon_loose_grenade();
void weapon_loose_minirocket();

void weapon_apply();
void weapon_deapply();

int handle_weapon(SaveFile* file);
#endif