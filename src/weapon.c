#include "weapons.h"
#include "enemies.h"
#include "useful_macros.h"
#include "globalGenesis.h"
#include "sound_system.h"
#include "timefx.h"
#include "log.h"
#include "extra_genesis.h"
#include "decals.h"
#include "time_damage.h"
#include "globals.h"
#include "fxbitmaps.h"
#include "grenade_manager.h"
#include "proposeg3d.h"
#include "explosions.h"
#include "infection.h"
#include "NearestActorContainer.h"


// for how many seconds to flash blue screen when picking-up a new weapon
#define PICKUP_FLASH 0.1f

#define SET_STATE(w, s, t) w->state = s; w->stateTime = t

#define BULLET_TRACE_DISTANCE 100000.0f
#define FIGHT_TRACE_DISTANCE  150.0f
#define FIGHT_ACC			  0.0f
geLight* weaponLight=0;
GE_RGBA weaponRgb;
geVec3d Offset;
geBoolean ApplyHLOffset;
GE_Collision wCol;
geVec3d wIn;

// macros for ease the displayeing of lightning effects
#define MAKE_LIGHT()		if(!weaponLight)	weaponLight = geWorld_AddLight(World)
#define DESTROY_LIGHT()		if( weaponLight)	{ geWorld_RemoveLight(World, weaponLight); weaponLight=0;}
#define SET_POS(v)			if( weaponLight)	geWorld_SetLightAttributes( World, weaponLight, &v, &weaponRgb, 650.0f, GE_TRUE)

// old includes, do we need to include theese when we mixed around the headers?
#include "genesis.h"
#include "inf_message_system.h"
#include "hud.h"
#include "player.h" // bullet trace function ?
#include "keys.h"	// for the bullet trace function - to make the accuracy variable
					//dependent on user interaction

#define NUMBER_OF_WEAPONS				4
#define NUMBER_OF_C						6
#define TOTAL_NUMBER_OF_WEAPONS			24

#define WEAPON_C_COMBATS				0
#define WEAPON_C_PISTOLS				1
#define WEAPON_C_RIFLES					2
#define WEAPON_C_AUTOMATICS				3
#define WEAPON_C_FLAMES					4
#define WEAPON_C_EXPLOSIVES				5

#define WEAPON_OPEN_SEC					4

// head for internal function
void disable_selector();

typedef struct	weapon_tag		weapon;
typedef struct	weaponType_tag	WeaponType;

typedef struct weaponType_tag{
	char name[45]; // temporary?
	unsigned int max_ammo;
	unsigned int mag;
	geBoolean hold_fire;
	geBoolean showBar;
	geBoolean unlimited_ammo;
	geBitmap *s_icon; // "selected" icon
	geBitmap *u_icon; // "not selected" icon
	geBitmap *n_icon; // "not have" icon
	soundsys_sound weaponSound[6]; // the weapon sounds for this weapon
	geBoolean inWater; // does this weapon function in water
	float mod_acc;
	char damageType;
	int damage;
	geBoolean ap;

	void (*fn_processState) (weapon* w,geFloat f);//call this evry frame the weapon is avaible
	void (*fn_fireWeapon)(weapon* w, geBoolean mbnew); // called when firing the weapon
	void (*fn_reloadWeapon)(weapon* w); // called when the user wants to reload the weapon
	void (*fn_fxMotion)(weapon* w); // set's the motion to a motion that indicates that the player has been resting to long
	void (*fn_applyWeapon)(weapon* w); // called when the player applys the weapon
	void (*fn_noAmmo)(weapon* w, geBoolean mbnew); // called when the weapon has no ammo
	void (*fn_secondFire)(weapon* w, geBoolean mbnew); // calls when the player fires the secondary fire
	void (*fn_deApply)(weapon* w); // calls when the player deselects this weapon for some reason
	geBoolean (*fn_changeable)(weapon* w); // can we change the weapon right now?
	float (*fn_renderLine)(weapon* w); // returns 0.0f if the weapons doesn't need to draw a progressbar, returns 0.0f to 1.0f to indicate progress
} WeaponType;


// Single weapon definition

typedef struct weapon_tag
{
	unsigned char ammonution; // current ammount of ammonution
	unsigned char magasines; // current ammount of magazines
	unsigned char number_of_weapons; // the number of weapons the player has for this weapon type.
	
	// FSM states
	unsigned char state; // current state
	float stateTime; // time to the next state
	int value; // used for zooming or different powers or types of the weapon etc.

	WeaponType* theWeapon;
} weapon;

WeaponType weaponTypes[TOTAL_NUMBER_OF_WEAPONS];
weapon weapons[NUMBER_OF_C][NUMBER_OF_WEAPONS];
weapon *current_weapon;

unsigned char currClass; // the current class
unsigned char currWeapon; // the current weapon

unsigned char selClass;
unsigned char selWeapon;

unsigned char open_sec; // how many seconds should the select-bar be opened?
geBitmap *select_background; // the sellect bar
geBitmap *slct; // the selector, indicates that you can select this weapon
geBitmap *forbidden; // the selector, indicates that you can't select this weapon

geBitmap *classNumber[6];
geBitmap *spot;

// if showBar == GE_TRUE
geBitmap *bar; // the bar - used when rendering flamethrower
geBitmap *bar_bkg; // the bar background

geBitmap *line; // the probressbar line at the bottom of the screen
geBitmap *line_bkg; // the progress bar line background

soundsys_sound ric[2];

soundsys_sound explosion_sound;

/********************************************************************************************************/

int handle_weapon(SaveFile* file){
	int iclass;
	int iweapon;
	weapon* w;

	for(iclass=0; iclass < NUMBER_OF_C; iclass++){
		for( iweapon=0; iweapon< NUMBER_OF_WEAPONS; iweapon++){
			w = &(weapons[iclass][iweapon]);
			UCHAR(w->ammonution, "Failed to handle weapon ammo");
			UCHAR(w->magasines, "Failed to handle magasines");
			UCHAR(w->number_of_weapons, "Failed to handle the number of weapons");
		}
	}

	UCHAR(currClass, "Failed to handle current class");
	UCHAR(currWeapon, "Failed to handle current weapon");
	UCHAR(selClass, "Failed to handle selection class");
	UCHAR(selWeapon, "Failed to handle selection weapon");
	UCHAR(open_sec, "Failed to handle open_sec");

	// update weapon
	current_weapon = & (weapons[currClass][currWeapon] );

	UCHAR(current_weapon->state, "Failed to handle state");
	FLOAT(current_weapon->stateTime, "Failed to handle state-time");
	// @@@todo@@@ hande saving the value

	return 1;
}

typedef struct weaponDamage
{
	int damage;
	char type; // type of damage
	geVec3d from; // the shooter pos
	geVec3d to; // estimated hit position
	geBoolean ap; //@@@todo@@@ armor piercing, does this work ???
} weaponDamage;

geBoolean cb_weapon(geWorld_Model* Model, geActor* Actor, void * Context)
{
	// this should be updated to give more check
	if( Actor )
	{
		weaponDamage *d=0;
		//geBoolean r;
		
		d = (weaponDamage*) Context;
		if(!d )	{
			system_message("cb_weapon : The damage is not passed through the Context");
			return GE_FALSE;
		}

		// if it is a enemy?
		if( enemy_canDamage(Actor, d->from, d->to) ){
			nac_addActor(Actor, enemy_getPosition(Actor), &(d->from) );
		}
		else {
			// consider it a grenade
			geActor_Def* def=0;
			void* data = geActor_GetUserData(Actor);
			// get the definition
			def = geActor_GetActorDef( Actor );

			if( def == grenadeActor && data != 0) {
				InfGrenade* gr = (InfGrenade*)(data);
				if( gr ) {
					if( geActor_Collision(gr->grenade, &(d->from), &(d->to)) ){
						weapon_grenade_destroy(gr);
					}
				}
			}
		}
	}

	// keep the check going
	return GE_TRUE;
}


void weapon_flash()
{
	if( !options.weaponFlash ) return; // if weaponFlash isn't wanted, then don't turn it on
	
	// set the color
	weaponRgb.r = 200.0f;
	weaponRgb.g = 200.0f;
	weaponRgb.b = 20.0f;
	weaponRgb.a = 150.0f;
	
	MAKE_LIGHT(); // create the light
	SET_POS(XForm.Translation); // set the position

	/* Reset to flame light */
	weaponRgb.r = 200.0f;
	weaponRgb.g = 20.0f;
	weaponRgb.b = 20.0f;
	weaponRgb.a = 200.0f;
}

geBoolean weapon_hasUnlimitedAmmo()
{
	return current_weapon->theWeapon->unlimited_ammo;
}

char* weapon_getName()
{
	return current_weapon->theWeapon->name;
}

int weapon_getMagasines()
{
	return current_weapon->magasines;
}

int weapon_getAmmonution()
{
	return current_weapon->ammonution;
}

geBoolean weapon_ok()
{
	return current_weapon != 0;
}

char sniper_enabled;
char sniper_on()
{
	return sniper_enabled;
}

void enable_sniper(float fov)
{
	//if( !sniper_on() )
	{
		geCamera_SetAttributes(Camera, fov, &Rect);
		sniper_enabled = 1;
		
		// change sensitivity
		mouse_x_sensitivity = real_mouse_x_sensitivity * fov;
		mouse_y_sensitivity = real_mouse_y_sensitivity * fov;
	}
}

void disable_sniper()
{
	if( sniper_on() )
	{
		geCamera_SetAttributes(Camera, 2.0f, &Rect);
		sniper_enabled = 0;

		// change sensitivity
		mouse_x_sensitivity = real_mouse_x_sensitivity;
		mouse_y_sensitivity = real_mouse_y_sensitivity;

		current_weapon->value = 0;
	}
}

/*
geBitmap*	stoneSmoke,
			snowSmoke,
			electricSmoke,
			pipeSmoke,
			gravelSmoke,
			mudSmoke;
*/

#define RICHOCHET	if( sfx ) soundsys_play3dsound(&(ric[rand()%2]), &lCol->Impact, 2.0f, GE_TRUE)

void bullet_hitObstacle_ex(GE_Collision* lCol, int decalType, char gfx, char sfx, char mark){
	if(lCol->Model){
		int materialId = getMaterial(lCol);
		switch(materialId){
		case MATERIAL_NO_MATERIAL:
			if(cheats.materialPrint) game_message("No material"); // Example: sky...
			return;
		case MATERIAL_WOOD:
			if(cheats.materialPrint) game_message("Material: Wood");
			if( gfx )
				ParticleExplosion(woodSprite, lCol->Impact, lCol->Plane.Normal);
			RICHOCHET;
			break;
		case MATERIAL_METAL:
			if(cheats.materialPrint) game_message("Material: Metal");
			// add big sparkles (as sprites?)
			// add glow (as a aparticle)
			if( gfx )
			{
				geVec3d pos;
				geVec3d_AddScaled(&(lCol->Impact), &(lCol->Plane.Normal), 8.0f, &pos);
				createSimpleSparkle(pos);
			}
			RICHOCHET;
			break;
		case MATERIAL_STONE:
			if(cheats.materialPrint) game_message("Material: Stone");
			if(gfx) createSmokeEmitter(stoneSmoke, lCol->Impact, lCol->Plane.Normal, SES_NORMAL);
			// add small sparkles (as sprites?)
			RICHOCHET;
			break;
		case MATERIAL_ELECTRIC:
			if(cheats.materialPrint) game_message("Material: Electric");
			//if( gfx )
			{
				geVec3d pos;
				geVec3d_AddScaled(&(lCol->Impact), &(lCol->Plane.Normal), 8.0f, &pos);
				createExplosionSmokeEmitter(electricSmoke, pos, 0.1f);
				createElectricSparkle(pos);
			}
			RICHOCHET;
			break;
		case MATERIAL_GLASS:
			if(cheats.materialPrint) game_message("Material: Glass");
			// change decalType(based on input decalType??)
			if( gfx )
			ParticleExplosion(glassSprite, lCol->Impact, lCol->Plane.Normal);
			break;
		case MATERIAL_GRASS:
			if(cheats.materialPrint) game_message("Material: Grass");
			if( gfx )
			ParticleExplosion(grassSprite, lCol->Impact, lCol->Plane.Normal);
			return; // no bullet hole
		case MATERIAL_MARBLE:
			if(cheats.materialPrint) game_message("Material: Marble");
			// change to a marbe bullet hole decal? Max Payne2 floor effect looking??
			if( gfx )
			ParticleExplosion(marbleSprite, lCol->Impact, lCol->Plane.Normal);
			RICHOCHET;
			break;
		case MATERIAL_SNOW:
			if(cheats.materialPrint) game_message("Material: Snow");
			if( gfx ) createSmokeEmitter(snowSmoke, lCol->Impact, lCol->Plane.Normal, SES_LARGE);
			break;
		case MATERIAL_LEAVES:
			if(cheats.materialPrint) game_message("Material: Leaves");
			if( gfx ) ParticleExplosion(leafSprite, lCol->Impact, lCol->Plane.Normal);
			return;// no bullet hole
		case MATERIAL_SOFA:
			if(cheats.materialPrint) game_message("Material: Sofa");
			if( gfx ) ParticleExplosion(sofaSprite, lCol->Impact, lCol->Plane.Normal);
			// spec. decal effect - 3d ???
			break;
		case MATERIAL_SMALL_ROCKS:
			if(cheats.materialPrint) game_message("Material: Small rocks");
			if(gfx) ParticleExplosion(rockSprite, lCol->Impact, lCol->Plane.Normal);
			RICHOCHET;
			return; // no bullet hole
		case MATERIAL_SAND:
			if(cheats.materialPrint) game_message("Material: Sand");
			if(gfx) ParticleExplosion(sandSprite, lCol->Impact, lCol->Plane.Normal);
			return;
		case MATERIAL_MUD:
			if(cheats.materialPrint) game_message("Material: Mud");
			if(gfx) createSmokeEmitter(mudSmoke, lCol->Impact, lCol->Plane.Normal, SES_NORMAL);
			return;
		case MATERIAL_GRAVEL:
			if(cheats.materialPrint) game_message("Material: Gravel");
			if(gfx) createSmokeEmitter(gravelSmoke, lCol->Impact, lCol->Plane.Normal, SES_NORMAL);
			return;
		case MATERIAL_PIPE:
			if(cheats.materialPrint) game_message("Material: Pipe");
			if(gfx) createSmokeEmitter(pipeSmoke, lCol->Impact, lCol->Plane.Normal, SES_LONG_AND_LARGE);
			RICHOCHET;
			break;
			// bullethole???
		case MATERIAL_ERROR:
			system_message("Material: Error");break;
		default:
			{
				char str[400];
				sprintf(str, "Bad! Default case in materialId switch, %i", materialId);
				system_message(str);
				break;
			}
		}
	}

	if(lCol->Mesh){
		game_message("bullet hitObstacle found a Mesh");
	}
	if(lCol->Actor){
		//game_message("bullet hitObstacle found an Actor");
		return;
	}

	{
		geVec3d mins;
		geVec3d maxs;
		GE_Contents conts;

		geVec3d_Set(&mins, -1.0f, -1.0f, -1.0f); 
		geVec3d_Set(&maxs, 1.0f, 1.0f, 1.0f);

		if( geWorld_GetContents(World, &(lCol->Impact), &mins, &maxs, GE_COLLIDE_ALL, 0xffffffff, NULL, NULL, &conts) ){
			if( conts.Contents & CONTENTS_NOMARK ){
				return;
			}
		}
	}

	if( mark )
	if(	!DecalMgr_AddDecal( dMgr, DecalMgr_GetRandomDecal(decalType),
							-1.0, RGBA_Array, 1.0f, &(lCol->Impact),
							&(lCol->Plane.Normal), &wIn)
		)
		system_message("Failed to add decal");
}

