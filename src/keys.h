/*
This has hand of all the controls of the infection, and changing theese
*/

#ifndef KEYS_H
#define KEYS_H

//Extra key help
#define INF_KEY_MW_UP		256
#define INF_KEY_MW_DOWN		257

typedef __int16 key;

// if keyvalue is zero, then the key bind is non existing or undefined.

typedef struct controls_tag
{
	key jump;
	key crouch;

	key forward;
	key backward;

	key right;
	key left;

	key holdwalk;

	// Weapon Class
	key wc1;
	key wc2;
	key wc3;
	key wc4;
	key wc5;
	key wc6;

	key next;
	key previous;
	key reload;

	key console;
	key jet;
	key use;
	
	key back;
	key screenshot;

	key toggleFlashlight;

	key quickSave;
	key quickLoad;

	key primaryFire;
	key secondaryFire;
} Controls;

Controls controls;

#endif