void bullet_hitObstacle(GE_Collision* lCol, int decalType){
	bullet_hitObstacle_ex(lCol, decalType, 1, 1, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		player_fire_gun(internal)
//			does the bullet fire function
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

geBoolean bullet_trace(geVec3d *from, geVec3d *to, GE_Collision *lCol, int dmg, char type, geBoolean ap){
	weaponDamage damage;

	damage.ap = ap; // armor piercing
	damage.damage = dmg; //the damage
	damage.from = *from; // shooter
	damage.to = *to; // estimated hit position
	damage.type = type; //damage type
	lCol->Impact = *to;

	nac_clear();

	if( geWorld_Collision(
		World,
		NULL, //raytrace
		NULL, //raytrace
		from,
		to,
		GE_CONTENTS_SOLID_CLIP,
		GE_COLLIDE_ALL,
		0xffffffff,
		cb_weapon,
		&damage,
		lCol)
		)
	{
		NacNa* na = nac_findAndRemoveClosest();

		if( na ) {
			if( damage.ap ) {
				int dmg = damage.damage;

				while( na && dmg > 0) {
					enemy_damage(na->actor, dmg, damage.type, damage.from, damage.to);
					dmg -= 5;
					na = nac_findAndRemoveClosest();
				}
			} else {
				enemy_damage(na->actor, damage.damage, damage.type, damage.from, damage.to);
			}
			return GE_FALSE;
		}

		return GE_TRUE;
	}
	else
	{
		return GE_FALSE;
	}
}

geBoolean player_fire_gun(geFloat distance, geFloat theAccuracy, geVec3d *In, GE_Collision *lCol, WeaponType* wt)
{
	geVec3d Result; //the result - the maximum point you can shoot to with the current weapon
	geVec3d displace;
	weaponDamage damage;
	geFloat accuracy = 0.0f;

	accuracy = theAccuracy;
	// change accuracy to depend on alot of variables
	
	if( timedamage_has(TIMEDAMAGE_ZOMBIEBITE) )
		accuracy += 3.0f;
	if( timedamage_has(TIMEDAMAGE_INFECTEDBITE) )
		accuracy += 2.0f;

	if( moved )
	{
		if( keys[ controls.holdwalk ] )
		{
			accuracy += 1.0f; // walking 6.0f 
		}
		else
		{
			accuracy += 2.0f; // running 12.0f
		}
	}

	if( keys[controls.crouch] )
		accuracy -= 2.0f;

	// we assume the player can't shoot from water
	if( player_getState() == STATE_LADDER ){
		accuracy += 1.0f;
	}

	if( timedamage_has(TIMEDAMAGE_DRUG) )
		accuracy -= 2.0f;

	if( accuracy < 0.0f ) accuracy = 0.0f;
	if( accuracy > 40.0f ) accuracy = 40.0f;

	accuracy *= wt->mod_acc;


	// basic fast hack, but does the work
	// if I would do this the right way I probably would end
	// up using some trigonomtry - and prefer not to use that
	displace.X = RAND_INVERT(RAND_FLOAT());
	displace.Y = RAND_INVERT(RAND_FLOAT());
	displace.Z = RAND_INVERT(RAND_FLOAT());

	geXForm3d_GetIn(&XForm, In);
	geVec3d_AddScaled(&XForm.Translation, In, distance, &Result);
	geVec3d_AddScaled(&Result, &displace, accuracy, &Result);

	damage.ap = GE_FALSE; // armor piercing
	damage.damage = 10; //the damage
	damage.from = XForm.Translation; // shooter
	damage.to = Result; // estimated hit position
	damage.type = DAMAGE_NORMAL; //damage type

	lCol->Impact = Result;

	return bullet_trace(&(XForm.Translation),&Result,lCol, wt->damage, wt->damageType, wt->ap);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
















/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		internal weapon specific functions
//			notations:
//				p: process state function
//				f: fire weapon function
//				r: reload weapon function
//				x: fx motion function
//				a: apply weapon function
//				n: no ammo function
//				s: second fire function
//				y: de-apply function
//				c: changeable - can we change weapon now?
//				l: should we render a line/progressbar?
/////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////
// class: Close combat
///////////////////////////////

//---------------------------
// weapon #1: hand to hand
void fn_p_hand (weapon* w,geFloat t)
{
	w->stateTime += t * heroTime/enemyTime;

	switch(w->state)
	{
	case 0: // apply weapon
		if( w->stateTime >0.05f )
		{
			SET_STATE(w, 9, 0.0f);
			game_message("It's time for some close combat");
		}
		break;
	case 1: // right hand action
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 2: // left hand action
		if( w->stateTime > 0.3f)
		{
			SET_STATE( w, 9, 0.0f);
		}
		break;
	case 3: // powershot loading
		if( !(GetAsyncKeyState(controls.secondaryFire) & 0x8000) || w->stateTime > 1.5f )
		{
			if( GetAsyncKeyState(controls.secondaryFire) & 0x8000 )
			{
				game_message("The last thing you'll remeber will be my fist");
				SET_STATE(w, 4, 0.0f);
			}
			else
			{
				game_message("Powerload failed");
				SET_STATE(w, 9, 0.0f);
			}
		}
		break;
	case 4: // power shot ready
		if( !(GetAsyncKeyState(controls.secondaryFire) & 0x8000) || w->stateTime > 5.0f)
		{
			if(GetAsyncKeyState(controls.secondaryFire) & 0x8000)
			{
				game_message( "Time limmit exceeded" );
				SET_STATE(w, 0, 0.0f);
			}
			else
			{
				SET_STATE(w, 5, 0.0f);
				game_message("AAAAAAAAAAAAAAAAAAAAA");

				if( player_fire_gun(FIGHT_TRACE_DISTANCE, FIGHT_ACC, &wIn, &wCol, w->theWeapon) )
				{
					bullet_hitObstacle_ex(&wCol, DECALTYPE_BULLETBIG, 0, 0, 1);
				}
				if( player_fire_gun(FIGHT_TRACE_DISTANCE, FIGHT_ACC, &wIn, &wCol, w->theWeapon) )
				{
					bullet_hitObstacle_ex(&wCol, DECALTYPE_BULLETBIG, 0, 0, 1);
				}
			}
		}
		break;
	case 5: // powershot action
		if( w->stateTime > 1.0f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 6: // fx1
		if( w->stateTime > 1.0f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 7: // fx 2
		if( w->stateTime > 1.0f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 8: // fx3
		if( w->stateTime > 1.0f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 9: // idle
		if( w->stateTime > 5.0f)
		{
			w->stateTime = 0.0f;
		}
		break;
	default:
		printLog("Error in process state for hand\n");
		break;
	}
}
void fn_f_hand (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	{
		if( w->state==9||w->state==6||w->state==7||w->state==8)
		{
			if( rand()%2 )
			{
				w->state=1;
				soundsys_play_sound( &(w->theWeapon->weaponSound[1]) , GE_FALSE);
			}
			else
			{
				soundsys_play_sound( &(w->theWeapon->weaponSound[2]) , GE_FALSE);
				w->state = 2;
			}
			w->stateTime = 0.0f;

			if( player_fire_gun(FIGHT_TRACE_DISTANCE, FIGHT_ACC, &wIn, &wCol, w->theWeapon) )
			{
				bullet_hitObstacle_ex(&wCol, DECALTYPE_BULLETBIG, 0, 0, 1);
			}
		}
	}
}
void fn_r_hand (weapon* w)
{
	system_message("Error in application: Shouldn't be able to reload hand");
}
void fn_x_hand (weapon* w)
{
	if( w->state == 9 )
	{
		switch( rand()%3 )
		{
		case 0:
			SET_STATE(w, 6, 0.0f);
			break;
		case 1:
			SET_STATE(w, 7, 0.0f);
			break;
		case 2:
			SET_STATE(w, 8, 0.0f);
			break;
		}
	}
}
void fn_a_hand (weapon* w)
{
	w->state = 0;
	w->stateTime = 0.0f;
}
void fn_n_hand (weapon* w, geBoolean mbnew)
{
	system_message("Error in applciation, can't run out of ammo for hand");
}
void fn_s_hand (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	if( w->state==9||w->state==6||w->state==7||w->state==8)
	{
		SET_STATE(w, 3, 0.0f);
		game_message("Powershot loading");
	}
}
void fn_y_hand (weapon* w )
{
}
geBoolean fn_c_hand (weapon* w)
{
	return ( w->state==9||w->state==6||w->state==7||w->state==8);
}
float fn_l_hand (weapon* w)
{
	if( w->state == 3 )
	{
		return ( w->stateTime / 1.5f );
	}
	return 0.0f;
}
//----------------------------


//---------------------------
// weapon #2: Ka-Bar Fighting knife
void fn_p_knife (weapon* w,geFloat t)
{
	w->stateTime += t * heroTime/enemyTime;

	switch(w->state)
	{
	case 0:
		if( w->stateTime > 0.5f)
		{
			SET_STATE(w, 9, 0.0f);
			game_message("My knife is cool");
		}
		break;
	case 1:
		if( w->stateTime > 0.2f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 2:
		if( w->stateTime > 0.2f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 3:
		if( w->stateTime > 0.2f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 4:
		if( w->stateTime > 0.2f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 5:
		if( w->stateTime > 1.5f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 6:
		if( w->stateTime > 1.5f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 9:
		if( w->stateTime > 10.0f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	default:
		printLog("Error in process state for knife\n");
		break;
	}
}
void fn_f_knife (weapon* w, geBoolean mbnew)
{
	if( w->state == 9 || w->state == 5|| w->state == 6)
	{
		switch(rand()%4 )
		{
		case 0:
			SET_STATE(w, 1, 0.0f);
			soundsys_play_sound( &(w->theWeapon->weaponSound[1]) , GE_FALSE);
			break;
		case 1:
			SET_STATE(w, 2, 0.0f);
			soundsys_play_sound( &(w->theWeapon->weaponSound[2]) , GE_FALSE);
			break;
		case 2:
			SET_STATE(w, 3, 0.0f);
			soundsys_play_sound( &(w->theWeapon->weaponSound[3]) , GE_FALSE);
			break;
		case 3:
			SET_STATE(w, 4, 0.0f);
			soundsys_play_sound( &(w->theWeapon->weaponSound[4]) , GE_FALSE);
			break;
		}
		if( player_fire_gun(FIGHT_TRACE_DISTANCE, FIGHT_ACC, &wIn, &wCol, w->theWeapon) )
		{
			bullet_hitObstacle_ex(&wCol, DECALTYPE_BULLETBIG, 0, 0, 1);
		}
	}
}
void fn_r_knife (weapon* w)
{
	system_message("Error in reload state for knife, can't reload knife");
}
void fn_x_knife (weapon* w)
{
	if( w->state == 9 )
	{
		if( rand()%2 )
		{
			SET_STATE(w, 5, 0.0f);
		}
		else
		{
			SET_STATE(w, 6, 0.0f);
		}
	}
}
void fn_a_knife (weapon* w)
{
	w->state = 0;
	w->stateTime = 0.0f;
}
void fn_n_knife (weapon* w, geBoolean mbnew)
{
	system_message("Error in code: can't run out of ammo for knife.");
}
void fn_s_knife (weapon* w, geBoolean mbnew)
{
}
void fn_y_knife (weapon* w )
{
}
geBoolean fn_c_knife (weapon* w)
{
	return ( w->state == 9 || w->state == 5|| w->state == 6);
}
float fn_l_knife (weapon* w)
{
	return 0.0f;
}
//----------------------------


//---------------------------
// weapon #3: 10 lb (4.5 kg) Sledge hammer
void fn_p_hammer (weapon* w,geFloat t)
{
	w->stateTime += t* heroTime/enemyTime;

	switch(w->state)
	{
	case 0: // apply weapon
		if( w->stateTime >0.7f )
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 1: // right hand action
		if( w->stateTime > 0.8f )
		{
			SET_STATE(w, 9, 0.8f);
		}
		break;
	case 2: // left hand action
		if( w->stateTime > 0.6f)
		{
			SET_STATE( w, 9, 0.0f);
		}
		break;
	case 3: // powershot loading
		if( !(GetAsyncKeyState(controls.secondaryFire) & 0x8000) || w->stateTime > 1.8f )
		{
			if(GetAsyncKeyState(controls.secondaryFire) & 0x8000)
			{
				SET_STATE(w, 4, 0.0f);
				game_message("You'll feel a little bit flat soon");
			}
			else
			{
				SET_STATE(w, 0, 0.0f);
				game_message("Powerload failed");
			}
		}
		break;
	case 4: // power shot ready
		if( !(GetAsyncKeyState(controls.secondaryFire) & 0x8000) || w->stateTime > 5.0f)
		{
			if (GetAsyncKeyState(controls.secondaryFire) & 0x8000)
			{
				SET_STATE(w, 9, 0.0f);
				game_message("Time limmit exceeded");
			}
			else
			{
				game_message("Haaaaiiiiyyyaaa");
				SET_STATE(w, 5, 0.0f );

				if( player_fire_gun(FIGHT_TRACE_DISTANCE, FIGHT_ACC, &wIn, &wCol, w->theWeapon) )
				{
					bullet_hitObstacle_ex(&wCol, DECALTYPE_BULLETBIG, 0, 0, 1);
				}
				if( player_fire_gun(FIGHT_TRACE_DISTANCE, FIGHT_ACC, &wIn, &wCol, w->theWeapon) )
				{
					bullet_hitObstacle_ex(&wCol, DECALTYPE_BULLETBIG, 0, 0, 1);
				}
			}
		}
		break;
	case 5: // powershot action
		if( w->stateTime > 1.0f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 6: // fx1
		if( w->stateTime > 2.0f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 7: // fx 2
		if( w->stateTime > 2.0f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 9: // idle
		if( w->stateTime > 5.0f)
		{
			w->stateTime = 0.0f;
		}
		break;
	default:
		printLog("Error in process state for hammer\n");
		break;
	}
}
void fn_f_hammer (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	if( w->state == 9 || w->state == 6 || w->state == 7)
	{
		if(generate_random(2) )
		{
			SET_STATE(w, 1, 0.0f);
			soundsys_play_sound( &(w->theWeapon->weaponSound[1]), GE_FALSE );
		}
		else
		{
			SET_STATE(w, 2, 0.0f);
			soundsys_play_sound( &(w->theWeapon->weaponSound[2]), GE_FALSE );
		}
		if( player_fire_gun(FIGHT_TRACE_DISTANCE, FIGHT_ACC, &wIn, &wCol, w->theWeapon) )
		{
			bullet_hitObstacle_ex(&wCol, DECALTYPE_BULLETBIG, 0, 0, 1);
		}
	}
}
void fn_r_hammer (weapon* w)
{
	game_message("You can't reload the sledge hammer");
}
void fn_x_hammer (weapon* w)
{
	if( w->state ==9)
	{
		if( generate_random(2) )
		{
			SET_STATE(w, 6, 0.0f);
		}
		else
		{
			SET_STATE(w, 7, 0.0f);
		}

	}
}
void fn_a_hammer (weapon* w)
{
	w->state = 0;
	w->stateTime = 0.0f;
}
void fn_n_hammer (weapon* w, geBoolean mbnew)
{
	system_message("You can't run out of ammo for the hammer");
}
void fn_s_hammer (weapon* w, geBoolean mbnew)
{
	if( mbnew )
		if( w->state == 9 || w->state == 6 || w->state == 7)
		{
			game_message("Loading powershot");
			SET_STATE(w, 3, 0.0f);
		}
}
void fn_y_hammer (weapon* w )
{
}
geBoolean fn_c_hammer (weapon* w)
{
	return ( w->state == 9 || w->state == 6 || w->state == 7);
}
float fn_l_hammer (weapon* w)
{
	if( w->state == 3 )
	{
		return ( w->stateTime / 1.8f );
	}
	return 0.0f;
}
//----------------------------


//---------------------------
// weapon #4: Flathead axe
void fn_p_axe (weapon* w,geFloat t)
{
	w->stateTime += t* heroTime/enemyTime;

	switch(w->state)
	{
	case 0: // apply weapon
		if( w->stateTime >0.7f )
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 1: // right hand action
		if( w->stateTime > 0.5f )
		{
			SET_STATE(w, 9, 0.8f);
		}
		break;
	case 2: // left hand action
		if( w->stateTime > 0.5f)
		{
			SET_STATE( w, 9, 0.0f);
		}
		break;
	case 3: // powershot loading
		if( !(GetAsyncKeyState(controls.secondaryFire) & 0x8000) || w->stateTime > 1.8f )
		{
			if (GetAsyncKeyState(controls.secondaryFire) & 0x8000)
			{
				SET_STATE(w, 4, 0.0f);
				game_message("Slash n' dash");
			}
			else
			{
				SET_STATE(w, 9, 0.0f );
				game_message("Powerload failed");
			}
		}
		break;
	case 4: // power shot ready
		if( !(GetAsyncKeyState(controls.secondaryFire) & 0x8000) || w->stateTime > 5.0f)
		{
			if (GetAsyncKeyState(controls.secondaryFire) & 0x8000)
			{
				game_message("Time limmit exceeded");
				SET_STATE(w, 9, 0.0f);
			}
			else
			{
				game_message("Ha!");
				SET_STATE(w, 5, 0.0f );

				if( player_fire_gun(FIGHT_TRACE_DISTANCE, FIGHT_ACC, &wIn, &wCol, w->theWeapon) )
				{
					bullet_hitObstacle_ex(&wCol, DECALTYPE_BULLETBIG, 0, 0, 1);
				}
			}
		}
		break;
	case 5: // powershot action
		if( w->stateTime > 1.0f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 6: // fx1
		if( w->stateTime > 2.0f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 7: // fx 2
		if( w->stateTime > 2.0f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 9: // idle
		if( w->stateTime > 5.0f)
		{
			w->stateTime = 0.0f;
		}
		break;
	default:
		printLog("Error in process state for axe\n");
		break;
	}
}
void fn_f_axe (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	if( w->state == 9 || w->state == 6 || w->state == 7)
	{
		if(generate_random(2) )
		{
			soundsys_play_sound( &(w->theWeapon->weaponSound[1]), GE_FALSE );
			SET_STATE(w, 1, 0.0f);
		}
		else
		{
			soundsys_play_sound( &(w->theWeapon->weaponSound[2]), GE_FALSE );
			SET_STATE(w, 2, 0.0f);
		}

		if( player_fire_gun(FIGHT_TRACE_DISTANCE, FIGHT_ACC, &wIn, &wCol, w->theWeapon) )
		{
			bullet_hitObstacle_ex(&wCol, DECALTYPE_BULLETBIG, 0, 0, 1);
		}
	}
}
void fn_r_axe (weapon* w)
{
	game_message("You can't reload the axe");
}
void fn_x_axe (weapon* w)
{
	if( w->state ==9)
	{
		if( generate_random(2) )
		{
			SET_STATE(w, 6, 0.0f);
		}
		else
		{
			SET_STATE(w, 7, 0.0f);
		}

	}
}
void fn_a_axe (weapon* w)
{
	w->state = 0;
	w->stateTime = 0.0f;
}
void fn_n_axe (weapon* w, geBoolean mbnew)
{
	system_message("You can't run out of ammo for the axe");
}
void fn_s_axe (weapon* w, geBoolean mbnew)
{
	if( mbnew )
		if( w->state == 9 || w->state == 6 || w->state == 7)
		{
			SET_STATE(w, 3, 0.0f);
			game_message("Powershot loading");
		}
}
void fn_y_axe (weapon* w )
{
}
geBoolean fn_c_axe (weapon* w)
{
	return ( w->state == 9 || w->state == 6 || w->state == 7);
}
float fn_l_axe (weapon* w)
{
	if( w->state == 3 )
	{
		return ( w->stateTime / 1.8f );
	}
	return 0.0f;
}
//----------------------------




///////////////////////////////
// class: Pistols
///////////////////////////////

//---------------------------
// weapon #1: Glock 9mm
void fn_p_9mm (weapon* w,geFloat t)
{
	w->stateTime += t;

	DESTROY_LIGHT();

	switch(w->state)
	{
	case 0:
		if( w->stateTime > 0.5f)
		{
			game_message("Rock n' roll");
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 1:
		if( w->stateTime > 0.14f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 2:
		if( w->stateTime > 0.14f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 3:
		if( w->stateTime > 0.7f)
		{
			soundsys_play_sound( &(w->theWeapon->weaponSound[3]), GE_FALSE );
			SET_STATE(w, 4, 0.0f);
		}
		break;
	case 4:
		if( w->stateTime > 0.7f)
		{
			soundsys_play_sound( &(w->theWeapon->weaponSound[4]), GE_FALSE );
			SET_STATE(w, 5, 0.0f);
		}
		break;
	case 5:
		if( w->stateTime > 0.7f)
		{
			game_message("Rock n' roll");
			SET_STATE(w, 9, 0.0f);
			if( !cheats.unlimited_mag ) w->magasines--;
			w->ammonution = w->theWeapon->max_ammo;
		}
		break;
	case 6:
		if( w->stateTime > 4.0f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 7:
		if( w->stateTime > 4.5f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 9:
		if( w->stateTime > 10.0f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	default:
		printLog("Error in process state for 9mm\n");
		break;
	}
}
void fn_f_9mm (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	if( w->state == 9 || w->state == 6 || w->state == 7 )
	{
		soundsys_play_sound( &(w->theWeapon->weaponSound[1]), GE_FALSE );
		SET_STATE(w, 1, 0.0f);
		if(!cheats.unlimited_ammo) w->ammonution -= 1;

		weapon_flash();

		if( player_fire_gun(BULLET_TRACE_DISTANCE, 4.0f, &wIn, &wCol, w->theWeapon) )
		{
			bullet_hitObstacle(&wCol, DECALTYPE_BULLET);
		}
	}
}
void fn_r_9mm (weapon* w)
{
	if( w->magasines > 0 )
	if( w->state == 9 || w->state == 6 || w->state == 7 )
	{
		SET_STATE(w, 3, 0.0f);
		soundsys_play_sound( &(w->theWeapon->weaponSound[2]), GE_FALSE );
	}
}
void fn_x_9mm (weapon* w)
{
	if( w->state == 9 )
	{
		if( generate_random(2) )
		{
			SET_STATE(w, 6, 0.0f);
		}
		else
		{
			SET_STATE(w, 7, 0.0f);
		}
	}
}
void fn_a_9mm (weapon* w)
{
	w->state = 0;
	w->stateTime = 0.0f;
	soundsys_play_sound( &(w->theWeapon->weaponSound[4]), GE_FALSE );
}
void fn_n_9mm (weapon* w, geBoolean mbnew)
{
	if( mbnew )
		game_message("Click");
}
void fn_s_9mm (weapon* w, geBoolean mbnew)
{
	if( w->ammonution > 0 )
	{
		if( w->state == 9 || w->state == 6 || w->state == 7 )
		{
			SET_STATE(w, 2, 0.0f);
			soundsys_play_sound( &(w->theWeapon->weaponSound[1]), GE_FALSE );
			if(!cheats.unlimited_ammo) w->ammonution -= 1;
			weapon_flash();
			if( player_fire_gun(BULLET_TRACE_DISTANCE, 8.0f, &wIn, &wCol, w->theWeapon) )
			{
				bullet_hitObstacle(&wCol, DECALTYPE_BULLET);
			}
		}
	}
	else
	{
		w->theWeapon->fn_noAmmo(w, mbnew);
	}
}
void fn_y_9mm (weapon* w )
{
}
geBoolean fn_c_9mm (weapon* w)
{
	return ( w->state == 9 || w->state == 6 || w->state == 7 );
}
float fn_l_9mm (weapon* w)
{
	return 0.0f;
}
//----------------------------


//---------------------------
// weapon #2: Desert Eagle .50



void fn_p_eagle (weapon* w,geFloat t)
{
	w->stateTime += t;

	DESTROY_LIGHT();

	switch(w->state)
	{
	case 0:
		if( w->stateTime > 0.5f)
		{
			game_message("Rock n' roll");
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 1:
		if( w->stateTime > 0.14f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 2:
		if( w->stateTime > 0.14f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 3:
		if( w->stateTime > 0.5f)
		{
			soundsys_play_sound( &(w->theWeapon->weaponSound[3]), GE_FALSE );
			SET_STATE(w, 4, 0.0f);
		}
		break;
	case 4:
		if( w->stateTime > 0.5f)
		{
			soundsys_play_sound( &(w->theWeapon->weaponSound[4]), GE_FALSE );
			SET_STATE(w, 5, 0.0f);
		}
		break;
	case 5:
		if( w->stateTime > 0.5f)
		{
			game_message("Rock n' roll");
			SET_STATE(w, 9, 0.0f);
			if( !cheats.unlimited_mag ) w->magasines--;
			w->ammonution = w->theWeapon->max_ammo;
		}
		break;
	case 6:
		if( w->stateTime > 4.0f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 7:
		if( w->stateTime > 4.5f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 9:
		if( w->stateTime > 10.0f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	default:
		printLog("Error in process state for Desert eagle\n");
		break;
	}
}
void fn_f_eagle (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	if( w->state == 9 || w->state == 6 || w->state == 7 )
	{
		soundsys_play_sound( &(w->theWeapon->weaponSound[1]), GE_FALSE );
		SET_STATE(w, 1, 0.0f);
		if(!cheats.unlimited_ammo) w->ammonution -= 1;
		weapon_flash();

		if( player_fire_gun(BULLET_TRACE_DISTANCE, 5.0f, &wIn, &wCol, w->theWeapon) )
		{
			bullet_hitObstacle(&wCol, DECALTYPE_BULLETBIG);
		}
	}
}
void fn_r_eagle (weapon* w)
{
	if( w->magasines > 0 )
	if( w->state == 9 || w->state == 6 || w->state == 7 )
	{
		SET_STATE(w, 3, 0.0f);
		soundsys_play_sound( &(w->theWeapon->weaponSound[2]), GE_FALSE );
	}
}
void fn_x_eagle (weapon* w)
{
	if( w->state == 9 )
	{
		if( generate_random(2) )
		{
			SET_STATE(w, 6, 0.0f);
		}
		else
		{
			SET_STATE(w, 7, 0.0f);
		}
	}
}
void fn_a_eagle (weapon* w)
{
	w->state = 0;
	w->stateTime = 0.0f;
	soundsys_play_sound( &(w->theWeapon->weaponSound[4]), GE_FALSE );
}
void fn_n_eagle (weapon* w, geBoolean mbnew)
{
	if( mbnew )
		game_message("Click");
}
void fn_s_eagle (weapon* w, geBoolean mbnew)
{
	if( w->ammonution > 0 )
	{
		if( w->state == 9 || w->state == 6 || w->state == 7 )
		{
			SET_STATE(w, 2, 0.0f);
			soundsys_play_sound( &(w->theWeapon->weaponSound[1]), GE_FALSE );
			if(!cheats.unlimited_ammo) w->ammonution -= 1;
			weapon_flash();
			if( player_fire_gun(BULLET_TRACE_DISTANCE, 10.0f, &wIn, &wCol, w->theWeapon) )
			{
				bullet_hitObstacle(&wCol, DECALTYPE_BULLETBIG);
			}
		}
	}
	else
	{
		w->theWeapon->fn_noAmmo(w, mbnew);
	}
}
void fn_y_eagle (weapon* w )
{
}
geBoolean fn_c_eagle (weapon* w)
{
	return ( w->state == 9 || w->state == 6 || w->state == 7 );
}
float fn_l_eagle (weapon* w)
{
	return 0.0f;
}
//----------------------------


//---------------------------
// weapon #3: Improved tranqualizer
void fn_p_tranqualizer (weapon* w,geFloat t)
{
	w->stateTime += t;

	DESTROY_LIGHT();

	switch(w->state)
	{
	case 0://grabbing weapon
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 1://shoot
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 2:
		if( w->stateTime > 0.5f )
		{
			game_message("Jack in");
			SET_STATE(w, 3, 0.0f);
		}
		break;
	case 3:
		if( w->stateTime > 0.5f )
		{
			game_message("click");
			SET_STATE(w, 4, 0.0f);
		}
		break;
	case 4:
		if( w->stateTime > 0.3f )
		{
			w->ammonution = 1;
			w->magasines--;
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 5:
		if( w->stateTime > 1.3f )
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 6:
		if( w->stateTime > 4.3f )
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 7:
		if( w->stateTime > 2.3f )
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 9:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	default:
		printLog("Error in process state for tranqualizer\n");
		break;
	}
}
void fn_f_tranqualizer (weapon* w, geBoolean mbnew)
{
	if( w->state==9||w->state==5||w->state==6||w->state==7 )
	{
		SET_STATE(w,1, 0.0f);
		game_message("Phjut");
		weapon_flash();
		if(!cheats.unlimited_ammo) w->ammonution -= 1;
	}
}
void fn_r_tranqualizer (weapon* w)
{
	if( w->magasines > 0 )
	if( w->state==9||w->state==5||w->state==6||w->state==7 )
	{
		SET_STATE(w, 2, 0.0f);
		if( !cheats.unlimited_mag ) w->magasines--;
	}
}
void fn_x_tranqualizer (weapon* w)
{
	if( w->state == 9 )
	{
		switch(generate_random(3) )
		{
		case 0:
			SET_STATE(w, 5, 0.0f);
			break;
		case 1:
			SET_STATE(w, 6, 0.0f);
			break;
		default:
			SET_STATE(w, 7, 0.0f);
			break;
		}
	}
}
void fn_a_tranqualizer (weapon* w)
{
	w->state = 0;
	w->stateTime = 0.0f;
}
void fn_n_tranqualizer (weapon* w, geBoolean mbnew)
{
	if( mbnew )
		game_message("click");
}
void fn_s_tranqualizer (weapon* w, geBoolean mbnew)
{
}
void fn_y_tranqualizer (weapon* w )
{
}
geBoolean fn_c_tranqualizer (weapon* w)
{
	return ( w->state==9||w->state==5||w->state==6||w->state==7 );
}
float fn_l_tranqualizer (weapon* w)
{
	return 0.0f;
}
//----------------------------


//---------------------------
// weapon #4: Improved tazer pistol
void fn_p_tazer (weapon* w,geFloat t)
{
	w->stateTime += t;

	DESTROY_LIGHT();

	switch(w->state)
	{
	case 0:
		if( w->stateTime > 0.5f)
		{
			soundsys_play_sound( &(w->theWeapon->weaponSound[0]), GE_FALSE );
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 1://fire
		if( w->stateTime > 0.15f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 2:
		if( w->stateTime > 0.13f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 3:
		if( w->stateTime > 0.5f)
		{
			game_message("Bat in");
			SET_STATE(w, 4, 0.0f);
		}
		break;
	case 4:
		if( w->stateTime > 0.5f)
		{
			soundsys_play_sound( &(w->theWeapon->weaponSound[2]), GE_FALSE );
			SET_STATE(w, 5, 0.0f);
		}
		break;
	case 5:
		if( w->stateTime > 0.5f)
		{
			game_message("Let's blast some");
			SET_STATE(w, 9, 0.0f);
			if( !cheats.unlimited_mag ) w->magasines--;
			w->ammonution = w->theWeapon->max_ammo;
		}
		break;
	case 6:
		if( w->stateTime > 4.0f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 7:
		if( w->stateTime > 4.5f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	case 9:
		if( w->stateTime > 10.0f)
		{
			SET_STATE(w, 9, 0.0f);
		}
		break;
	default:
		printLog("Error in process state for tazer\n");
		break;
	}
}
void fn_f_tazer (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	if( w->state == 9 || w->state == 6 || w->state == 7 )
	{
		soundsys_play_sound( &(w->theWeapon->weaponSound[1]), GE_FALSE );
		SET_STATE(w, 1, 0.0f);
		weapon_flash();
		if(!cheats.unlimited_ammo) w->ammonution -= 1;

		if( player_fire_gun(BULLET_TRACE_DISTANCE, 0.5f, &wIn, &wCol, w->theWeapon) )
		{
			bullet_hitObstacle(&wCol, DECALTYPE_BULLETBIG);
		}

		fx_blast(XForm.Translation, wCol.Impact);
	}
}
void fn_r_tazer (weapon* w)
{
	if( w->magasines > 0 )
	if( w->state == 9 || w->state == 6 || w->state == 7 )
	{
		SET_STATE(w, 3, 0.0f);
		game_message("Bat out");
	}
}
void fn_x_tazer (weapon* w)
{
	if( w->state == 9 )
	{
		if( generate_random(2) )
		{
			SET_STATE(w, 6, 0.0f);
		}
		else
		{
			SET_STATE(w, 7, 0.0f);
		}
	}
}
void fn_a_tazer (weapon* w)
{
	w->state = 0;
	w->stateTime = 0.0f;
}
void fn_n_tazer (weapon* w, geBoolean mbnew)
{
	if( mbnew )
		game_message("Click");
}
void fn_s_tazer (weapon* w, geBoolean mbnew)
{
	if( w->ammonution > 0 )
	{
		if( w->state == 9 || w->state == 6 || w->state == 7 )
		{
			SET_STATE(w, 2, 0.0f);
			soundsys_play_sound( &(w->theWeapon->weaponSound[1]), GE_FALSE );
			if(!cheats.unlimited_ammo) w->ammonution -= 1;
			weapon_flash();
			if( player_fire_gun(BULLET_TRACE_DISTANCE, 12.0f, &wIn, &wCol, w->theWeapon) )
			{
				bullet_hitObstacle(&wCol, DECALTYPE_BULLETBIG);
			}
			fx_blast(XForm.Translation, wCol.Impact);
		}
	}
	else
	{
		w->theWeapon->fn_noAmmo(w, mbnew);
	}
}
void fn_y_tazer (weapon* w )
{
}
geBoolean fn_c_tazer (weapon* w)
{
	return ( w->state == 9 || w->state == 6 || w->state == 7 );
}
float fn_l_tazer (weapon* w)
{
	return 0.0f;
}
//----------------------------



///////////////////////////////
// class: Rifles
///////////////////////////////

//---------------------------
// weapon #1: Itacha 37 homeland secutrity 
void fn_p_shotgun (weapon* w,geFloat t)
{
	w->stateTime += t;

	DESTROY_LIGHT();

	switch(w->state)
	{
	case 0:
		if( w->stateTime > 0.5f )
		{
			SET_STATE(w, 1, 0.0f );
		}
		break;
	case 1:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 1, 0.0f );
		}
		break;
	case 2:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 2, 0.0f );
		}
		break;
	case 3://bullet in
		if( w->stateTime > 0.6f )
		{
			w->ammonution ++;
			if( !cheats.unlimited_mag ) w->magasines--;

			if( GetAsyncKeyState( controls.reload )&0x8000 && w->magasines>0 && w->ammonution!=w->theWeapon->max_ammo)
			{
				SET_STATE(w, 3, 0.0f);
				soundsys_play_sound( &(w->theWeapon->weaponSound[2]), GE_FALSE );
			}
			else
			{
				SET_STATE(w, 4, 0.0f );
				soundsys_play_sound( &(w->theWeapon->weaponSound[3]), GE_FALSE );
			}
		}
		break;
	case 4:
		if( w->stateTime > 1.0f )
		{
			game_message("Okay punk");
			SET_STATE(w, 1, 0.0f );
		}
		break;
	case 5:
		if( w->stateTime > 0.5f )
		{
			soundsys_play_sound( &(w->theWeapon->weaponSound[3]), GE_FALSE );
			SET_STATE(w, 7, 0.0f );
		}
		break;
	case 6:
		if( w->stateTime > 0.3f )
		{
			soundsys_play_sound( &(w->theWeapon->weaponSound[3]), GE_FALSE );
			SET_STATE(w, 8, 0.0f );
		}
		break;
	case 7:
		if( w->stateTime > 0.5f )
		{
			SET_STATE(w, 1, 0.0f );
		}
		break;
	case 8:
		if( w->stateTime > 0.4f )
		{
			SET_STATE(w, 2, 0.0f );
		}
		break;
	default:
		printLog("Error in process state for shotgun\n");
		break;
	}
}
void fn_f_shotgun (weapon* w, geBoolean mbnew)
{
	if( w->state == 1 ) // ok shot
	{
		if( mbnew )
		{
			char i;

			soundsys_play_sound( &(w->theWeapon->weaponSound[1]), GE_FALSE );
			SET_STATE(w, 5, 0.0f);
			if(!cheats.unlimited_ammo) w->ammonution -= 1;
			weapon_flash();

			for( i= 0; i< 6; i++ )
			{
				if( player_fire_gun(BULLET_TRACE_DISTANCE, 40.0f, &wIn, &wCol, w->theWeapon) )
				{
					bullet_hitObstacle(&wCol, DECALTYPE_BULLETBIG);
				}
			}
		}
	}
	else
	{
		if( w->state == 2 ) // good shot
		{
			char i;

			soundsys_play_sound( &(w->theWeapon->weaponSound[1]), GE_FALSE );
			SET_STATE(w, 6, 0.0f);
			if(!cheats.unlimited_ammo) w->ammonution -= 1;
			weapon_flash();
			for( i=0; i< 6; i++)
			{
				if( player_fire_gun(BULLET_TRACE_DISTANCE, 20.0f, &wIn, &wCol, w->theWeapon) )
				{
					bullet_hitObstacle(&wCol, DECALTYPE_BULLETBIG);
				}
			}
		}
	}
}
void fn_r_shotgun (weapon* w)
{
	if( w->magasines > 0 )
	{
		if( w->state == 1 || w->state == 2 )
		{
			disable_sniper();
			SET_STATE(w, 3, 0.0f);
			soundsys_play_sound( &(w->theWeapon->weaponSound[2]), GE_FALSE );
		}
	}
}
void fn_x_shotgun (weapon* w)
{
}
void fn_a_shotgun (weapon* w)
{
	w->state = 0;
	w->stateTime = 0.0f;
	soundsys_play_sound( &(w->theWeapon->weaponSound[3]), GE_FALSE );
}
void fn_n_shotgun (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	game_message("click");
}
void fn_s_shotgun (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	if( w->state == 1 )
	{
		SET_STATE(w, 2, 0.0f );
		enable_sniper(1.0f);
	}
	else
	{
		if( w->state == 2 )
		{
			SET_STATE(w, 1, 0.0f );
			disable_sniper();
		}
	}
}
void fn_y_shotgun (weapon* w )
{
	disable_sniper();
}
geBoolean fn_c_shotgun (weapon* w)
{
	return ( w->state==1 || w->state==2 );
}
float fn_l_shotgun (weapon* w)
{
	return 0.0f;
}
//----------------------------


//---------------------------
// weapon #2: Mag7
void fn_p_mag7 (weapon* w,geFloat t)
{
	w->stateTime += t;

	DESTROY_LIGHT();

	switch(w->state)
	{
	case 0:
		if( w->stateTime > 0.3f )
		{
			game_message("Wanna some mag7");
			SET_STATE(w, 9, 0.0f );
		}
		break;
	case 1:
		if( w->stateTime > 0.3f )
		{
			soundsys_play_sound( &(w->theWeapon->weaponSound[2]) , GE_FALSE);
			SET_STATE(w, 2, 0.0f );
		}
		break;
	case 2:
		if( w->stateTime > 0.4f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	case 3:
		if( w->stateTime > 0.4f )
		{
			soundsys_play_sound( &(w->theWeapon->weaponSound[3]), GE_FALSE );
			w->ammonution = w->theWeapon->max_ammo;
			if( !cheats.unlimited_mag ) w->magasines--;
			SET_STATE(w, 4, 0.0f );
		}
		break;
	case 4:
		if( w->stateTime > 0.4f )
		{
			soundsys_play_sound( &(w->theWeapon->weaponSound[2]), GE_FALSE );
			SET_STATE(w, 2, 0.0f );
		}
		break;
	case 5:
		if( w->stateTime > 1.3f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	case 6:
		if( w->stateTime > 0.8f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	case 9:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	default:
		printLog("Error in process state for mag7\n");
		break;
	}
}
void fn_f_mag7 (weapon* w, geBoolean mbnew)
{
	if( w->state == 9 || w->state == 5 || w->state == 6 )
	{
		char i;
		
		soundsys_play_sound( &(w->theWeapon->weaponSound[1]), GE_FALSE );
		SET_STATE(w, 1, 0.0f);
		weapon_flash();
		if(!cheats.unlimited_ammo) w->ammonution -= 1;
		
		for( i=0; i< 6; i++)
		{
			if( player_fire_gun(BULLET_TRACE_DISTANCE, 60.0f, &wIn, &wCol, w->theWeapon) )
			{
				bullet_hitObstacle(&wCol, DECALTYPE_BULLETBIG);
			}
		}
	}
}
void fn_r_mag7 (weapon* w)
{
	if( w->magasines > 0 )
	{
		if( w->state == 9 || w->state == 5 || w->state == 6 )
		{
			SET_STATE(w, 3, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[4]), GE_FALSE );
		}
	}
}
void fn_x_mag7 (weapon* w)
{
	if( w->state == 9 )
	{
		if( generate_random(2) )
		{
			SET_STATE(w, 5, 0.0f);
		}
		else
		{
			SET_STATE(w, 6, 0.0f);
		}
	}
}
void fn_a_mag7 (weapon* w)
{
	w->state = 0;
	w->stateTime = 0.0f;
	soundsys_play_sound( &(w->theWeapon->weaponSound[2]), GE_FALSE );
}
void fn_n_mag7 (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	game_message("click");
}
void fn_s_mag7 (weapon* w, geBoolean mbnew)
{
}
void fn_y_mag7 (weapon* w )
{
}
geBoolean fn_c_mag7 (weapon* w)
{
	return ( w->state == 9 || w->state == 5 || w->state == 6 );
}
float fn_l_mag7 (weapon* w)
{
	return 0.0f;
}
//----------------------------


//---------------------------
// weapon #3: Beretta 501 Sniper
void fn_p_sniper (weapon* w,geFloat t)
{
	w->stateTime += t;

	DESTROY_LIGHT();

	switch(w->state)
	{
	case 0:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 9, 0.0f );
			w->value = 0;
		}
		break;
	case 1:
		if( w->stateTime > 0.5f )
		{
			SET_STATE(w, 2, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[4]), GE_FALSE );
		}
		break;
	case 2:
		if( w->stateTime > 0.6f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	case 3:
		if( w->stateTime > 0.6f )
		{
			SET_STATE(w, 4, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[2]), GE_FALSE );
		}
		break;
	case 4:
		if( w->stateTime > 0.6f )
		{
			SET_STATE(w, 2, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[4]), GE_FALSE );
			if( !cheats.unlimited_mag ) w->magasines--;
			w->ammonution = w->theWeapon->max_ammo;
		}
		break;
	case 5:
		if( w->stateTime > 0.4f )
		{
			SET_STATE(w, 6, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[4]), GE_FALSE );
		}
		break;
	case 6:
		if( w->stateTime > 0.4f )
		{
			SET_STATE(w, 8, 0.0f );
		}
		break;
	case 8:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 8, 0.0f );
		}
		break;
	case 9:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	default:
		printLog("Error in process state for sniper\n");
		break;
	}
}
void fn_f_sniper (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	if( w->state == 9 )
	{
		soundsys_play_sound( &(w->theWeapon->weaponSound[1]), GE_FALSE );
		SET_STATE(w, 1, 0.0f );
		weapon_flash();
		if(!cheats.unlimited_ammo) w->ammonution -= 1;

		if( player_fire_gun(BULLET_TRACE_DISTANCE, 4.0f, &wIn, &wCol, w->theWeapon) )
		{
				bullet_hitObstacle(&wCol, DECALTYPE_BULLET);
		}
	}
	else
	{
		if( w->state == 8 )
		{
			soundsys_play_sound( &(w->theWeapon->weaponSound[1]), GE_FALSE );
			SET_STATE(w, 5, 0.0f );
			weapon_flash();
			if(!cheats.unlimited_ammo) w->ammonution -= 1;

			if( player_fire_gun(BULLET_TRACE_DISTANCE, 1.0f, &wIn, &wCol, w->theWeapon) )
			{
				bullet_hitObstacle(&wCol, DECALTYPE_BULLET);
			}
		}
	}
}
void fn_r_sniper (weapon* w)
{
	if( w->magasines > 0 )
	if( w->state == 9 || w->state == 8)
	{
		SET_STATE(w, 3, 0.0f );
		soundsys_play_sound( &(w->theWeapon->weaponSound[3]), GE_FALSE );
		disable_sniper();
	}
}
void fn_x_sniper (weapon* w)
{
}
void fn_a_sniper (weapon* w)
{
	w->state = 0;
	w->stateTime = 0.0f;
}
void fn_n_sniper (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	game_message("click");
}
void fn_s_sniper (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	{
		if( w->state == 8 || w->state == 9 ){
			switch( w->value ) {
			case 0:
				SET_STATE(w, 8, 0.0f );
				enable_sniper( 0.7f );
				break;
			case 1:
				SET_STATE(w, 8, 0.0f );
				enable_sniper( 0.5f );
				break;
			case 2:
				SET_STATE(w, 8, 0.0f );
				enable_sniper( 0.2f );
				break;
			case 3:
				SET_STATE(w, 9, 0.0f );
				disable_sniper();
				return;
			}
			w->value++;
			soundsys_play_sound( &(w->theWeapon->weaponSound[0]), GE_FALSE );
		}
		/*if( w->state == 8 )
		{
			SET_STATE(w, 9, 0.0f );
			disable_sniper();
		}
		else if ( w->state == 9 )
		{
			SET_STATE(w, 8, 0.0f );
			enable_sniper( 0.5f );
		}*/
	}
}
void fn_y_sniper (weapon* w )
{
	disable_sniper();
}
geBoolean fn_c_sniper (weapon* w)
{
	return (w->state==8 || w->state==9);
}
float fn_l_sniper (weapon* w)
{
	return 0.0f;
}
//----------------------------


//---------------------------
// weapon #4: Barett "Light fiffty" M82A1
void fn_p_barett (weapon* w,geFloat t)
{
	w->stateTime += t;

	DESTROY_LIGHT();

	switch(w->state)
	{
	case 0:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 9, 0.0f );
			w->value = 0;
		}
		break;
	case 1:
		if( w->stateTime > 0.5f )
		{
			SET_STATE(w, 2, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[4]), GE_FALSE );
		}
		break;
	case 2:
		if( w->stateTime > 0.6f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	case 3:
		if( w->stateTime > 0.6f )
		{
			SET_STATE(w, 4, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[2]), GE_FALSE );
		}
		break;
	case 4:
		if( w->stateTime > 0.6f )
		{
			SET_STATE(w, 2, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[4]), GE_FALSE );
			if( !cheats.unlimited_mag ) w->magasines--;
			w->ammonution = w->theWeapon->max_ammo;
		}
		break;
	case 5:
		if( w->stateTime > 0.4f )
		{
			SET_STATE(w, 6, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[4]), GE_FALSE );
		}
		break;
	case 6:
		if( w->stateTime > 0.4f )
		{
			SET_STATE(w, 8, 0.0f );
		}
		break;
	case 8:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 8, 0.0f );
		}
		break;
	case 9:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	default:
		printLog("Error in process state for barett\n");
		break;
	}
}
void fn_f_barett (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	if( w->state == 9 )
	{
		soundsys_play_sound( &(w->theWeapon->weaponSound[1]), GE_FALSE );
		SET_STATE(w, 1, 0.0f );
		weapon_flash();
		if(!cheats.unlimited_ammo) w->ammonution -= 1;

		if( player_fire_gun(BULLET_TRACE_DISTANCE, 1.0f, &wIn, &wCol, w->theWeapon) )
		{
			bullet_hitObstacle(&wCol, DECALTYPE_BULLETBIG);
		}
	}
	else
	{
		if( w->state == 8 )
		{
			soundsys_play_sound( &(w->theWeapon->weaponSound[1]), GE_FALSE );
			SET_STATE(w, 5, 0.0f );
			weapon_flash();
			if(!cheats.unlimited_ammo) w->ammonution -= 1;

			if( player_fire_gun(BULLET_TRACE_DISTANCE, 0.3f, &wIn, &wCol, w->theWeapon) )
			{
				bullet_hitObstacle(&wCol, DECALTYPE_BULLETBIG);
			}
		}
	}
}
void fn_r_barett (weapon* w)
{
	if( w->magasines > 0 )
	if( w->state == 9 || w->state == 8)
	{
		SET_STATE(w, 3, 0.0f );
		soundsys_play_sound( &(w->theWeapon->weaponSound[3]), GE_FALSE );
		disable_sniper();
	}
}
void fn_x_barett (weapon* w)
{
}
void fn_a_barett (weapon* w)
{
	w->state = 0;
	w->stateTime = 0.0f;
}
void fn_n_barett (weapon* w, geBoolean mbnew)
{
	if( mbnew )
		game_message("click");
}
void fn_s_barett (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	{
		if( w->state == 8 || w->state == 9 ){
			soundsys_play_sound( &(w->theWeapon->weaponSound[0]), GE_FALSE );
			switch( w->value ) {
			case 0:
				SET_STATE(w, 8, 0.0f );
				enable_sniper( 0.5f );
				break;
			case 1:
				SET_STATE(w, 8, 0.0f );
				enable_sniper( 0.1f );
				break;
			case 2:
				SET_STATE(w, 8, 0.0f );
				enable_sniper( 0.05f );
				break;
			case 3:
				SET_STATE(w, 9, 0.0f );
				disable_sniper();
				return;
			}
			w->value++;
		}
	}
}
void fn_y_barett (weapon* w )
{
	disable_sniper();
}
geBoolean fn_c_barett (weapon* w)
{
	return (w->state==8 || w->state==9);
}
float fn_l_barett (weapon* w)
{
	return 0.0f;
}
//----------------------------



///////////////////////////////
// class: Automatics
///////////////////////////////

//---------------------------
// weapon #1: HK mp-5k a4
void fn_p_smg (weapon* w,geFloat t)
{
	w->stateTime += t;

	DESTROY_LIGHT();

	switch(w->state)
	{
	case 0:
		if( w->stateTime > 0.4 )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	case 1:
		if( w->stateTime > 0.1 )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	case 2:
		if( w->stateTime > 0.3 )
		{
			SET_STATE(w, 3, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[2]), GE_FALSE );
		}
		break;
	case 3:
		if( w->stateTime > 0.6 )
		{
			SET_STATE(w, 4, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[1]), GE_FALSE );
		}
		break;
	case 4:
		if( w->stateTime > 0.6 )
		{
			SET_STATE(w, 5, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[3]), GE_FALSE );
			w->ammonution = w->theWeapon->max_ammo;
			if( !cheats.unlimited_mag ) w->magasines--;
		}
		break;
	case 5:
		if( w->stateTime > 0.3 )
		{
			SET_STATE(w, 9, 0.0f );
			game_message("Make my day");
		}
		break;
	case 9:
		if( w->stateTime > 10.0 )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	default:
		printLog("Error in process state for smg\n");
		break;
	}
}
void fn_f_smg (weapon* w, geBoolean mbnew)
{
	if( w->state == 9)
	{
		if(!cheats.unlimited_ammo) w->ammonution -= 1;
		SET_STATE(w, 1, 0.0f );
		soundsys_play_sound( &(w->theWeapon->weaponSound[0]), GE_FALSE );

		weapon_flash();

		if( player_fire_gun(BULLET_TRACE_DISTANCE, 15.0f, &wIn, &wCol, w->theWeapon) )
		{
			bullet_hitObstacle(&wCol, DECALTYPE_BULLET);
		}
	}
}
void fn_r_smg (weapon* w)
{
	if( w->magasines > 0 )
	if( w->state == 9 )
	{
		SET_STATE(w, 2, 0.0f );
		soundsys_play_sound( &(w->theWeapon->weaponSound[4]), GE_FALSE );
	}
}
void fn_x_smg (weapon* w)
{
}
void fn_a_smg (weapon* w)
{
	w->state = 0;
	w->stateTime = 0.0f;
}
void fn_n_smg (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	if( w->state == 9 )
	{
		game_message("click");
	}
}
void fn_s_smg (weapon* w, geBoolean mbnew)
{
}
void fn_y_smg (weapon* w )
{
}
geBoolean fn_c_smg (weapon* w)
{
	return ( w->state == 9 );
}
float fn_l_smg (weapon* w)
{
	return 0.0f;
}
//----------------------------


//---------------------------
// weapon #2: Uzi 9mm
void fn_p_uzi (weapon* w,geFloat t)
{
	w->stateTime += t;

	DESTROY_LIGHT();

	switch(w->state)
	{
	case 0:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	case 1:
		if( w->stateTime > 0.05f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	case 2:
		if( w->stateTime > 0.4f )
		{
			SET_STATE(w, 3, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[1]), GE_FALSE );
		}
		break;
	case 3:
		if( w->stateTime > 0.4f )
		{
			SET_STATE(w, 4, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[3]), GE_FALSE );
		}
		break;
	case 4:
		if( w->stateTime > 0.4f )
		{
			SET_STATE(w, 9, 0.0f );
			game_message("Com' on");
			w->ammonution = w->theWeapon->max_ammo;
			if( !cheats.unlimited_mag ) w->magasines--;
		}
		break;
	case 9:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	default:
		printLog("Error in process state for uzi\n");
		break;
	}
}
void fn_f_uzi (weapon* w, geBoolean mbnew)
{
	if( w->state == 9 )
	{
		soundsys_play_sound( &(w->theWeapon->weaponSound[0]), GE_FALSE );
		if(!cheats.unlimited_ammo) w->ammonution -= 1;
		SET_STATE(w, 1, 0.0f );
		weapon_flash();

		if( player_fire_gun(BULLET_TRACE_DISTANCE, 19.0f, &wIn, &wCol, w->theWeapon) )
		{
			bullet_hitObstacle(&wCol, DECALTYPE_BULLET);
		}
	}
}
void fn_r_uzi (weapon* w)
{
	if( w->magasines > 0 )
	if( w->state == 9 )
	{
		SET_STATE(w, 2, 0.0f );
		soundsys_play_sound( &(w->theWeapon->weaponSound[2]), GE_FALSE );
	}
}
void fn_x_uzi (weapon* w)
{
}
void fn_a_uzi (weapon* w)
{
	w->state = 0;
	w->stateTime = 0.0f;
}
void fn_n_uzi (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	{
		soundsys_play_sound( &(w->theWeapon->weaponSound[4]), GE_FALSE );
	}
}
void fn_s_uzi (weapon* w, geBoolean mbnew)
{
}
void fn_y_uzi (weapon* w )
{
}
geBoolean fn_c_uzi (weapon* w)
{
	return ( w->state == 9 );
}
float fn_l_uzi (weapon* w)
{
	return 0.0f;
}
//----------------------------


//---------------------------
// weapon #3: Ak47
void fn_p_ak47 (weapon* w,geFloat t)
{
	w->stateTime += t;

	DESTROY_LIGHT();

	switch(w->state)
	{
	case 0:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	case 1:
		if( w->stateTime > 0.1f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	case 2:
		if( w->stateTime > 0.4f )
		{
			SET_STATE(w, 3, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[1]), GE_FALSE );
		}
		break;
	case 3:
		if( w->stateTime > 0.4f )
		{
			SET_STATE(w, 4, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[3]), GE_FALSE );
		}
		break;
	case 4:
		if( w->stateTime > 0.4f )
		{
			SET_STATE(w, 9, 0.0f );
			game_message("Com' on");
			w->ammonution = w->theWeapon->max_ammo;
			if( !cheats.unlimited_mag ) w->magasines--;
		}
		break;
	case 9:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	default:
		printLog("Error in process state for ak47\n");
		break;
	}
}
void fn_f_ak47 (weapon* w, geBoolean mbnew)
{
	if( w->state == 9 )
	{
		soundsys_play_sound( &(w->theWeapon->weaponSound[0]), GE_FALSE );
		if(!cheats.unlimited_ammo) w->ammonution -= 1;
		SET_STATE(w, 1, 0.0f );
		weapon_flash();

		if( player_fire_gun(BULLET_TRACE_DISTANCE, 27.0f, &wIn, &wCol, w->theWeapon) )
		{
			bullet_hitObstacle(&wCol, DECALTYPE_BULLETBIG);
		}
	}
}
void fn_r_ak47 (weapon* w)
{
	if( w->magasines > 0 )
	if( w->state == 9 )
	{
		SET_STATE(w, 2, 0.0f );
		soundsys_play_sound( &(w->theWeapon->weaponSound[2]), GE_FALSE );
	}
}
void fn_x_ak47 (weapon* w)
{
}
void fn_a_ak47 (weapon* w)
{
	w->state = 0;
	w->stateTime = 0.0f;
}
void fn_n_ak47 (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	{
		game_message("click");
	}
}
void fn_s_ak47 (weapon* w, geBoolean mbnew)
{
}
void fn_y_ak47 (weapon* w )
{
}
geBoolean fn_c_ak47 (weapon* w)
{
	return ( w->state == 9 );
}
float fn_l_ak47 (weapon* w)
{
	return 0.0f;
}
//----------------------------


//---------------------------
// weapon #4: Colt Commando 733
void fn_p_commando (weapon* w,geFloat t)
{
	w->stateTime += t;

	DESTROY_LIGHT();

	switch(w->state)
	{
	case 0:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	case 1:
		if( w->stateTime > 0.08f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	case 2:
		if( w->stateTime > 0.4f )
		{
			SET_STATE(w, 3, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[0]), GE_FALSE );
		}
		break;
	case 3:
		if( w->stateTime > 0.4f )
		{
			SET_STATE(w, 4, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[3]), GE_FALSE );
		}
		break;
	case 4:
		if( w->stateTime > 0.4f )
		{
			SET_STATE(w, 9, 0.0f );
			game_message("Com' on");
			w->ammonution = w->theWeapon->max_ammo;
			if( !cheats.unlimited_mag ) w->magasines--;
		}
		break;
	case 9:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	default:
		printLog("Error in process state for commando\n");
		break;
	}
}
void fn_f_commando (weapon* w, geBoolean mbnew)
{
	if( w->state == 9 )
	{
		soundsys_play_sound( &(w->theWeapon->weaponSound[2]), GE_FALSE );
		if(!cheats.unlimited_ammo) w->ammonution -= 1;
		SET_STATE(w, 1, 0.0f );
		weapon_flash();

		if( player_fire_gun(BULLET_TRACE_DISTANCE, 10.0f, &wIn, &wCol, w->theWeapon) )
		{
			bullet_hitObstacle(&wCol, DECALTYPE_BULLETBIG);
		}
	}
}
void fn_r_commando (weapon* w)
{
	if( w->magasines > 0 )
	if( w->state == 9 )
	{
		SET_STATE(w, 2, 0.0f );
		soundsys_play_sound( &(w->theWeapon->weaponSound[1]), GE_FALSE );
	}
}
void fn_x_commando (weapon* w)
{
}
void fn_a_commando (weapon* w)
{
	w->state = 0;
	w->stateTime = 0.0f;
}
void fn_n_commando (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	{
		game_message("click");
	}
}
void fn_s_commando (weapon* w, geBoolean mbnew)
{
}
void fn_y_commando (weapon* w )
{
}
geBoolean fn_c_commando (weapon* w)
{
	return ( w->state == 9 );
}
float fn_l_commando (weapon* w)
{
	return 0.0f;
}
//----------------------------



///////////////////////////////
// class: Fire/Flames
///////////////////////////////

//---------------------------
// weapon #1: Light flamethrower LFT 37
void fn_p_lft (weapon* w,geFloat t)
{
	w->stateTime += t;

	SET_POS( XForm.Translation );

	switch(w->state)
	{
	case 0:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 9, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[5]), GE_TRUE );
		}
		break;
	case 1:
		if( w->stateTime > 0.1f )
		{
			SET_STATE(w, 8, 0.0f );
		}
		break;
	case 2:
		if( w->stateTime > 3.0f )
		{
			game_message("Screw new amo");
			SET_STATE(w, 3, 0.0f );
		}
		break;
	case 3:
		if( w->stateTime > 3.0f )
		{
			SET_STATE(w, 9, 0.0f );
			w->ammonution = w->theWeapon->max_ammo;
			if( !cheats.unlimited_mag ) w->magasines--;
			game_message("Need a light");
		}
		break;
	case 9:
		if( w->stateTime > 0.24f )
		{
			SET_STATE(w, 9, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[5]), GE_TRUE );
		}
		break;
	case 8:
		if( w->stateTime > 0.2f )
		{
			SET_STATE(w, 9, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[5]), GE_TRUE );
			soundsys_stop( &(w->theWeapon->weaponSound[0]) );
		}
		break;
	default:
		printLog("Error in process state for lft\n");
		break;
	}
}
void fn_f_lft (weapon* w, geBoolean mbnew)
{
	if( w->ammonution )
	if( w->state == 9 || w->state == 8)
	{
		if(!cheats.unlimited_ammo) w->ammonution -= 1;
		SET_STATE(w, 1, 0.0f );
		soundsys_play_sound( &(w->theWeapon->weaponSound[0]), GE_TRUE );

		{
			geVec3d direction;
			geXForm3d_GetIn(&XForm, &direction);
			geVec3d_Scale(&direction, 2400.0f, &direction);
			fx_shootFire(XForm.Translation, direction);
		}
	}
}
void fn_r_lft (weapon* w)
{
	if( w->magasines > 0 )
	if( w->state == 9 )
	{
		game_message("Unscrew old ammo");
		SET_STATE(w, 2, 0.0f );
	}
}
void fn_x_lft (weapon* w)
{
}
void fn_a_lft (weapon* w)
{
	MAKE_LIGHT();
	SET_POS( XForm.Translation );
	w->state = 0;
	w->stateTime = 0.0f;
}
void fn_n_lft (weapon* w, geBoolean mbnew)
{
}
void fn_s_lft (weapon* w, geBoolean mbnew)
{
}
void fn_y_lft (weapon* w )
{
	DESTROY_LIGHT()
	system_message("deApply for lft");
	soundsys_stop( &(w->theWeapon->weaponSound[0]) );
	soundsys_stop( &(w->theWeapon->weaponSound[5]) );
}
geBoolean fn_c_lft (weapon* w)
{
	return ( w->state == 9 );
}
float fn_l_lft (weapon* w)
{
	return 0.0f;
}
//----------------------------


//---------------------------
// weapon #2: Heavy flame thrower
void fn_p_flamethrower (weapon* w,geFloat t)
{
{
	w->stateTime += t;

	SET_POS( XForm.Translation);

	switch(w->state)
	{
	case 0:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 9, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[5]), GE_FALSE );
		}
		break;
	case 1:
		if( w->stateTime > 0.1f )
		{
			SET_STATE(w, 8, 0.0f );
		}
		break;
	case 2:
		if( w->stateTime > 3.0f )
		{
			game_message("hook new ammo");
			SET_STATE(w, 3, 0.0f );
		}
		break;
	case 3:
		if( w->stateTime > 3.0f )
		{
			SET_STATE(w, 9, 0.0f );
			w->ammonution=255;
			if( !cheats.unlimited_mag ) w->magasines--;
			game_message("Burn baby burn");
		}
		break;
	case 9:
		if( w->stateTime > 2.0f )
		{
			SET_STATE(w, 9, 0.0f );
			soundsys_play_sound( &(w->theWeapon->weaponSound[5]), GE_FALSE );
		}
		break;
	case 8:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 9, 0.0f );
		}
	default:
		printLog("Error in process state for flamethrower\n");
		break;
	}
}
}
void fn_f_flamethrower (weapon* w, geBoolean mbnew)
{
	if( w->ammonution )
	if( w->state == 9 || w->state == 8 )
	{
		if(!cheats.unlimited_ammo) w->ammonution -= 1;
		SET_STATE(w, 1, 0.0f );
		soundsys_play_sound( &(w->theWeapon->weaponSound[0]), GE_FALSE );
	}
}
void fn_r_flamethrower (weapon* w)
{
	if( w->magasines > 0 )
	if( w->state == 9 )
	{
		game_message("Unhook old ammo");
		SET_STATE(w, 2, 0.0f );
	}
}
void fn_x_flamethrower (weapon* w)
{
}
void fn_a_flamethrower (weapon* w)
{
	w->state = 0;
	w->stateTime = 0.0f;
	MAKE_LIGHT();
	SET_POS( XForm.Translation);
}
void fn_n_flamethrower (weapon* w, geBoolean mbnew)
{
}
void fn_s_flamethrower (weapon* w, geBoolean mbnew)
{
}
void fn_y_flamethrower (weapon* w )
{
	DESTROY_LIGHT();
	system_message("deApply for flamethrower");
}
geBoolean fn_c_flamethrower (weapon* w)
{
	return ( w->state == 9 );
}
float fn_l_flamethrower (weapon* w)
{
	return 0.0f;
}
//----------------------------


//---------------------------
// weapon #3: Molotov cocktail
void fn_p_molotov (weapon* w,geFloat t)
{
	w->stateTime += t;

	SET_POS( XForm.Translation);

	switch(w->state)
	{
	case 0:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 2, 0.0f );
			game_message("Fire it up");
			MAKE_LIGHT();
			SET_POS( XForm.Translation);
		}
		break;
	case 1:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 6, 0.0f );
			game_message("Need another");
			DESTROY_LIGHT();
		}
		break;
	case 2:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 2, 0.0f );
		}
		break;
	case 3:
		if( w->stateTime > 1.0f )
		{
			SET_STATE(w, 4, 0.0f );
			game_message("Uuuh");
		}
		break;
	case 4:
		if( !(GetAsyncKeyState(controls.primaryFire) & 0x8000) || w->stateTime > 10.0f )
		{
			SET_STATE(w, 5, 0.0f );
			game_message("Huh");
		}
		break;
	case 5:
		if( w->stateTime > 0 )
		{
			SET_STATE(w, 1, 0.0f );
			game_message("Empty hands");
			w->ammonution = 0;
		}
		break;
	case 6:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 6, 0.0f );
		}
		break;
	default:
		printLog("Error in process state for molotov\n");
		break;
	}
}
void fn_f_molotov (weapon* w, geBoolean mbnew)
{
	if( w->state==2 )
	{
		SET_STATE(w, 3, 0.0f );
		soundsys_play_sound( &(w->theWeapon->weaponSound[0]), GE_FALSE);
	}
}
void fn_r_molotov (weapon* w)
{
	if( w->magasines > 0 )
	if( w->state == 6)
	{
		w->ammonution=1;
		if( !cheats.unlimited_mag ) w->magasines--;
		SET_STATE(w, 0, 0.0f );
	}
}
void fn_x_molotov (weapon* w)
{
}
void fn_a_molotov (weapon* w)
{
	if( w->ammonution )
		w->state = 0;
	else
		w->state = 1;
	w->stateTime = 0.0f;
}
void fn_n_molotov (weapon* w, geBoolean mbnew)
{
}
void fn_s_molotov (weapon* w, geBoolean mbnew)
{
}
void fn_y_molotov (weapon* w )
{
	DESTROY_LIGHT();
	system_message("deApply for molotov");
}
geBoolean fn_c_molotov (weapon* w)
{
	return (w->state==6 || w->state==2);
}
float fn_l_molotov (weapon* w)
{
	if( w->state == 4 )
	{
		return ( w->stateTime / 10.0f );
	}
	return 0.0f;
}
//----------------------------


//---------------------------
// weapon #4: Germal Signal pistol: Leuchtpistole 42
void fn_p_signal (weapon* w,geFloat t)
{
	w->stateTime += t;

	switch(w->state)
	{
	case 0:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 1, 0.0f );
		}
		break;
	case 1:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 1, 0.0f );
		}
		break;
	case 2:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 3, 0.0f );
			game_message("Ammo out");
		}
		break;
	case 3:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 4, 0.0f );
			game_message("Weapon is empty");
		}
		break;
	case 4:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 4, 0.0f );
		}
		break;
	case 5:
		if( w->stateTime > 1.0f )
		{
			SET_STATE(w, 6, 0.0f );
			game_message("Hit");
		}
		break;
	case 6:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 1, 0.0f );
			w->ammonution=1;
			if( !cheats.unlimited_mag ) w->magasines--;
			game_message("Kick in");
		}
		break;
	case 9:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 4, 0.0f );
		}
		break;
	default:
		printLog("Error in process state for signal\n");
		break;
	}
}
void fn_f_signal (weapon* w, geBoolean mbnew)
{
	//if( w->mbnew )
	if( w->state == 1 )
	{
		w->ammonution -= 1;
		SET_STATE(w, 2, 0.0f );
		game_message("Wang");
	}
}
void fn_r_signal (weapon* w)
{
	if( w->magasines > 0 )
	if( w->state == 4 )
	{
		SET_STATE(w, 5, 0.0f );
		game_message("Ammo in");
	}
}
void fn_x_signal (weapon* w)
{
}
void fn_a_signal (weapon* w)
{
	if( w->ammonution > 0 )
	{
		w->state = 0;
	}
	else
	{
		w->state = 9;
	}
	w->stateTime = 0.0f;
}
void fn_n_signal (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	{
		game_message("press");
	}
}
void fn_s_signal (weapon* w, geBoolean mbnew)
{
}
void fn_y_signal (weapon* w )
{
}
geBoolean fn_c_signal (weapon* w)
{
	return (w->state==1 || w->state==4);
}
float fn_l_signal (weapon* w)
{
	return 0.0f;
}
//----------------------------



///////////////////////////////
// class: Explosives
///////////////////////////////

//---------------------------
// weapon #1: m79 grenadelauncher
void fn_p_m79 (weapon* w,geFloat t)
{
	w->stateTime += t;

	switch(w->state)
	{
	case 0:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 1, 0.0f );
		}
		break;
	case 1:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 1, 0.0f );
		}
		break;
	case 2:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 3, 0.0f );
			game_message("Mag out");
		}
		break;
	case 3:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 4, 0.0f );
			game_message("Weapon is empty");
		}
		break;
	case 4:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 4, 0.0f );
		}
		break;
	case 5:
		if( w->stateTime > 1.0f )
		{
			SET_STATE(w, 6, 0.0f );
			game_message("Hit");
		}
		break;
	case 6:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 1, 0.0f );
			w->ammonution=1;
			if( !cheats.unlimited_mag ) w->magasines--;
			game_message("Kick in");
		}
		break;
	case 9:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 4, 0.0f );
		}
		break;
	default:
		printLog("Error in process state for m79\n");
		break;
	}
}
void fn_f_m79 (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	if( w->state == 1 )
	{
		w->ammonution -= 1;
		SET_STATE(w, 2, 0.0f );
		//game_message("Whoosh");
		soundsys_play_sound( &(w->theWeapon->weaponSound[0]) , GE_FALSE);

		// @@@ temporary code until we can actully shoot m79 grenades
		{
			geVec3d velocity;
			geXForm3d_GetIn(&XForm, &velocity);
			geVec3d_Scale(&velocity, 5000.0f, &velocity);
			weapon_grenade_throw(velocity, XForm.Translation, 3.0f);
		}
	}
}
void fn_r_m79 (weapon* w)
{
	if( w->magasines > 0 )
	if( w->state == 4 )
	{
		SET_STATE(w, 5, 0.0f );
		game_message("Ammo in");
	}
}
void fn_x_m79 (weapon* w)
{
}
void fn_a_m79 (weapon* w)
{
	if( w->ammonution > 0 )
	{
		w->state = 0;
	}
	else
	{
		w->state = 9;
	}
	w->stateTime = 0.0f;
}
void fn_n_m79 (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	{
		game_message("press");
	}
}
void fn_s_m79 (weapon* w, geBoolean mbnew)
{
}
void fn_y_m79 (weapon* w )
{
}
geBoolean fn_c_m79 (weapon* w)
{
	return (w->state==1 || w->state==4);
}
float fn_l_m79 (weapon* w)
{
	return 0.0f;
}
//----------------------------


//---------------------------
// weapon #2: l2a2 grenade
void fn_p_grenade (weapon* w,geFloat t)
{
	w->stateTime += t;

	switch(w->state)
	{
	case 0:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 2, 0.0f );
		}
		break;
	case 1:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 6, 0.0f );
		}
		break;
	case 2:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 2, 0.0f );
		}
		break;
	case 3:
		if( w->stateTime > 1.0f )
		{
			SET_STATE(w, 4, 0.0f );
			game_message("Uuuh");
		}
		break;
	case 4:
		if( !(GetAsyncKeyState(controls.primaryFire) & 0x8000) || w->stateTime > 1.0f )
		{
			{
				geVec3d velocity;
				geXForm3d_GetIn(&XForm, &velocity);
				geVec3d_Scale(&velocity, w->stateTime * 1000.0f, &velocity);
				weapon_grenade_throw(velocity, XForm.Translation, 3.0f );
			}
			SET_STATE(w, 5, 0.0f );
			game_message("Huh");
			w->ammonution = 0;
		}
		break;
	case 5:
		if( w->stateTime > 0.4f )
		{
			SET_STATE(w, 6, 0.0f );
			game_message("Empty hands");
		}
		break;
	case 6:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 6, 0.0f );
		}
		break;
	default:
		printLog("Error in process state for grenade\n");
		break;
	}
}
void fn_f_grenade (weapon* w, geBoolean mbnew)
{
	if( w->state==2 )
	{
		SET_STATE(w, 3, 0.0f );
		soundsys_play_sound( &(w->theWeapon->weaponSound[0]) , GE_FALSE);
	}
}
void fn_r_grenade (weapon* w)
{
	if( w->state == 6)
	{
		w->ammonution=1;
		if( !cheats.unlimited_mag ) w->magasines--;
		SET_STATE(w, 2, 0.0f );
	}
}
void fn_x_grenade (weapon* w)
{
}
void fn_a_grenade (weapon* w)
{
	if( w->ammonution )
		w->state = 0;
	else
		w->state = 1;
	w->stateTime = 0.0f;
}
void fn_n_grenade (weapon* w, geBoolean mbnew)
{
}
void fn_s_grenade (weapon* w, geBoolean mbnew)
{
}
void fn_y_grenade (weapon* w)
{
}
geBoolean fn_c_grenade (weapon* w)
{
	return (w->state==2 || w->state==6);
}
float fn_l_grenade (weapon* w)
{
	if( w->state == 4 )
	{
		return ( w->stateTime / 1.0f );
	}

	return 0.0f;
}
//----------------------------


//---------------------------
// weapon #3: c4
void fn_p_c4 (weapon* w,geFloat t)
{
	w->stateTime += t;

	switch(w->state)
	{
	case 0:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	case 1:
		if( w->stateTime > 0.4f )
		{
			SET_STATE(w, 9, 0.0f );
			if(!cheats.unlimited_ammo) w->ammonution -= 1;
		}
		break;
	case 2:
		if( w->stateTime > 0.4f )
		{
			SET_STATE(w, 9, 0.0f );
			w->ammonution=1;
			if( !cheats.unlimited_mag ) w->magasines--;
		}
		break;
	case 3:
		if( w->stateTime > 0.4f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	case 9:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	
	default:
		printLog("Error in process state for c4\n");
		break;
	}
}
void fn_f_c4 (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	if( w->state == 9 )
	{
		game_message("Place");
		SET_STATE(w, 1, 0.0f );
	}
}
void fn_r_c4 (weapon* w)
{
	if( w->magasines > 0 )
	if(w->state == 9 )
	{
		game_message("Grabbing new");
		SET_STATE(w, 2, 0.0f );
	}
}
void fn_x_c4 (weapon* w)
{
}
void fn_a_c4 (weapon* w)
{
	w->state = 0;
	w->stateTime = 0.0f;
}
void fn_n_c4 (weapon* w, geBoolean mbnew)
{
}
void fn_s_c4 (weapon* w, geBoolean mbnew)
{
	if( mbnew )
	if( w->state==9 )
	{
		SET_STATE(w, 3, 0.0f );
		game_message("Biip");
	}
}
void fn_y_c4 (weapon* w )
{
}
geBoolean fn_c_c4 (weapon* w)
{
	return (w->state == 9);
}
float fn_l_c4 (weapon* w)
{
	return 0.0f;
}
//----------------------------


//---------------------------
// weapon #4: PMA-2 plastic personal mine
void fn_p_mine (weapon* w,geFloat t)
{
	w->stateTime += t;

	switch(w->state)
	{
	case 0:
		if( w->stateTime > 0.3f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	case 1:
		if( w->stateTime > 0.4f )
		{
			SET_STATE(w, 2, 0.0f );
			game_message("empty hands");
		}
		break;
	case 2:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 2, 0.0f );
		}
		break;
	case 3:
		if( w->stateTime > 0.4 )
		{
			SET_STATE(w, 9, 0.0f );
			game_message("Ready for r n' r");
			w->ammonution = 1;
			if( !cheats.unlimited_mag ) w->magasines--;
		}
		break;
	case 4:
		if( w->stateTime > 0.3 )
		{
			SET_STATE(w, 2, 0.0f );
		}
		break;
	case 9:
		if( w->stateTime > 10.0f )
		{
			SET_STATE(w, 9, 0.0f );
		}
		break;
	default:
		printLog("Error in process state for mine\n");
		break;
	}
}
void fn_f_mine (weapon* w, geBoolean mbnew)
{
	if( w->state == 9 )
	{
		game_message("Placing");
		SET_STATE(w, 1, 0.0f );
		w->ammonution -= 1;
	}
}
void fn_r_mine (weapon* w)
{
	//game_message("abc");
	if( w->magasines > 0 )
	{
		//game_message("def");
		if( w->state == 2 )
		{
			SET_STATE(w, 3, 0.0f );
			game_message("Grabbing new");
		}
	}
}
void fn_x_mine (weapon* w)
{
}
void fn_a_mine (weapon* w)
{
	if( w->ammonution )
	{
		w->state = 0;
	}
	else
	{
		w->state = 4;
	}
	w->stateTime = 0.0f;
}
void fn_n_mine (weapon* w, geBoolean mbnew)
{
}
void fn_s_mine (weapon* w, geBoolean mbnew)
{
}
void fn_y_mine (weapon* w )
{
}
geBoolean fn_c_mine (weapon* w)
{
	return (w->state==2 || w->state==9);
}
float fn_l_mine (weapon* w)
{
	return 0.0f;
}
//----------------------------





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		apply_weapon(internal function)
//			updates the current weapon(current_weapon variable)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void apply_weapon()
{
	if( current_weapon )
	{
		if( current_weapon->theWeapon->fn_changeable(current_weapon) )
		{
			//game_message("Changing weapon approved");
		}
		else
		{
			game_message("Changing weapon denied");
			return;
		}

		current_weapon->theWeapon->fn_deApply(current_weapon);
	}
	current_weapon = & (weapons[selClass][selWeapon] );

	if( current_weapon )
	{
		char doit = 1;

		if( current_weapon->number_of_weapons <= 0 )
		{
			selClass = WEAPON_C_COMBATS;
			selWeapon = 0;
			current_weapon = & (weapons[selClass][selWeapon] );
			

			if( !current_weapon)
			{
				system_message("WARNING: current_weapon is null when changing weapon to default hand");
				doit = 0;
			}
		}

		if(doit)
		{
			current_weapon->theWeapon->fn_applyWeapon(current_weapon);
			currClass = selClass;
			currWeapon = selWeapon;
		}
	}
	else
	{
		system_message("WARNING: current_weapon is null, in apply_weapon()");
	}

	disable_selector();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void weapon_apply()
{
	apply_weapon();
}

void weapon_deapply()
{
	if( current_weapon )
	{
		if( current_weapon->theWeapon->fn_changeable(current_weapon) )
		{
			//game_message("Changing weapon approved");
		}
		else
		{
			game_message("Changing weapon denied");
			return;
		}

		current_weapon->theWeapon->fn_deApply(current_weapon);
		current_weapon = 0;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		enable_selector(internal function)
//			enables the showing of the graphical 2d wiev of the weapons
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void enable_selector()
{
	open_sec = WEAPON_OPEN_SEC;
	set_y_shift(18);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////









///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		disable_selector(internal function)
//			disable the showing of the graphical 2d wiev of the weapons
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void disable_selector()
{
	open_sec = 0;
	set_y_shift(0);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////













///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		weapon_fire
//			fires the current selected weapon
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void weapon_fire( geBoolean mbnew)
{
	if( open_sec > 0 )
	{
		apply_weapon();
		return;
	}

	if( current_weapon)
	{
		char doit = 0;

		//current_weapon->mbnew = mbnew;

		if( current_weapon->theWeapon->unlimited_ammo )
		{
			doit = 1;
		}
		else
		{
			if( current_weapon->ammonution != 0)
			{
				doit = 1;
			}
			else
			{
				//game_message("You need to reload your weapon");
				current_weapon->theWeapon->fn_noAmmo(current_weapon, mbnew);
			}
		}

		if( doit )
		{
			char really=1;
			if( player_getState() == STATE_WATER )
			{
				if( !weapon_water() ) really = 0;

			}
			if( really ) current_weapon->theWeapon->fn_fireWeapon(current_weapon, mbnew);
		}
	}
	else
	{
		system_message("WARNING: Current weapon is null");
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void weapon_newWorld(geWorld* world){
	weapon_grenade_newWorld(world);
}

/*
#define DAMAGE_ARMORPIERCING	1
#define DAMAGE_IMPALING			2
#define DAMAGE_CRUSHING			3
#define DAMAGE_NORMAL			4
#define DAMAGE_ELECTRICAL		4
#define DAMAGE_TRANQUALIXER		4
*/

int setup_weaponTypes(){
	int i=0;
	int b = 1;
	WeaponType* wt;

	//--------------------------------------------------------------------------------------------------------------------
	//Closecombat
	//--------------------------------------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------------------------------------
	//Hand to hand - unarmed
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_TRUE;
	wt->max_ammo = 0;
	sprintf(wt->name, "Hand to hand");
	wt->unlimited_ammo = GE_TRUE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\hand.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\hand.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\hand.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_TRUE;
	wt->mag = 0;
	wt->mod_acc = 1.0f;
	wt->damage = 1;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_FALSE;
	
	wt->fn_processState	=	fn_p_hand ;
	wt->fn_fireWeapon	=	fn_f_hand ;
	wt->fn_reloadWeapon =	fn_r_hand ;
	wt->fn_fxMotion		=	fn_x_hand ;
	wt->fn_applyWeapon	=	fn_a_hand ;
	wt->fn_noAmmo		=	fn_n_hand ;
	wt->fn_secondFire	=	fn_s_hand ;
	wt->fn_deApply		=	fn_y_hand ;
	wt->fn_changeable	=	fn_c_hand ;
	wt->fn_renderLine	=	fn_l_hand ;

	soundsys_loadWaw(".\\sfx\\weapon\\hands\\fire.wav", &(wt->weaponSound[1]) );
	soundsys_loadWaw(".\\sfx\\weapon\\hands\\fire2.wav", &(wt->weaponSound[2]) );
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Knife
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_TRUE;
	wt->max_ammo = 0;
	sprintf(wt->name, "Ka-Bar fighting knife");
	wt->unlimited_ammo = GE_TRUE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\knife.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\knife.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\knife.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_TRUE;
	wt->mag = 0;
	wt->mod_acc = 1.0f;
	wt->damage = 5;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_FALSE;
	
	wt->fn_processState	=	fn_p_knife ;
	wt->fn_fireWeapon	=	fn_f_knife ;
	wt->fn_reloadWeapon =	fn_r_knife ;
	wt->fn_fxMotion		=	fn_x_knife ;
	wt->fn_applyWeapon	=	fn_a_knife ;
	wt->fn_noAmmo		=	fn_n_knife ;
	wt->fn_secondFire	=	fn_s_knife ;
	wt->fn_deApply		=	fn_y_knife ;
	wt->fn_changeable	=	fn_c_knife ;
	wt->fn_renderLine	=	fn_l_knife ;

	soundsys_loadWaw(".\\sfx\\weapon\\knife\\fire.wav", &(wt->weaponSound[1]) );
	soundsys_loadWaw(".\\sfx\\weapon\\knife\\fire2.wav", &(wt->weaponSound[2]) );
	soundsys_loadWaw(".\\sfx\\weapon\\knife\\fire3.wav", &(wt->weaponSound[3]) );
	soundsys_loadWaw(".\\sfx\\weapon\\knife\\fire4.wav", &(wt->weaponSound[4]) );
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Hammer
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_TRUE;
	wt->max_ammo = 0;
	sprintf(wt->name, "10lb sledge hammer");
	wt->unlimited_ammo = GE_TRUE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\hammer.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\hammer.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\hammer.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_TRUE;
	wt->mag = 0;
	wt->mod_acc = 1.0f;
	wt->damage = 15;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_FALSE;
	
	wt->fn_processState	=	fn_p_hammer ;
	wt->fn_fireWeapon	=	fn_f_hammer ;
	wt->fn_reloadWeapon =	fn_r_hammer ;
	wt->fn_fxMotion		=	fn_x_hammer ;
	wt->fn_applyWeapon	=	fn_a_hammer ;
	wt->fn_noAmmo		=	fn_n_hammer ;
	wt->fn_secondFire	=	fn_s_hammer ;
	wt->fn_deApply		=	fn_y_hammer ;
	wt->fn_changeable	=	fn_c_hammer ;
	wt->fn_renderLine	=	fn_l_hammer ;

	soundsys_loadWaw(".\\sfx\\weapon\\hammer\\fire.wav", &(wt->weaponSound[1]) );
	soundsys_loadWaw(".\\sfx\\weapon\\hammer\\fire2.wav", &(wt->weaponSound[2]) );
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Axe
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_TRUE;
	wt->max_ammo = 0;
	sprintf(wt->name, "Flathead axe");
	wt->unlimited_ammo = GE_TRUE;
	//wt->sound = load_sound(".\\sound\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\axe.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\axe.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\axe.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_TRUE;
	wt->mag = 0;
	wt->mod_acc = 1.0f;
	wt->damage = 10;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_FALSE;
	
	wt->fn_processState	=	fn_p_axe ;
	wt->fn_fireWeapon	=	fn_f_axe ;
	wt->fn_reloadWeapon =	fn_r_axe ;
	wt->fn_fxMotion		=	fn_x_axe ;
	wt->fn_applyWeapon	=	fn_a_axe ;
	wt->fn_noAmmo		=	fn_n_axe ;
	wt->fn_secondFire	=	fn_s_axe ;
	wt->fn_deApply		=	fn_y_axe ;
	wt->fn_changeable	=	fn_c_axe ;
	wt->fn_renderLine	=	fn_l_axe ;

	soundsys_loadWaw(".\\sfx\\weapon\\axe\\fire.wav", &(wt->weaponSound[1]) );
	soundsys_loadWaw(".\\sfx\\weapon\\axe\\fire2.wav", &(wt->weaponSound[2]) );
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//PISTOLS
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//9mm
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_FALSE;
	wt->max_ammo = 6; //10, 17, 19, 31
	sprintf(wt->name, "Glock 17 9mm");
	wt->unlimited_ammo = GE_FALSE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\9mm.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\9mm.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\9mm.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_FALSE;
	wt->mag = 30;
	wt->mod_acc = 200.0f;
	wt->damage = 10;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_FALSE;
	
	wt->fn_processState	=	fn_p_9mm ;
	wt->fn_fireWeapon	=	fn_f_9mm ;
	wt->fn_reloadWeapon =	fn_r_9mm ;
	wt->fn_fxMotion		=	fn_x_9mm ;
	wt->fn_applyWeapon	=	fn_a_9mm ;
	wt->fn_noAmmo		=	fn_n_9mm ;
	wt->fn_secondFire	=	fn_s_9mm ;
	wt->fn_deApply		=	fn_y_9mm ;
	wt->fn_changeable	=	fn_c_9mm ;
	wt->fn_renderLine	=	fn_l_9mm ;


	soundsys_loadWaw(".\\sfx\\weapon\\9mm\\fire.wav", &(wt->weaponSound[1]) );
	soundsys_loadWaw(".\\sfx\\weapon\\9mm\\clipin.wav", &(wt->weaponSound[2]) );
	soundsys_loadWaw(".\\sfx\\weapon\\9mm\\clipout.wav", &(wt->weaponSound[3]) );
	soundsys_loadWaw(".\\sfx\\weapon\\9mm\\click.wav", &(wt->weaponSound[4]) );
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Desert eagle
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_FALSE;
	wt->max_ammo = 9;//9, 8, 7
	sprintf(wt->name, "Desert Eagle .50");
	wt->unlimited_ammo = GE_FALSE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\desert_eagle.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\desert_eagle.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\desert_eagle.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_FALSE;
	wt->mag = 30;
	wt->mod_acc = 200.0f;
	wt->damage = 19;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_TRUE;
	
	wt->fn_processState	=	fn_p_eagle ;
	wt->fn_fireWeapon	=	fn_f_eagle ;
	wt->fn_reloadWeapon =	fn_r_eagle ;
	wt->fn_fxMotion		=	fn_x_eagle ;
	wt->fn_applyWeapon	=	fn_a_eagle ;
	wt->fn_noAmmo		=	fn_n_eagle ;
	wt->fn_secondFire	=	fn_s_eagle ;
	wt->fn_deApply		=	fn_y_eagle ;
	wt->fn_changeable	=	fn_c_eagle ;
	wt->fn_renderLine	=	fn_l_eagle ;

	soundsys_loadWaw(".\\sfx\\weapon\\deagle\\fire.wav", &(wt->weaponSound[1]) );
	soundsys_loadWaw(".\\sfx\\weapon\\deagle\\clipin.wav", &(wt->weaponSound[2]) );
	soundsys_loadWaw(".\\sfx\\weapon\\deagle\\clipout.wav", &(wt->weaponSound[3]) );
	soundsys_loadWaw(".\\sfx\\weapon\\deagle\\click.wav", &(wt->weaponSound[4]) );
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Improved tazer
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_FALSE;
	wt->max_ammo = 10;
	sprintf(wt->name, "Improved tazer");
	wt->unlimited_ammo = GE_FALSE;
	//wt->sound = load_sound(".//sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\improved_tazer.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\improved_tazer.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\improved_tazer.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_FALSE;
	wt->mag = 12;
	wt->mod_acc = 200.0f;
	wt->damage = 10;
	wt->damageType = DAMAGE_ELECTRICAL;
	wt->ap = GE_TRUE;
	
	wt->fn_processState	=	fn_p_tazer ;
	wt->fn_fireWeapon	=	fn_f_tazer ;
	wt->fn_reloadWeapon =	fn_r_tazer ;
	wt->fn_fxMotion		=	fn_x_tazer ;
	wt->fn_applyWeapon	=	fn_a_tazer ;
	wt->fn_noAmmo		=	fn_n_tazer ;
	wt->fn_secondFire	=	fn_s_tazer ;
	wt->fn_deApply		=	fn_y_tazer ;
	wt->fn_changeable	=	fn_c_tazer ;
	wt->fn_renderLine	=	fn_l_tazer ;

	soundsys_loadWaw(".\\sfx\\weapon\\tazer\\beep.wav", &(wt->weaponSound[0]) );
	soundsys_loadWaw(".\\sfx\\weapon\\tazer\\fire.wav", &(wt->weaponSound[1]) );
	soundsys_loadWaw(".\\sfx\\weapon\\tazer\\reload.wav", &(wt->weaponSound[2]) );
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Improved tranqualizer
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_FALSE;
	wt->max_ammo = 1;
	sprintf(wt->name, "Improved Tranqualizer");
	wt->unlimited_ammo = GE_FALSE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\improved_tranqualizer.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\improved_tranqualizer.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\improved_tranqualizer.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_FALSE;
	wt->mag = 30;
	wt->mod_acc = 200.0f;
	wt->damage = 10;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_FALSE;
	
	wt->fn_processState	=	fn_p_tranqualizer ;
	wt->fn_fireWeapon	=	fn_f_tranqualizer ;
	wt->fn_reloadWeapon =	fn_r_tranqualizer ;
	wt->fn_fxMotion		=	fn_x_tranqualizer ;
	wt->fn_applyWeapon	=	fn_a_tranqualizer ;
	wt->fn_noAmmo		=	fn_n_tranqualizer ;
	wt->fn_secondFire	=	fn_s_tranqualizer ;
	wt->fn_deApply		=	fn_y_tranqualizer ;
	wt->fn_changeable	=	fn_c_tranqualizer ;
	wt->fn_renderLine	=	fn_l_tranqualizer ;
	//--------------------------------------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------------------------------------
	//RIFLES
	//--------------------------------------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------------------------------------
	//Pump-Shotgun
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_FALSE;
	wt->max_ammo = 4;
	sprintf(wt->name, "Itacha 37 \"Homeland security\"");
	wt->unlimited_ammo = GE_FALSE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\pumpaction_shotgun.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\pumpaction_shotgun.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\pumpaction_shotgun.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_FALSE;
	wt->mag = 50;
	wt->mod_acc = 200.0f;
	wt->damage = 17;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_FALSE;
	
	wt->fn_processState	=	fn_p_shotgun ;
	wt->fn_fireWeapon	=	fn_f_shotgun ;
	wt->fn_reloadWeapon =	fn_r_shotgun ;
	wt->fn_fxMotion		=	fn_x_shotgun ;
	wt->fn_applyWeapon	=	fn_a_shotgun ;
	wt->fn_noAmmo		=	fn_n_shotgun ;
	wt->fn_secondFire	=	fn_s_shotgun ;
	wt->fn_deApply		=	fn_y_shotgun ;
	wt->fn_changeable	=	fn_c_shotgun ;
	wt->fn_renderLine	=	fn_l_shotgun ;

	soundsys_loadWaw(".\\sfx\\weapon\\shotgun\\fire.wav", &(wt->weaponSound[1]) );
	soundsys_loadWaw(".\\sfx\\weapon\\shotgun\\bulletin.wav", &(wt->weaponSound[2]) );
	soundsys_loadWaw(".\\sfx\\weapon\\shotgun\\chakka.wav", &(wt->weaponSound[3]) );
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Mag 7
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_TRUE;
	wt->max_ammo = 5;
	sprintf(wt->name, "Mag7");
	wt->unlimited_ammo = GE_FALSE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\mag7.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\mag7.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\mag7.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_FALSE;
	wt->mag = 25;
	wt->mod_acc = 200.0f;
	wt->damage = 13;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_FALSE;
	
	wt->fn_processState	=	fn_p_mag7 ;
	wt->fn_fireWeapon	=	fn_f_mag7 ;
	wt->fn_reloadWeapon =	fn_r_mag7 ;
	wt->fn_fxMotion		=	fn_x_mag7 ;
	wt->fn_applyWeapon	=	fn_a_mag7 ;
	wt->fn_noAmmo		=	fn_n_mag7 ;
	wt->fn_secondFire	=	fn_s_mag7 ;
	wt->fn_deApply		=	fn_y_mag7 ;
	wt->fn_changeable	=	fn_c_mag7 ;
	wt->fn_renderLine	=	fn_l_mag7 ;

	soundsys_loadWaw(".\\sfx\\weapon\\mag7\\fire.wav", &(wt->weaponSound[1]) );
	soundsys_loadWaw(".\\sfx\\weapon\\mag7\\chakka.wav", &(wt->weaponSound[2]) );
	soundsys_loadWaw(".\\sfx\\weapon\\mag7\\clipin.wav", &(wt->weaponSound[3]) );
	soundsys_loadWaw(".\\sfx\\weapon\\mag7\\clipout.wav", &(wt->weaponSound[4]) );
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Sniper rifle
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_FALSE;
	wt->max_ammo = 5;
	sprintf(wt->name, "Beretta 501 Sniper");
	wt->unlimited_ammo = GE_FALSE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\sniper_rifle.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\sniper_rifle.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\sniper_rifle.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_FALSE;
	wt->mag = 5;
	wt->mod_acc = 200.0f;
	wt->damage = 30;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_FALSE;
	
	wt->fn_processState	=	fn_p_sniper ;
	wt->fn_fireWeapon	=	fn_f_sniper ;
	wt->fn_reloadWeapon =	fn_r_sniper ;
	wt->fn_fxMotion		=	fn_x_sniper ;
	wt->fn_applyWeapon	=	fn_a_sniper ;
	wt->fn_noAmmo		=	fn_n_sniper ;
	wt->fn_secondFire	=	fn_s_sniper ;
	wt->fn_deApply		=	fn_y_sniper ;
	wt->fn_changeable	=	fn_c_sniper ;
	wt->fn_renderLine	=	fn_l_sniper ;

	soundsys_loadWaw(".\\sfx\\weapon\\sniper\\zoom.wav", &(wt->weaponSound[0]) );
	soundsys_loadWaw(".\\sfx\\weapon\\sniper\\fire.wav", &(wt->weaponSound[1]) );
	soundsys_loadWaw(".\\sfx\\weapon\\sniper\\clipin.wav", &(wt->weaponSound[2]) );
	soundsys_loadWaw(".\\sfx\\weapon\\sniper\\clipout.wav", &(wt->weaponSound[3]) );
	soundsys_loadWaw(".\\sfx\\weapon\\sniper\\bolt.wav", &(wt->weaponSound[4]) );
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Improved sniper rifle
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_TRUE;
	wt->max_ammo = 10;
	sprintf(wt->name, "Barret \"Light fiffty\" M82A1");
	wt->unlimited_ammo = GE_FALSE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\improved_sniper_rifle.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\improved_sniper_rifle.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\improved_sniper_rifle.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_FALSE;
	wt->mag = 5;
	wt->mod_acc = 200.0f;
	wt->damage = 40;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_TRUE;
	
	wt->fn_processState	=	fn_p_barett ;
	wt->fn_fireWeapon	=	fn_f_barett ;
	wt->fn_reloadWeapon =	fn_r_barett ;
	wt->fn_fxMotion		=	fn_x_barett ;
	wt->fn_applyWeapon	=	fn_a_barett ;
	wt->fn_noAmmo		=	fn_n_barett ;
	wt->fn_secondFire	=	fn_s_barett ;
	wt->fn_deApply		=	fn_y_barett ;
	wt->fn_changeable	=	fn_c_barett ;
	wt->fn_renderLine	=	fn_l_barett ;

	soundsys_loadWaw(".\\sfx\\weapon\\barett\\zoom.wav", &(wt->weaponSound[0]) );
	soundsys_loadWaw(".\\sfx\\weapon\\barett\\fire.wav", &(wt->weaponSound[1]) );
	soundsys_loadWaw(".\\sfx\\weapon\\barett\\clipin.wav", &(wt->weaponSound[2]) );
	soundsys_loadWaw(".\\sfx\\weapon\\barett\\clipout.wav", &(wt->weaponSound[3]) );
	soundsys_loadWaw(".\\sfx\\weapon\\barett\\boltpull.wav", &(wt->weaponSound[4]) );
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Automatics
	//--------------------------------------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------------------------------------
	//smg
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_TRUE;
	wt->max_ammo = 30;
	sprintf(wt->name, "smg hk mp-5k a4");
	wt->unlimited_ammo = GE_FALSE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\smg.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\smg.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\smg.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_FALSE;
	wt->mag = 30;
	wt->mod_acc = 200.0f;
	wt->damage = 12;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_FALSE;
	
	wt->fn_processState	=	fn_p_smg ;
	wt->fn_fireWeapon	=	fn_f_smg ;
	wt->fn_reloadWeapon =	fn_r_smg ;
	wt->fn_fxMotion		=	fn_x_smg ;
	wt->fn_applyWeapon	=	fn_a_smg ;
	wt->fn_noAmmo		=	fn_n_smg ;
	wt->fn_secondFire	=	fn_s_smg ;
	wt->fn_deApply		=	fn_y_smg ;
	wt->fn_changeable	=	fn_c_smg ;
	wt->fn_renderLine	=	fn_l_smg ;

	soundsys_loadWaw(".\\sfx\\weapon\\smg\\fire.wav", &(wt->weaponSound[0]) );
	soundsys_loadWaw(".\\sfx\\weapon\\smg\\clipin.wav", &(wt->weaponSound[1]) );
	soundsys_loadWaw(".\\sfx\\weapon\\smg\\clipout.wav", &(wt->weaponSound[2]) );
	soundsys_loadWaw(".\\sfx\\weapon\\smg\\boltslap.wav", &(wt->weaponSound[3]) );
	soundsys_loadWaw(".\\sfx\\weapon\\smg\\boltpull.wav", &(wt->weaponSound[4]) );
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Uzi
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_TRUE;
	wt->max_ammo = 32;
	sprintf(wt->name, "Uzi 9mm");
	wt->unlimited_ammo = GE_FALSE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\uzi.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\uzi.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\uzi.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_FALSE;
	wt->mag = 25;
	wt->mod_acc = 200.0f;
	wt->damage = 10;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_FALSE;
	
	wt->fn_processState	=	fn_p_uzi ;
	wt->fn_fireWeapon	=	fn_f_uzi ;
	wt->fn_reloadWeapon =	fn_r_uzi ;
	wt->fn_fxMotion		=	fn_x_uzi ;
	wt->fn_applyWeapon	=	fn_a_uzi ;
	wt->fn_noAmmo		=	fn_n_uzi ;
	wt->fn_secondFire	=	fn_s_uzi ;
	wt->fn_deApply		=	fn_y_uzi ;
	wt->fn_changeable	=	fn_c_uzi ;
	wt->fn_renderLine	=	fn_l_uzi ;

	soundsys_loadWaw(".\\sfx\\weapon\\uzi\\fire.wav", &(wt->weaponSound[0]) );
	soundsys_loadWaw(".\\sfx\\weapon\\uzi\\clipin.wav", &(wt->weaponSound[1]) );
	soundsys_loadWaw(".\\sfx\\weapon\\uzi\\clipout.wav", &(wt->weaponSound[2]) );
	soundsys_loadWaw(".\\sfx\\weapon\\uzi\\slidepull.wav", &(wt->weaponSound[3]) );
	soundsys_loadWaw(".\\sfx\\weapon\\uzi\\empty.wav", &(wt->weaponSound[4]) );
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Assult rifle
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_TRUE;
	wt->max_ammo = 30;
	sprintf(wt->name, "Ak47");
	wt->unlimited_ammo = GE_FALSE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\ak47.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\ak47.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\ak47.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_FALSE;
	wt->mag = 10;
	wt->mod_acc = 200.0f;
	wt->damage = 17;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_TRUE;
	
	wt->fn_processState	=	fn_p_ak47 ;
	wt->fn_fireWeapon	=	fn_f_ak47 ;
	wt->fn_reloadWeapon =	fn_r_ak47 ;
	wt->fn_fxMotion		=	fn_x_ak47 ;
	wt->fn_applyWeapon	=	fn_a_ak47 ;
	wt->fn_noAmmo		=	fn_n_ak47 ;
	wt->fn_secondFire	=	fn_s_ak47 ;
	wt->fn_deApply		=	fn_y_ak47 ;
	wt->fn_changeable	=	fn_c_ak47 ;
	wt->fn_renderLine	=	fn_l_ak47 ;

	soundsys_loadWaw(".\\sfx\\weapon\\ak47\\fire.wav", &(wt->weaponSound[0]) );
	soundsys_loadWaw(".\\sfx\\weapon\\ak47\\clipin.wav", &(wt->weaponSound[1]) );
	soundsys_loadWaw(".\\sfx\\weapon\\ak47\\clipout.wav", &(wt->weaponSound[2]) );
	soundsys_loadWaw(".\\sfx\\weapon\\ak47\\slideback.wav", &(wt->weaponSound[3]) );
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Advanced assult rifle
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_TRUE;
	wt->max_ammo = 60;
	sprintf(wt->name, "Colt commando 733");
	wt->unlimited_ammo = GE_FALSE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\commando.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\commando.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\commando.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_FALSE;
	wt->mag = 10;
	wt->mod_acc = 200.0f;
	wt->damage = 15;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_TRUE;
	
	wt->fn_processState	=	fn_p_commando ;
	wt->fn_fireWeapon	=	fn_f_commando ;
	wt->fn_reloadWeapon =	fn_r_commando ;
	wt->fn_fxMotion		=	fn_x_commando ;
	wt->fn_applyWeapon	=	fn_a_commando ;
	wt->fn_noAmmo		=	fn_n_commando ;
	wt->fn_secondFire	=	fn_s_commando ;
	wt->fn_deApply		=	fn_y_commando ;
	wt->fn_changeable	=	fn_c_commando ;
	wt->fn_renderLine	=	fn_l_commando ;

	soundsys_loadWaw(".\\sfx\\weapon\\commando\\clipin.wav", &(wt->weaponSound[0]) );
	soundsys_loadWaw(".\\sfx\\weapon\\commando\\clipout.wav", &(wt->weaponSound[1]) );
	soundsys_loadWaw(".\\sfx\\weapon\\commando\\fire.wav", &(wt->weaponSound[2]) );
	soundsys_loadWaw(".\\sfx\\weapon\\commando\\boltpull.wav", &(wt->weaponSound[3]) );
	//--------------------------------------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------------------------------------
	//FLAMES
	//--------------------------------------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------------------------------------
	//Flamethrower
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_TRUE;
	wt->max_ammo = 150;
	sprintf(wt->name, "Light Flamethrower");
	wt->unlimited_ammo = GE_FALSE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\light_flamethrower.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\light_flamethrower.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\light_flamethrower.bmp");
	wt->showBar = GE_TRUE;
	wt->inWater = GE_FALSE;
	wt->mag = 5;
	wt->mod_acc = 200.0f;
	wt->damage = 10;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_FALSE;
	
	wt->fn_processState	=	fn_p_lft ;
	wt->fn_fireWeapon	=	fn_f_lft ;
	wt->fn_reloadWeapon =	fn_r_lft ;
	wt->fn_fxMotion		=	fn_x_lft ;
	wt->fn_applyWeapon	=	fn_a_lft ;
	wt->fn_noAmmo		=	fn_n_lft ;
	wt->fn_secondFire	=	fn_s_lft ;
	wt->fn_deApply		=	fn_y_lft ;
	wt->fn_changeable	=	fn_c_lft ;
	wt->fn_renderLine	=	fn_l_lft ;

	soundsys_loadWaw(".\\sfx\\weapon\\lft\\fire.wav", &(wt->weaponSound[0]) );
	soundsys_loadWaw(".\\sfx\\weapon\\lft\\idle.wav", &(wt->weaponSound[5]) );
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Improved flamethrower
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_TRUE;
	wt->max_ammo = 255;
	sprintf(wt->name, "Heavy flamethrower");
	wt->unlimited_ammo = GE_FALSE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\heavy_flamethrower.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\heavy_flamethrower.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\heavy_flamethrower.bmp");
	wt->showBar = GE_TRUE;
	wt->inWater = GE_FALSE;
	wt->mag = 2;
	wt->mod_acc = 200.0f;
	wt->damage = 10;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_FALSE;
	
	wt->fn_processState	=	fn_p_flamethrower ;
	wt->fn_fireWeapon	=	fn_f_flamethrower ;
	wt->fn_reloadWeapon =	fn_r_flamethrower ;
	wt->fn_fxMotion		=	fn_x_flamethrower ;
	wt->fn_applyWeapon	=	fn_a_flamethrower ;
	wt->fn_noAmmo		=	fn_n_flamethrower ;
	wt->fn_secondFire	=	fn_s_flamethrower ;
	wt->fn_deApply		=	fn_y_flamethrower ;
	wt->fn_changeable	=	fn_c_flamethrower ;
	wt->fn_renderLine	=	fn_l_flamethrower ;

	soundsys_loadWaw(".\\sfx\\weapon\\flamethrower\\fire.wav", &(wt->weaponSound[0]) );
	soundsys_loadWaw(".\\sfx\\weapon\\flamethrower\\idle.wav", &(wt->weaponSound[5]) );
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Molotov cocktail
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_FALSE;
	wt->max_ammo = 1;
	sprintf(wt->name, "Molotov cocktail");
	wt->unlimited_ammo = GE_FALSE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\molotov.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\molotov.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\molotov.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_FALSE;
	wt->mag = 10;
	wt->mod_acc = 200.0f;
	wt->damage = 10;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_FALSE;
	
	wt->fn_processState	=	fn_p_molotov ;
	wt->fn_fireWeapon	=	fn_f_molotov ;
	wt->fn_reloadWeapon =	fn_r_molotov ;
	wt->fn_fxMotion		=	fn_x_molotov ;
	wt->fn_applyWeapon	=	fn_a_molotov ;
	wt->fn_noAmmo		=	fn_n_molotov ;
	wt->fn_secondFire	=	fn_s_molotov ;
	wt->fn_deApply		=	fn_y_molotov ;
	wt->fn_changeable	=	fn_c_molotov ;
	wt->fn_renderLine	=	fn_l_molotov ;

	soundsys_loadWaw(".\\sfx\\weapon\\molotov\\light.wav", &(wt->weaponSound[0]) );
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Signalpistol
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_FALSE;
	wt->max_ammo = 1;
	sprintf(wt->name, "Leuchtpistole 42");
	wt->unlimited_ammo = GE_FALSE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\signal_pistol.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\signal_pistol.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\signal_pistol.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_FALSE;
	wt->mag = 5;
	wt->mod_acc = 200.0f;
	wt->damage = 10;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_FALSE;
	
	wt->fn_processState	=	fn_p_signal ;
	wt->fn_fireWeapon	=	fn_f_signal ;
	wt->fn_reloadWeapon =	fn_r_signal ;
	wt->fn_fxMotion		=	fn_x_signal ;
	wt->fn_applyWeapon	=	fn_a_signal ;
	wt->fn_noAmmo		=	fn_n_signal ;
	wt->fn_secondFire	=	fn_s_signal ;
	wt->fn_deApply		=	fn_y_signal ;
	wt->fn_changeable	=	fn_c_signal ;
	wt->fn_renderLine	=	fn_l_signal ;
	//--------------------------------------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------------------------------------
	//Explosives
	//--------------------------------------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------------------------------------
	//Grenade launcher
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_FALSE;
	wt->max_ammo = 1;
	sprintf(wt->name, "m79 Grenade launcher");
	wt->unlimited_ammo = GE_FALSE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\grenade_launcher.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\grenade_launcher.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\grenade_launcher.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_FALSE;
	wt->mag = 5;
	wt->mod_acc = 200.0f;
	wt->damage = 10;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_FALSE;
	
	wt->fn_processState	=	fn_p_m79 ;
	wt->fn_fireWeapon	=	fn_f_m79 ;
	wt->fn_reloadWeapon =	fn_r_m79 ;
	wt->fn_fxMotion		=	fn_x_m79 ;
	wt->fn_applyWeapon	=	fn_a_m79 ;
	wt->fn_noAmmo		=	fn_n_m79 ;
	wt->fn_secondFire	=	fn_s_m79 ;
	wt->fn_deApply		=	fn_y_m79 ;
	wt->fn_changeable	=	fn_c_m79 ;
	wt->fn_renderLine	=	fn_l_m79 ;

	soundsys_loadWaw(".\\sfx\\weapon\\launcher\\fire.wav", &(wt->weaponSound[0]) );
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Grenade
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_FALSE;
	wt->max_ammo = 1;
	sprintf(wt->name, "L2A2 grenades");
	wt->unlimited_ammo = GE_FALSE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\grenade.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\grenade.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\grenade.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_FALSE;
	wt->mag = 25;
	wt->mod_acc = 200.0f;
	wt->damage = 10;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_FALSE;
	
	wt->fn_processState	=	fn_p_grenade ;
	wt->fn_fireWeapon	=	fn_f_grenade ;
	wt->fn_reloadWeapon =	fn_r_grenade ;
	wt->fn_fxMotion		=	fn_x_grenade ;
	wt->fn_applyWeapon	=	fn_a_grenade ;
	wt->fn_noAmmo		=	fn_n_grenade ;
	wt->fn_secondFire	=	fn_s_grenade ;
	wt->fn_deApply		=	fn_y_grenade ;
	wt->fn_changeable	=	fn_c_grenade ;
	wt->fn_renderLine	=	fn_l_grenade ;

	soundsys_loadWaw(".\\sfx\\weapon\\grenade\\sprintout.wav", &(wt->weaponSound[0]) );
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//c4
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_FALSE;
	wt->max_ammo = 1;
	sprintf(wt->name, "C4");
	wt->unlimited_ammo = GE_FALSE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\c4.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\c4.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\c4.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_FALSE;
	wt->mag = 5;
	wt->mod_acc = 200.0f;
	wt->damage = 10;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_FALSE;
	
	wt->fn_processState	=	fn_p_c4 ;
	wt->fn_fireWeapon	=	fn_f_c4 ;
	wt->fn_reloadWeapon =	fn_r_c4 ;
	wt->fn_fxMotion		=	fn_x_c4 ;
	wt->fn_applyWeapon	=	fn_a_c4 ;
	wt->fn_noAmmo		=	fn_n_c4 ;
	wt->fn_secondFire	=	fn_s_c4 ;
	wt->fn_deApply		=	fn_y_c4 ;
	wt->fn_changeable	=	fn_c_c4 ;
	wt->fn_renderLine	=	fn_l_c4 ;
	//--------------------------------------------------------------------------------------------------------------------



	//--------------------------------------------------------------------------------------------------------------------
	//Mine
	wt = & ( weaponTypes[i] ); i++;
	wt->hold_fire = GE_TRUE;
	wt->max_ammo = 1;
	sprintf(wt->name, "PMA-2 plastic personal mine");
	wt->unlimited_ammo = GE_FALSE;
	//wt->sound = load_sound(".\\sound\\weapons\\9mm.wav");
	wt->s_icon = LoadBmp(".\\gfx\\weapons\\s\\mine.bmp");
	wt->u_icon = LoadBmp(".\\gfx\\weapons\\u\\mine.bmp");
	wt->n_icon = LoadBmp(".\\gfx\\weapons\\n\\mine.bmp");
	wt->showBar = GE_FALSE;
	wt->inWater = GE_FALSE;
	wt->mag = 10;
	wt->mod_acc = 200.0f;
	wt->damage = 10;
	wt->damageType = DAMAGE_NORMAL;
	wt->ap = GE_FALSE;
	
	wt->fn_processState	=	fn_p_mine ;
	wt->fn_fireWeapon	=	fn_f_mine ;
	wt->fn_reloadWeapon =	fn_r_mine ;
	wt->fn_fxMotion		=	fn_x_mine ;
	wt->fn_applyWeapon	=	fn_a_mine ;
	wt->fn_noAmmo		=	fn_n_mine ;
	wt->fn_secondFire	=	fn_s_mine ;
	wt->fn_deApply		=	fn_y_mine ;
	wt->fn_changeable	=	fn_c_mine ;
	wt->fn_renderLine	=	fn_l_mine ;
	//--------------------------------------------------------------------------------------------------------------------

	printLog("Weapon type:\n");
	for( i = 0; i < TOTAL_NUMBER_OF_WEAPONS; i++)
	{
		if( !weaponTypes[i].s_icon )
		{
			sprintf(str, "Failed to load s_icon weapontype %i\n", i+1 );
			printLog(str);
			b=0;
		}
			
		if( !weaponTypes[i].u_icon )
		{
			sprintf(str, "Failed to load u_icon weapontype %i\n", i+1 );
			printLog(str);
			b=0;
		}
			
		if( !weaponTypes[i].n_icon )
		{
			sprintf(str, "Failed to load n_icon weapontype %i\n", i+1 );
			printLog(str);
			b=0;
		}
	}

	return b;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		weapon_init
//			init all the weapons, load the bitmaps and set all the values
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
geBoolean weapon_init()
{
	//simple init
	int c=0;
//	int w;
	int i=0;
	int ub=0;

	sniper_enabled = 0;

	soundsys_loadWaw(".\\sfx\\weapon\\ric1.wav", &(ric[0]) );
	soundsys_loadWaw(".\\sfx\\weapon\\ric2.wav", &(ric[1]) );
	soundsys_loadWaw(".\\sfx\\weapon\\bang.wav", &explosion_sound);

	if( !weapon_grenade_init() ){
		printLog("Failed to init weapons grenade.\n");
		error("Failed to init weapons grenade");
		return GE_FALSE;
	}

	//set offset to knows value
	Offset.X = Offset.Y = Offset.Z = 0.0f;
	ApplyHLOffset = GE_FALSE;

	//set all weapons to known values
	for( c = 0; c < TOTAL_NUMBER_OF_WEAPONS; c++)
	{
			weaponTypes[c].s_icon = 0;
			weaponTypes[c].u_icon = 0;
			weaponTypes[c].n_icon = 0;
	}

	select_background = LoadBmp(".\\gfx\\weapons\\select_background.bmp");
	slct = LoadBmp(".\\gfx\\weapons\\selected.bmp");
	forbidden = LoadBmp(".\\gfx\\weapons\\forbidden.bmp");
	spot = LoadBmp(".\\gfx\\hud\\meny\\spot.bmp");

	bar = LoadBmp(".\\gfx\\hud\\bar.bmp");
	bar_bkg = LoadBmp(".\\gfx\\hud\\bar_bkg.bmp");

	line = LoadBmp(".\\gfx\\hud\\line.bmp");
	line_bkg = LoadBmp(".\\gfx\\hud\\line_bkg.bmp");

	classNumber[0] = LoadBmp(".\\gfx\\hud\\meny\\1.bmp");
	classNumber[1] = LoadBmp(".\\gfx\\hud\\meny\\2.bmp");
	classNumber[2] = LoadBmp(".\\gfx\\hud\\meny\\3.bmp");
	classNumber[3] = LoadBmp(".\\gfx\\hud\\meny\\4.bmp");
	classNumber[4] = LoadBmp(".\\gfx\\hud\\meny\\5.bmp");
	classNumber[5] = LoadBmp(".\\gfx\\hud\\meny\\6.bmp");

	ub = setup_weaponTypes();

	//--------------------------------------------------------------------------------------------------------------------
	//Closecombat
	//--------------------------------------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------------------------------------
	//Hand to hand - unarmed
	current_weapon = & (weapons[WEAPON_C_COMBATS][0] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	current_weapon->number_of_weapons = 1;
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Knife
	current_weapon = & (weapons[WEAPON_C_COMBATS][1] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	current_weapon->number_of_weapons = 1;
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Hammer
	current_weapon = & (weapons[WEAPON_C_COMBATS][2] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	current_weapon->number_of_weapons = 1;
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Axe
	current_weapon = & (weapons[WEAPON_C_COMBATS][3] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	current_weapon->number_of_weapons = 1;
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//PISTOLS
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//9mm
	current_weapon = & (weapons[WEAPON_C_PISTOLS][0] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Desert eagle
	current_weapon = & (weapons[WEAPON_C_PISTOLS][1] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Improved tazer
	current_weapon = & (weapons[WEAPON_C_PISTOLS][2] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Improved trancalizer
	current_weapon = & (weapons[WEAPON_C_PISTOLS][3] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	//--------------------------------------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------------------------------------
	//RIFLES
	//--------------------------------------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------------------------------------
	//Pump-Shotgun
	current_weapon = & (weapons[WEAPON_C_RIFLES][0] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Mag 7
	current_weapon = & (weapons[WEAPON_C_RIFLES][1] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Sniper rifle
	current_weapon = & (weapons[WEAPON_C_RIFLES][2] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Improved sniper rifle
	current_weapon = & (weapons[WEAPON_C_RIFLES][3] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Automatics
	//--------------------------------------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------------------------------------
	//smg
	current_weapon = & (weapons[WEAPON_C_AUTOMATICS][0] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Uzi
	current_weapon = & (weapons[WEAPON_C_AUTOMATICS][1] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Assult rifle
	current_weapon = & (weapons[WEAPON_C_AUTOMATICS][2] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Advanced assult rifle
	current_weapon = & (weapons[WEAPON_C_AUTOMATICS][3] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	//--------------------------------------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------------------------------------
	//FLAMES
	//--------------------------------------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------------------------------------
	//Flamethrower
	current_weapon = & (weapons[WEAPON_C_FLAMES][0] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Improved flamethrower
	current_weapon = & (weapons[WEAPON_C_FLAMES][1] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Molotov cocktail
	current_weapon = & (weapons[WEAPON_C_FLAMES][2] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Signalpistol
	current_weapon = & (weapons[WEAPON_C_FLAMES][3] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	//--------------------------------------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------------------------------------
	//Explosives
	//--------------------------------------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------------------------------------
	//Grenade launcher
	current_weapon = & (weapons[WEAPON_C_EXPLOSIVES][0] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//Grenade
	current_weapon = & (weapons[WEAPON_C_EXPLOSIVES][1] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	//--------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------
	//c4
	current_weapon = & (weapons[WEAPON_C_EXPLOSIVES][2] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	//--------------------------------------------------------------------------------------------------------------------



	//--------------------------------------------------------------------------------------------------------------------
	//Mine
	current_weapon = & (weapons[WEAPON_C_EXPLOSIVES][3] );
	current_weapon->theWeapon = & weaponTypes[i]; i++;
	current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
	current_weapon->magasines = current_weapon->theWeapon->mag;
	current_weapon->state = 0;
	current_weapon->stateTime = 0.0f;
	current_weapon->value = 0;
	//--------------------------------------------------------------------------------------------------------------------

	{
		int b=ub;

		printLog("Weapon loading report:\n");

		if( !b ) {
			printLog("Failed to do weapontypes.\n");
		}

		for( c= 0; c< 6; c++)
		{
			if( !classNumber[c] )
			{
				sprintf(str, "Failed to load classNumber[%i]\n",c);
				printLog(str);
				b=0;
			}
		}

		if( !spot )
		{
			b=0;
			printLog("Failed to load spot.\n");
		}

		if( !select_background )
		{
			b=0;
			printLog("Failed to load select_bacground.\n");
		}

		if( !slct )
		{
			b=0;
			printLog("Failed to load select.\n");
		}

		if( !forbidden )
		{
			b=0;
			printLog("Failed to load forbidden.\n");
		}

		if( !bar )
		{
			b=0;
			printLog("Failed to load bar.\n");
		}

		if( !bar_bkg )
		{
			b=0;
			printLog("Failed to load bar_bkg.\n");
		}

		if( !line )
		{
			b=0;
			printLog("Failed to load line.\n");
		}

		if( !line_bkg )
		{
			b=0;
			printLog("Failed to load line_bkg.\n");
		}

		if( b)
		{
			printLog("Loadings ok\n");
		}
		else
		{
			printLog("Loadings failed\n");
			return GE_FALSE;
		}
	}

	current_weapon = 0;
	selClass = WEAPON_C_PISTOLS;
	selWeapon = 0;
	//apply_weapon();

	weaponRgb.r = 200.0f;
	weaponRgb.g = 20.0f;
	weaponRgb.b = 20.0f;
	weaponRgb.a = 200.0f;
#ifdef NDEBUG
	weapon_strip();
#pragma message("Striping player")
#else
	#pragma message("Leaving default weaponammo")
#endif

	return GE_TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		weapon_reload
//			reloads the current weapon
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void weapon_reload()
{
	disable_selector();
	if( current_weapon)
	{
		if( current_weapon->theWeapon->unlimited_ammo )
		{
			game_message("You can't reload this weapon");
		}
		else
		{
			if( current_weapon->magasines > 0 )
			{
				if( current_weapon->ammonution < current_weapon->theWeapon->max_ammo )
				{
					current_weapon->theWeapon->fn_reloadWeapon(current_weapon);
				}
			}
			else
			{
				game_message("You have nothing to reload with");
			}
		}
	}
	else
	{
		system_message("WARNING: Current weapon is null");
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		weapon_select
//			selects a weapon based on what number/weaponclass the user pressed/choosed(1-6)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void weapon_select(unsigned char number)
{
	unsigned char pre = open_sec;
	enable_selector();

	if( number >= NUMBER_OF_C )
	{
		system_message("WARNING: weapon select larger than number of classes");
	}

	if( pre <= 0)
	{
		if( number != selClass)
		{
			selClass = number;
			selWeapon = 0;
		}
		return;
	}

	if( number == selClass)
	{
		selWeapon++;
		if( selWeapon >= NUMBER_OF_WEAPONS )
			selWeapon = 0;
	}
	else
	{
		selClass = number;
		selWeapon = 0;
	}
}




void weapon_tick()
{
	weapon_grenade_proccess(TIME * enemyTime);
	if( current_weapon )
	{
		current_weapon->theWeapon->fn_processState(current_weapon, TIME * enemyTime);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		weapon_destroy
//			cleans up all allocated memory( bitmaps...) contained(used) by the weapons
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void weapon_destroy()
{
	int c;
//	int w;

	weapon_deapply();

	weapon_grenade_kill();

	if( select_background )
	{
		printLog("Removing select_background.\n");
		geEngine_RemoveBitmap(Engine, select_background);
		geBitmap_Destroy( &select_background );
	}

	if( slct )
	{
		printLog("Removing slct - selector.\n");
		geEngine_RemoveBitmap(Engine, slct);
		geBitmap_Destroy( &slct);
	}

	if( forbidden )
	{
		printLog("Removing forbidden.\n");
		geEngine_RemoveBitmap(Engine, forbidden);
		geBitmap_Destroy( &forbidden );
	}

	for( c=0; c < 6; c++)
	{
		if( classNumber[c] )
		{
			printLog("Removing classNumber[c].\n");
			geEngine_RemoveBitmap(Engine, classNumber[c]);
			geBitmap_Destroy( &(classNumber[c]) );
		}
	}

	if( spot )
	{
		printLog("Removing spot.\n");
		geEngine_RemoveBitmap(Engine, spot);
		geBitmap_Destroy( &spot );
	}

	if( bar )
	{
		printLog("Removing bar.\n");
		geEngine_RemoveBitmap(Engine, bar);
		geBitmap_Destroy( &bar );
	}

	if( bar_bkg )
	{
		printLog("Removing bar_bkg.\n");
		geEngine_RemoveBitmap(Engine, bar_bkg);
		geBitmap_Destroy( &bar_bkg );
	}

	if( line )
	{
		printLog("Removing line.\n");
		geEngine_RemoveBitmap(Engine, line);
		geBitmap_Destroy( &line );
	}

	if( line_bkg )
	{
		printLog("Removing line_bkg.\n");
		geEngine_RemoveBitmap(Engine, line_bkg);
		geBitmap_Destroy( &line_bkg );
	}

	for( c = 0; c < TOTAL_NUMBER_OF_WEAPONS; c++)
	{
		WeaponType* wt;
		wt = & weaponTypes[c];
		if( wt->s_icon )
		{
			printLog("Removing current_weapon->s_icon.\n");
			geEngine_RemoveBitmap(Engine, wt->s_icon);
			geBitmap_Destroy( &(wt->s_icon) );
		}
		if( wt->u_icon )
		{
			printLog("Removing current_weapon->u_icon.\n");
			geEngine_RemoveBitmap(Engine, wt->u_icon);
			geBitmap_Destroy( &(wt->u_icon) );
		}
		if( wt->n_icon )
		{
			printLog("Removing current_weapon->n_icon.\n");
			geEngine_RemoveBitmap(Engine, wt->n_icon);
			geBitmap_Destroy( &(wt->n_icon) );
		}
	}

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		weapon_evry_sec
//			does operations that is used evry second
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void weapon_evry_sec()
{
	if( open_sec )
	{
		open_sec--;

		if( !open_sec )
		{
			disable_selector();
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




geBoolean weapon_hasBar()
{
	if( current_weapon )
	{
		return current_weapon->theWeapon->showBar;
	}

	return GE_FALSE;
}

geBoolean weapon_drawBar(int x, int y)
{
	GE_Rect br;
	float f;

	if( !current_weapon )
	{
		printLog("current weapon is null!\n");
		return GE_FALSE;
	}

	f = (float)(current_weapon->ammonution) / (float)(current_weapon->theWeapon->max_ammo);

	f = 1- f;

	if( !renderBitmap(bar_bkg, x, y) )
	{
		printLog("failed to draw bkg!\n");
		return GE_FALSE;
	}

	br.Left = 0;
	br.Right = 13;
	br.Top = (int32) (f* 30);
	br.Bottom = 31;

	if( !geEngine_DrawBitmap(Engine, bar, &br, x, y+br.Top ) )
	{
		printLog("Failed to draw bar.\n");
		return GE_FALSE;
	}

	return GE_TRUE;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		render_classnumber(internal function)
//			function for easing rendering of all the classnumbers - numbers at the top left
//			perhaps this should be in the hud function calls
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
geBoolean render_classnumber(geBitmap* bmp, int x, int y, geBoolean render)
{
	if(render)
	{
		if( !renderBitmap(spot, x, y) )return GE_FALSE;
	}
	else
	{
		if( !renderBitmap(bmp, x, y) ) return GE_FALSE;
	}
	return GE_TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		render_weapon_bmp(internal_function)
//			function for easing the bitmap weapon
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
geBoolean render_weapon_bmp(weapon *w, int x, int y, geBoolean render)
{
	geBitmap *b=0;
	geBoolean ok = GE_TRUE;

	if( w->number_of_weapons > 0)
	{
		if( (w->magasines + w->ammonution) > 0 || w->theWeapon->unlimited_ammo == GE_TRUE)
		{
			//show ok
			ok = GE_TRUE;
			b = w->theWeapon->s_icon;
		}
		else
		{
			//show half ok
			ok = GE_FALSE;
			b = w->theWeapon->u_icon;
		}
	}
	else
	{
		//show not ok
		ok = GE_FALSE;
		b = w->theWeapon->n_icon;
	}

	if( render )
	{
		if( ok)
		{
			if( !renderBitmap(slct, x, y) )return GE_FALSE;
		}
	}

	if( !renderBitmap(b, x, y) ) return GE_FALSE;

	if( render )
	{
		if( !ok)
		{
			if( !renderBitmap(forbidden, x, y) ) return GE_FALSE;
		}
	}

	return GE_TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		weapon_render
//			render all the weapons
//			*selector and the weapon bitmaps
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
geBoolean weapon_render()
{
	float f = 0.0f;

	if( current_weapon )
	{
		f = current_weapon->theWeapon->fn_renderLine(current_weapon) ;
	}
	else
	{
		printLog("current_weapon is null in weapon_render()\n");
	}

//	char str[600];
	if( open_sec )
	{
		//do da render
		if(! render_classnumber(classNumber[0], 0,  1 , selClass == 0) )return GE_FALSE;
		if(! render_classnumber(classNumber[1], 17, 1 , selClass == 1) )return GE_FALSE;
		if(! render_classnumber(classNumber[2], 33, 1 , selClass == 2) )return GE_FALSE;
		if(! render_classnumber(classNumber[3], 49, 1 , selClass == 3) )return GE_FALSE;
		if(! render_classnumber(classNumber[4], 65, 1 , selClass == 4) )return GE_FALSE;
		if(! render_classnumber(classNumber[5], 81, 1 , selClass == 5) )return GE_FALSE;


		//render select_background
		if( !renderBitmap(select_background, Width-85, Height-304) )
			return GE_FALSE;
		//render weapon category with selected weapon / cross
		if(! render_weapon_bmp(&(weapons[selClass][0]), Width-71, Height-289, 0 == selWeapon) )return GE_FALSE;
		if(! render_weapon_bmp(&(weapons[selClass][1]), Width-71, Height-231, 1 == selWeapon) )return GE_FALSE;
		if(! render_weapon_bmp(&(weapons[selClass][2]), Width-71, Height-173, 2 == selWeapon) )return GE_FALSE;
		if(! render_weapon_bmp(&(weapons[selClass][3]), Width-71, Height-115, 3 == selWeapon) )return GE_FALSE;
	}

//	sprintf(str, "Ammo %i", current_weapon->ammonution);
//	geEngine_Printf(Engine, 400 , 300 , str );

	if( f != 0.0f )
	{
		GE_Rect br;
		
		int x=Width-500;
		int y=Height-50;
		
		f = 1- f;
		
		if( !renderBitmap(line_bkg, x, y) )
		{
			printLog("failed to draw bkg!\n");
			return GE_FALSE;
		}
		
		f = 1.0f - f;

		br.Left = 0;
		br.Right = (int32) (f* 250);
		br.Top = 0;
		br.Bottom = 20;
		
		if( !geEngine_DrawBitmap(Engine, line, &br, x, y ) )
		{
			printLog("Failed to draw bar.\n");
			return GE_FALSE;
		}
		
		return GE_TRUE;
	}

	return GE_TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		weapon_next
//			selects the next weapon in the line
//				current	is	last in class 1
//				next	is	first in class 2
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void weapon_next()
{
	enable_selector();

	selWeapon++;

	if( selWeapon >= NUMBER_OF_WEAPONS )
	{
		//change class
		selWeapon =0;
		selClass++;
	}

	if( selClass >= NUMBER_OF_C )
		selClass =0;

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		weapon_previos
//			selects the previous weapon in the line
//				current	is	first in class 2
//				next	is	last in class 1
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void weapon_previous()
{
	enable_selector();

	if( selWeapon == 0 )
	{
		//change class
		selWeapon =NUMBER_OF_WEAPONS-1;

		if( selClass == 0 )
		{
			selClass = NUMBER_OF_C-1;
		}
		else
		{
			selClass--;
		}
	}
	else
	{
		selWeapon--;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
















/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		weapon_sfire
//			second fires th current weapon
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void weapon_sfire(geBoolean mbnew)
{
	if( current_weapon )
	{
		current_weapon->theWeapon->fn_secondFire(current_weapon, mbnew);
	}
	else
	{
		system_message("Current weapon is null");
	}
}

























/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		weapon_bob
//			moves the weapon forward and backward, and calls the "play walk sound" functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////////



void weapon_bob(char bob)
{
	float multi=1.0;

	if(keys[ controls.holdwalk ] )
		multi = 0.04f; // 2.0f / 50.0f
	else
		multi = 0.06f; // 3.0f / 20.0f

	if(bob)
	{
		if((Offset.Z ==-7.0f || Offset.Z >=-8.2f) && ApplyHLOffset == GE_TRUE)
		{
			Offset.Z -= TIME * multi * heroTime *speed;

			if(Offset.Z <=-8.2f)
			{
				ApplyHLOffset = GE_FALSE;
				play_walksound_right();
			}
		}

		if(ApplyHLOffset == GE_FALSE)
		{
			Offset.Z += TIME * multi * heroTime * speed;

			if(Offset.Z >=-7.0f)
			{
				ApplyHLOffset = GE_TRUE;
				play_walksound_left();
			}
		}
	}
	else
	{
		Offset.Z += TIME * 3.0f * multi * heroTime * speed;
		if(Offset.Z >=-7.0f)
			Offset.Z =-7.0f;
	}
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		weapon_water
//			returns true if the weapon works in water, false if not
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

geBoolean weapon_water()
{
	if( current_weapon )
		return current_weapon->theWeapon->inWater;
	return GE_FALSE;
}

void weapon_strip()
{
	weapon_loose_knife();
	weapon_loose_hammer();
	weapon_loose_axe();
	weapon_loose_glock();
	weapon_loose_deagle();
	weapon_loose_tazer();
	weapon_loose_tranqualizer();
	weapon_loose_shotgun();
	weapon_loose_mag7();
	weapon_loose_sniper();
	weapon_loose_barret();
	weapon_loose_smg();
	weapon_loose_uzi();
	weapon_loose_ak47();
	weapon_loose_commando();
	weapon_loose_lft();
	weapon_loose_flamethrower();
	weapon_loose_molotov();
	weapon_loose_signalpistol();
	weapon_loose_launcher();
	weapon_loose_grenade();
	weapon_loose_c4();
	weapon_loose_mine();

	current_weapon = 0;
	selClass = WEAPON_C_COMBATS;
	selWeapon = 0;
	apply_weapon();
}

void weapon_give_all()
{
	weapon_give_knife();
	weapon_give_hammer();
	weapon_give_axe();
	weapon_give_glock(0);
	weapon_give_deagle(0);
	weapon_give_tazer(0);
	weapon_give_tranqualizer(0);
	weapon_give_shotgun(0);
	weapon_give_mag7(0);
	weapon_give_sniper(0);
	weapon_give_barret(0);
	weapon_give_smg(0);
	weapon_give_uzi(0);
	weapon_give_ak47(0);
	weapon_give_commando(0);
	weapon_give_lft(0);
	weapon_give_flamethrower(0);
	weapon_give_molotov(0);
	weapon_give_signalpistol(0);
	weapon_give_launcher(0);
	weapon_give_grenade(0);
	weapon_give_c4(0);
	weapon_give_mine(0);
}

/*********************************************************************************************************/
/*********************************************************************************************************/
/*********************************************************************************************************/
/*********************************************************************************************************/

void weapon_got(char* name){
	char message[300];
	sprintf(message, "You picked up a %s", name);
	game_message(message);
}
void weapon_got_ammo(char* name){
	char message[300];
	sprintf(message, "You picked up magasines for %s", name);
	game_message(message);
}

void weapon_give_knife()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_COMBATS][1] );
	weapon_got(current_weapon->theWeapon->name);
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}

void weapon_give_hammer()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_COMBATS][2] );
	weapon_got(current_weapon->theWeapon->name);
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}

void weapon_give_axe()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_COMBATS][3] );
	weapon_got(current_weapon->theWeapon->name);
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}

	/////////////////////////////////////////////////////
	//Pistols

void weapon_give_glock(int mag)
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_PISTOLS][0] );
	if( mag ) {
		if( (current_weapon->magasines + mag) >= 99 ) {
			current_weapon->magasines = 99;
		}
		else {
			current_weapon->magasines += mag;
		}
		weapon_got_ammo(current_weapon->theWeapon->name);
	}
	else {
		current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
		current_weapon->magasines = current_weapon->theWeapon->mag;
		weapon_got(current_weapon->theWeapon->name);
	}
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}
	
void weapon_give_deagle(int mag)
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_PISTOLS][1] );
	if( mag ) {
		if( (current_weapon->magasines + mag) >= 99 ) {
			current_weapon->magasines = 99;
		}
		else {
			current_weapon->magasines += mag;
		}
		weapon_got_ammo(current_weapon->theWeapon->name);
	}
	else {
		current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
		current_weapon->magasines = current_weapon->theWeapon->mag;
		weapon_got(current_weapon->theWeapon->name);
	}
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}

void weapon_give_tazer(int mag)
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_PISTOLS][2] );
	if( mag ) {
		if( (current_weapon->magasines + mag) >= 99 ) {
			current_weapon->magasines = 99;
		}
		else {
			current_weapon->magasines += mag;
		}
		weapon_got_ammo(current_weapon->theWeapon->name);
	}
	else {
		current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
		current_weapon->magasines = current_weapon->theWeapon->mag;
		weapon_got(current_weapon->theWeapon->name);
	}
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}

void weapon_give_tranqualizer(int mag)
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_PISTOLS][3] );
	if( mag ) {
		if( (current_weapon->magasines + mag) >= 99 ) {
			current_weapon->magasines = 99;
		}
		else {
			current_weapon->magasines += mag;
		}
		weapon_got_ammo(current_weapon->theWeapon->name);
	}
	else {
		current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
		current_weapon->magasines = current_weapon->theWeapon->mag;
		weapon_got(current_weapon->theWeapon->name);
	}
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}
	
	//--------------------------------------------------------------------------------------------------------------------
	//RIFLES
	//--------------------------------------------------------------------------------------------------------------------

void weapon_give_shotgun(int mag)
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_RIFLES][0] );
	if( mag ) {
		if( (current_weapon->magasines + mag) >= 99 ) {
			current_weapon->magasines = 99;
		}
		else {
			current_weapon->magasines += mag;
		}
		weapon_got_ammo(current_weapon->theWeapon->name);
	}
	else {
		current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
		current_weapon->magasines = current_weapon->theWeapon->mag;
		weapon_got(current_weapon->theWeapon->name);
	}
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}

void weapon_give_mag7(int mag)
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_RIFLES][1] );
	if( mag ) {
		if( (current_weapon->magasines + mag) >= 99 ) {
			current_weapon->magasines = 99;
		}
		else {
			current_weapon->magasines += mag;
		}
		weapon_got_ammo(current_weapon->theWeapon->name);
	}
	else {
		current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
		current_weapon->magasines = current_weapon->theWeapon->mag;
		weapon_got(current_weapon->theWeapon->name);
	}
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}
	
void weapon_give_sniper(int mag)
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_RIFLES][2] );
	if( mag ) {
		if( (current_weapon->magasines + mag) >= 99 ) {
			current_weapon->magasines = 99;
		}
		else {
			current_weapon->magasines += mag;
		}
		weapon_got_ammo(current_weapon->theWeapon->name);
	}
	else {
		current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
		current_weapon->magasines = current_weapon->theWeapon->mag;
		weapon_got(current_weapon->theWeapon->name);
	}
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}

void weapon_give_barret(int mag)
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_RIFLES][3] );
	if( mag ) {
		if( (current_weapon->magasines + mag) >= 99 ) {
			current_weapon->magasines = 99;
		}
		else {
			current_weapon->magasines += mag;
		}
		weapon_got_ammo(current_weapon->theWeapon->name);
	}
	else {
		current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
		current_weapon->magasines = current_weapon->theWeapon->mag;
		weapon_got(current_weapon->theWeapon->name);
	}
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}
	
	//--------------------------------------------------------------------------------------------------------------------
	//Automatics
	//--------------------------------------------------------------------------------------------------------------------

void weapon_give_smg(int mag)
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_AUTOMATICS][0] );
	if( mag ) {
		if( (current_weapon->magasines + mag) >= 99 ) {
			current_weapon->magasines = 99;
		}
		else {
			current_weapon->magasines += mag;
		}
		weapon_got_ammo(current_weapon->theWeapon->name);
	}
	else {
		current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
		current_weapon->magasines = current_weapon->theWeapon->mag;
		weapon_got(current_weapon->theWeapon->name);
	}
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}

void weapon_give_uzi(int mag)
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_AUTOMATICS][1] );
	if( mag ) {
		if( (current_weapon->magasines + mag) >= 99 ) {
			current_weapon->magasines = 99;
		}
		else {
			current_weapon->magasines += mag;
		}
		weapon_got_ammo(current_weapon->theWeapon->name);
	}
	else {
		current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
		current_weapon->magasines = current_weapon->theWeapon->mag;
		weapon_got(current_weapon->theWeapon->name);
	}
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}

void weapon_give_ak47(int mag)
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_AUTOMATICS][2] );
	if( mag ) {
		if( (current_weapon->magasines + mag) >= 99 ) {
			current_weapon->magasines = 99;
		}
		else {
			current_weapon->magasines += mag;
		}
		weapon_got_ammo(current_weapon->theWeapon->name);
	}
	else {
		current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
		current_weapon->magasines = current_weapon->theWeapon->mag;
		weapon_got(current_weapon->theWeapon->name);
	}
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}

void weapon_give_commando(int mag)
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_AUTOMATICS][3] );
	if( mag ) {
		if( (current_weapon->magasines + mag) >= 99 ) {
			current_weapon->magasines = 99;
		}
		else {
			current_weapon->magasines += mag;
		}
		weapon_got_ammo(current_weapon->theWeapon->name);
	}
	else {
		current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
		current_weapon->magasines = current_weapon->theWeapon->mag;
		weapon_got(current_weapon->theWeapon->name);
	}
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}
	
	//--------------------------------------------------------------------------------------------------------------------
	//FLAMES
	//--------------------------------------------------------------------------------------------------------------------

void weapon_give_lft(int mag)
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_FLAMES][0] );
	if( mag ) {
		if( (current_weapon->magasines + mag) >= 99 ) {
			current_weapon->magasines = 99;
		}
		else {
			current_weapon->magasines += mag;
		}
		weapon_got_ammo(current_weapon->theWeapon->name);
	}
	else {
		current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
		current_weapon->magasines = current_weapon->theWeapon->mag;
		weapon_got(current_weapon->theWeapon->name);
	}
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}

void weapon_give_flamethrower(int mag)
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_FLAMES][1] );
	if( mag ) {
		if( (current_weapon->magasines + mag) >= 99 ) {
			current_weapon->magasines = 99;
		}
		else {
			current_weapon->magasines += mag;
		}
		weapon_got_ammo(current_weapon->theWeapon->name);
	}
	else {
		current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
		current_weapon->magasines = current_weapon->theWeapon->mag;
		weapon_got(current_weapon->theWeapon->name);
	}
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}

void weapon_give_molotov(int mag)
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_FLAMES][2] );
	if( mag ) {
		if( (current_weapon->magasines + mag) >= 99 ) {
			current_weapon->magasines = 99;
		}
		else {
			current_weapon->magasines += mag;
		}
		weapon_got_ammo(current_weapon->theWeapon->name);
	}
	else {
		current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
		current_weapon->magasines = current_weapon->theWeapon->mag;
		weapon_got(current_weapon->theWeapon->name);
	}
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}

void weapon_give_signalpistol(int mag)
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_FLAMES][3] );
	if( mag ) {
		if( (current_weapon->magasines + mag) >= 99 ) {
			current_weapon->magasines = 99;
		}
		else {
			current_weapon->magasines += mag;
		}
		weapon_got_ammo(current_weapon->theWeapon->name);
	}
	else {
		current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
		current_weapon->magasines = current_weapon->theWeapon->mag;
		weapon_got(current_weapon->theWeapon->name);
	}
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}
	
	//--------------------------------------------------------------------------------------------------------------------
	//Explosives
	//--------------------------------------------------------------------------------------------------------------------

void weapon_give_launcher(int mag)
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_EXPLOSIVES][0] );
	if( mag ) {
		if( (current_weapon->magasines + mag) >= 99 ) {
			current_weapon->magasines = 99;
		}
		else {
			current_weapon->magasines += mag;
		}
		weapon_got_ammo(current_weapon->theWeapon->name);
	}
	else {
		current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
		current_weapon->magasines = current_weapon->theWeapon->mag;
		weapon_got(current_weapon->theWeapon->name);
	}
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}

void weapon_give_grenade(int mag)
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_EXPLOSIVES][1] );
	if( mag ) {
		if( (current_weapon->magasines + mag) >= 99 ) {
			current_weapon->magasines = 99;
		}
		else {
			current_weapon->magasines += mag;
		}
		weapon_got_ammo(current_weapon->theWeapon->name);
	}
	else {
		current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
		current_weapon->magasines = current_weapon->theWeapon->mag;
		weapon_got(current_weapon->theWeapon->name);
	}
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}

void weapon_give_c4(int mag)
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_EXPLOSIVES][2] );
	if( mag ) {
		if( (current_weapon->magasines + mag) >= 99 ) {
			current_weapon->magasines = 99;
		}
		else {
			current_weapon->magasines += mag;
		}
		weapon_got_ammo(current_weapon->theWeapon->name);
	}
	else {
		current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
		current_weapon->magasines = current_weapon->theWeapon->mag;
		weapon_got(current_weapon->theWeapon->name);
	}
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}

void weapon_give_mine(int mag)
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_EXPLOSIVES][3] );
	if( mag ) {
		if( (current_weapon->magasines + mag) >= 99 ) {
			current_weapon->magasines = 99;
		}
		else {
			current_weapon->magasines += mag;
		}
		weapon_got_ammo(current_weapon->theWeapon->name);
	}
	else {
		current_weapon->ammonution = current_weapon->theWeapon->max_ammo;
		current_weapon->magasines = current_weapon->theWeapon->mag;
		weapon_got(current_weapon->theWeapon->name);
	}
	current_weapon->number_of_weapons = 1;
	current_weapon = cw;
	add_blueFlash( PICKUP_FLASH );
}








/*********************************************************************************************************/
/*********************************************************************************************************/
/*********************************************************************************************************/
/*********************************************************************************************************/










void weapon_loose_knife()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_COMBATS][1] );
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}

void weapon_loose_hammer()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_COMBATS][2] );
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}

void weapon_loose_axe()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_COMBATS][3] );
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}

	/////////////////////////////////////////////////////
	//Pistols

void weapon_loose_glock()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_PISTOLS][0] );
	current_weapon->ammonution = 0;
	current_weapon->magasines = 0;
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}
	
void weapon_loose_deagle()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_PISTOLS][1] );
	current_weapon->ammonution = 0;
	current_weapon->magasines = 0;
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}

void weapon_loose_tazer()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_PISTOLS][2] );
	current_weapon->ammonution = 0;
	current_weapon->magasines = 0;
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}

void weapon_loose_tranqualizer()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_PISTOLS][3] );
	current_weapon->ammonution = 0;
	current_weapon->magasines = 0;
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}
	
	//--------------------------------------------------------------------------------------------------------------------
	//RIFLES
	//--------------------------------------------------------------------------------------------------------------------

void weapon_loose_shotgun()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_RIFLES][0] );
	current_weapon->ammonution = 0;
	current_weapon->magasines = 0;
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}

void weapon_loose_mag7()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_RIFLES][1] );
	current_weapon->ammonution = 0;
	current_weapon->magasines = 0;
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}
	
void weapon_loose_sniper()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_RIFLES][2] );
	current_weapon->ammonution = 0;
	current_weapon->magasines = 0;
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}

void weapon_loose_barret()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_RIFLES][3] );
	current_weapon->ammonution = 0;
	current_weapon->magasines = 0;
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}
	
	//--------------------------------------------------------------------------------------------------------------------
	//Automatics
	//--------------------------------------------------------------------------------------------------------------------

void weapon_loose_smg()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_AUTOMATICS][0] );
	current_weapon->ammonution = 0;
	current_weapon->magasines = 0;
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}

void weapon_loose_uzi()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_AUTOMATICS][1] );
	current_weapon->ammonution = 0;
	current_weapon->magasines = 0;
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}

void weapon_loose_ak47()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_AUTOMATICS][2] );
	current_weapon->ammonution = 0;
	current_weapon->magasines = 0;
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}

void weapon_loose_commando()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_AUTOMATICS][3] );
	current_weapon->ammonution = 0;
	current_weapon->magasines = 0;
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}
	
	//--------------------------------------------------------------------------------------------------------------------
	//FLAMES
	//--------------------------------------------------------------------------------------------------------------------

void weapon_loose_lft()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_FLAMES][0] );
	current_weapon->ammonution = 0;
	current_weapon->magasines = 0;
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}

void weapon_loose_flamethrower()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_FLAMES][1] );
	current_weapon->ammonution = 0;
	current_weapon->magasines = 0;
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}

void weapon_loose_molotov()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_FLAMES][2] );
	current_weapon->ammonution = 0;
	current_weapon->magasines = 0;
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}

void weapon_loose_signalpistol()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_FLAMES][3] );
	current_weapon->ammonution = 0;
	current_weapon->magasines = 0;
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}
	
	//--------------------------------------------------------------------------------------------------------------------
	//Explosives
	//--------------------------------------------------------------------------------------------------------------------

void weapon_loose_launcher()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_EXPLOSIVES][0] );
	current_weapon->ammonution = 0;
	current_weapon->magasines = 0;
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}

void weapon_loose_grenade()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_EXPLOSIVES][1] );
	current_weapon->ammonution = 0;
	current_weapon->magasines = 0;
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}

void weapon_loose_c4()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_EXPLOSIVES][2] );
	current_weapon->ammonution = 0;
	current_weapon->magasines = 0;
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}

void weapon_loose_mine()
{
	weapon* cw = current_weapon;
	current_weapon = & (weapons[WEAPON_C_EXPLOSIVES][3] );
	current_weapon->ammonution = 0;
	current_weapon->magasines = 0;
	current_weapon->number_of_weapons = 0;
	current_weapon = cw;
}










void weapon_grenade_bang(geVec3d position, float radius){
#define MIN 0.1f
#define		COLLISION		geWorld_Collision(World,&(bb.Min),&(bb.Max),&position,&to,GE_CONTENTS_SOLID_CLIP,GE_COLLIDE_ALL,0xffffffff, 0 ,NULL, &lCol)
#define		ADD_MARK		{ bullet_hitObstacle(&lCol, DECALTYPE_SCORCHBIG ); }
#define		SETUP_TO		geVec3d_AddScaled(&position, &direction, 15.0f, &to);
	geVec3d direction;
	geVec3d to;
	geExtBox bb;
	GE_Collision lCol;
	geBoolean result;
	geVec3d wIn;
	geXForm3d_GetIn(&XForm, &wIn);

	geExtBox_Set(&bb, -1.0f * MIN , -1.0f * MIN , -1.0f * MIN , MIN, MIN, MIN);
	soundsys_play3dsound(&explosion_sound, &position, radius, GE_TRUE);
	fx_grenadeExplosion(position);

	explosion_at(&position, radius, 150);

	geVec3d_Clear(&direction);
	direction.Y = -10.0f;
	SETUP_TO;
	result = COLLISION;
	if( result ) {
		ADD_MARK;
	}

	direction.Y = 10.0f;
	SETUP_TO;
	result = COLLISION;
	if( result ) {
		ADD_MARK;
	}

	geVec3d_Clear(&direction);

	direction.X = 10.0f;
	SETUP_TO;
	result = COLLISION;
	if( result ) {
		ADD_MARK;
	}

	direction.X = -10.0f;
	SETUP_TO;
	result = COLLISION;
	if( result ) {
		ADD_MARK;
	}

	geVec3d_Clear(&direction);

	direction.Z = 10.0f;
	SETUP_TO;
	result = COLLISION;
	if( result ) {
		ADD_MARK;
	}

	direction.Z = -10.0f;
	SETUP_TO;
	result = COLLISION;
	if( result ) {
		ADD_MARK;
	}
#undef SETUP_TO
#undef COLLISION
#undef ADD_MARK
#undef MIN
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// end of file
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////