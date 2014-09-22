#include "pickup.h"
#include "infection.h"
#include "inf_actor.h"
#include "player.h"
#include "level.h"
#include "proposeg3d.h"
#include "globalGenesis.h"
#include "sound_system.h"
#include "weapons.h"
#include "log.h"

#define PICKUP_FLASH 0.1f

// geActor_Def* pickup
geActor_Def* pickupMedkit =0;
geActor_Def* pickupArmor =0;
geActor_Def* pickupGrenade =0;
geActor_Def* pickupSmg =0;
geActor_Def* pickupAmmoSmg =0;
geActor_Def* pickupShotgun =0;
geActor_Def* pickupKnife =0;
geActor_Def* pickupPistol =0;
geActor_Def* pickupAmmoPistol =0;
geActor_Def* pickupEagle =0;
geActor_Def* pickupAmmoEagle =0;
geActor_Def* pickupAmmoShotgun =0; // single
geActor_Def* pickupBeretta =0;
geActor_Def* pickupAmmoBeretta =0;
geActor_Def* pickupUzi =0;
geActor_Def* pickupAmmoUzi =0;
geActor_Def* pickupFlamethrower =0;
geActor_Def* pickupAmmoGas =0;
geActor_Def* pickupMolotov =0;
geActor_Def* pickupMinirocketLauncher=0;
geActor_Def* pickupMinirocket=0;
geActor_Def* pickupShotgunShells=0;//several
geActor_Def* pickupEmergencyKit=0;
geActor_Def* pickupSmallMedkit=0;

soundsys_sound pickup_armor;
soundsys_sound pickup_hp[5];

geActor* pickup_getActor(geActor_Def* def, geWorld* World, geVec3d pos){
	geXForm3d xf;
	geActor* act = 0;
	geExtBox box;

	geXForm3d_SetIdentity(&xf);
	xf.Translation = pos;
	act = LoadActor(def, World, 2.0f, GE_ACTOR_RENDER_MIRRORS | GE_ACTOR_RENDER_NORMAL | GE_ACTOR_COLLIDE, &xf );
	if( !act ){
		return 0;
	}
	setup_box( &box,act, pos);
	return act;
}

#define GET_ACTOR(index,definition)		case index: return pickup_getActor(definition, world, pos)
geActor* pickup_getActorByIndex(geWorld* world, int type, geVec3d pos){
	switch( type ){
		GET_ACTOR(1,	pickupMedkit				);
		GET_ACTOR(2,	pickupArmor					);
		GET_ACTOR(3,	pickupGrenade				);
		GET_ACTOR(4,	pickupSmg					);
		GET_ACTOR(5,	pickupAmmoSmg				);
		GET_ACTOR(6,	pickupShotgun				);
		GET_ACTOR(7,	pickupKnife					);
		GET_ACTOR(8,	pickupPistol				);
		GET_ACTOR(9,	pickupAmmoPistol			);
		GET_ACTOR(10,	pickupEagle					);
		GET_ACTOR(11,	pickupAmmoEagle				);
		GET_ACTOR(12,	pickupAmmoShotgun			);
		GET_ACTOR(13,	pickupBeretta				);
		GET_ACTOR(14,	pickupAmmoBeretta			);
		GET_ACTOR(15,	pickupUzi					);
		GET_ACTOR(16,	pickupAmmoUzi				);
		GET_ACTOR(17,	pickupFlamethrower			);
		GET_ACTOR(18,	pickupAmmoGas				);
		GET_ACTOR(19,	pickupMolotov				);
		GET_ACTOR(20,	pickupMinirocketLauncher	);
		GET_ACTOR(21,	pickupMinirocket			);
		GET_ACTOR(22,	pickupShotgunShells			);
		GET_ACTOR(23,	pickupEmergencyKit			);
		GET_ACTOR(24,	pickupSmallMedkit			);
	default:
		return 0;
	}
}
char pickup_loadActorDefinitions(){
	pickupMedkit				= LoadActorDef(".\\actors\\pickup\\medkit.act");
	pickupArmor					= LoadActorDef(".\\actors\\pickup\\armor.act");
	pickupGrenade				= LoadActorDef(".\\actors\\pickup\\grenade.act");
	pickupSmg					= LoadActorDef(".\\actors\\pickup\\smg.act");
	pickupAmmoSmg				= LoadActorDef(".\\actors\\pickup\\asmg.act");
	pickupShotgun				= LoadActorDef(".\\actors\\pickup\\shotgun.act");
	pickupKnife					= LoadActorDef(".\\actors\\pickup\\knife.act");
	pickupPistol				= LoadActorDef(".\\actors\\pickup\\pistol.act");
	pickupAmmoPistol			= LoadActorDef(".\\actors\\pickup\\apistol.act");
	pickupEagle					= LoadActorDef(".\\actors\\pickup\\eagle.act");
	pickupAmmoEagle				= LoadActorDef(".\\actors\\pickup\\aeagle.act");
	pickupAmmoShotgun			= LoadActorDef(".\\actors\\pickup\\ashot.act");
	pickupBeretta				= LoadActorDef(".\\actors\\pickup\\beretta.act");
	pickupAmmoBeretta			= LoadActorDef(".\\actors\\pickup\\aberetta.act");
	pickupUzi					= LoadActorDef(".\\actors\\pickup\\uzi.act");
	pickupAmmoUzi				= LoadActorDef(".\\actors\\pickup\\auzi.act");
	pickupFlamethrower			= LoadActorDef(".\\actors\\pickup\\flamethrower.act");
	pickupAmmoGas				= LoadActorDef(".\\actors\\pickup\\gas.act");
	pickupMolotov				= LoadActorDef(".\\actors\\pickup\\molotov.act");
	pickupMinirocketLauncher	= LoadActorDef(".\\actors\\pickup\\minirocketlauncher.act");
	pickupMinirocket			= LoadActorDef(".\\actors\\pickup\\minirocket.act");
	pickupShotgunShells			= LoadActorDef(".\\actors\\pickup\\shotgunshells.act");
	pickupEmergencyKit			= LoadActorDef(".\\actors\\pickup\\emergencykit.act");
	pickupSmallMedkit			= LoadActorDef(".\\actors\\pickup\\smallmedkit.act");

	soundsys_loadSound(".\\sfx\\pickup\\armor.wav", 1, &pickup_armor, 0, TYPE_2D);
	soundsys_loadSound(".\\sfx\\pickup\\hp1.wav", 1, &pickup_hp[0], 0, TYPE_2D);
	soundsys_loadSound(".\\sfx\\pickup\\hp2.wav", 1, &pickup_hp[1], 0, TYPE_2D);
	soundsys_loadSound(".\\sfx\\pickup\\hp3.wav", 1, &pickup_hp[2], 0, TYPE_2D);
	soundsys_loadSound(".\\sfx\\pickup\\hp4.wav", 1, &pickup_hp[3], 0, TYPE_2D);
	soundsys_loadSound(".\\sfx\\pickup\\hp5.wav", 1, &pickup_hp[4], 0, TYPE_2D);

	if(! pickupMedkit ) return 0;
	if(! pickupArmor ) return 0;
	if(! pickupGrenade ) return 0;
	if(! pickupSmg ) return 0;
	if(! pickupAmmoSmg ) return 0;
	if(! pickupShotgun ) return 0;
	if(! pickupKnife ) return 0;
	if(! pickupPistol ) return 0;
	if(! pickupAmmoPistol ) return 0;
	if(! pickupEagle ) return 0;
	if(! pickupAmmoEagle ) return 0;
	if(! pickupAmmoShotgun ) return 0;
	if(! pickupBeretta ) return 0;
	if(! pickupAmmoBeretta ) return 0;
	if(! pickupUzi ) return 0;
	if(! pickupAmmoUzi ) return 0;
	if(! pickupFlamethrower ) return 0;
	if(! pickupAmmoGas ) return 0;
	if(! pickupMolotov ) return 0;
	if(! pickupMinirocketLauncher ) return 0;
	if(! pickupMinirocket ) return 0;
	if(! pickupShotgunShells ) return 0;
	if(! pickupEmergencyKit ) return 0;
	if(! pickupSmallMedkit ) return 0;

	return 1;
}

#define KILL_DEF(def) if(def) {geActor_DefDestroy(& def ); def = 0;}
void pickup_clearActorDefinitions(){
	KILL_DEF( pickupMedkit );
	KILL_DEF( pickupArmor );
	KILL_DEF( pickupGrenade );
	KILL_DEF( pickupSmg );
	KILL_DEF( pickupAmmoSmg );
	KILL_DEF( pickupShotgun );
	KILL_DEF( pickupKnife );
	KILL_DEF( pickupPistol );
	KILL_DEF( pickupAmmoPistol );
	KILL_DEF( pickupEagle );
	KILL_DEF( pickupAmmoEagle );
	KILL_DEF( pickupAmmoShotgun );
	KILL_DEF( pickupBeretta );
	KILL_DEF( pickupAmmoBeretta );
	KILL_DEF( pickupUzi );
	KILL_DEF( pickupAmmoUzi );
	KILL_DEF( pickupFlamethrower );
	KILL_DEF( pickupAmmoGas );
	KILL_DEF( pickupMolotov );
	KILL_DEF( pickupMinirocketLauncher );
	KILL_DEF( pickupMinirocket );
	KILL_DEF( pickupShotgunShells );
	KILL_DEF( pickupEmergencyKit );
	KILL_DEF( pickupSmallMedkit );
}

char pickup_handlePickup(int index){
	switch( index ) {
	case 1:
		if( hero_hit_points < 100 ){
			hero_hit_points = 100;
			soundsys_play_sound(&pickup_hp[rand()%5], GE_FALSE, TYPE_STOP, 0.0f, 1.0f);
		}
		else {
			return 0;
		}
		break;
	case 2:
		if( hero_armor_points < 100 ){
			hero_armor_points = 100;
			soundsys_play_sound(&pickup_armor, GE_FALSE, TYPE_PLAY, 0.0f, 1.0f);
		}
		else {
			return 0;
		}
		break;
	case 3:
		weapon_give_grenade(0);
		break;
	case 4:
		weapon_give_smg(0);
		break;
	case 5:
		weapon_give_smg(1);
		break;
	case 6:
		weapon_give_shotgun(0);
		break;
	case 7:
		weapon_give_knife();
		break;
	case 8:
		weapon_give_glock(0);
		break;
	case 9:
		weapon_give_glock(1);
		break;
	case 10:
		weapon_give_deagle(0);
		break;
	case 11:
		weapon_give_deagle(1);
		break;
	case 12:
		weapon_give_shotgun(1);
		break;
	case 13:
		weapon_give_sniper(0);
		break;
	case 14:
		weapon_give_sniper(1);
		break;
	case 15:
		weapon_give_uzi(0);
		break;
	case 16:
		weapon_give_uzi(1);
		break;
	case 17:
		weapon_give_lft(0);
		break;
	case 18:
		weapon_give_lft(1);
		break;
	case 19:
		weapon_give_molotov(0);
		break;
	case 20:
		weapon_give_minirocket(0);
		break;
	case 21:
		weapon_give_minirocket(1);
		break;
	case 22:
		weapon_give_shotgun(3); // a pack of shells for the shotgun
		break;
	case 23:
		if( hero_hit_points < 50 ){
			hero_hit_points = 50;
			soundsys_play_sound(&pickup_hp[rand()%5], GE_FALSE, TYPE_STOP, 0.0f, 1.0f);
		}
		else {
			return 0;
		}
		break;
	case 24:
		if( hero_hit_points < 100 ){
			hero_hit_points += 25;
			if( hero_hit_points > 100 ) hero_hit_points = 100;
			soundsys_play_sound(&pickup_hp[rand()%5], GE_FALSE, TYPE_STOP, 0.0f, 1.0f);
		}
		else {
			return 0;
		}
		break;
	default:
		break;
	}
	return 1;
}

#define TEST_DEF(d)  { if(!ok) if(def==d) ok=1; }
char pickup_handleActor(geActor* actor, geWorld* world, geVec3d* from, geVec3d* to){
	geActor_Def* def=0;
	Inf_Pickup* pickup;
	char ok = 0;

	def = geActor_GetActorDef(actor); 

	TEST_DEF( pickupMedkit );
	TEST_DEF( pickupArmor );
	TEST_DEF( pickupGrenade );
	TEST_DEF( pickupSmg );
	TEST_DEF( pickupAmmoSmg );
	TEST_DEF( pickupShotgun );
	TEST_DEF( pickupKnife );
	TEST_DEF( pickupPistol );
	TEST_DEF( pickupAmmoPistol );
	TEST_DEF( pickupEagle );
	TEST_DEF( pickupAmmoEagle );
	TEST_DEF( pickupAmmoShotgun );
	TEST_DEF( pickupBeretta );
	TEST_DEF( pickupAmmoBeretta );
	TEST_DEF( pickupUzi );
	TEST_DEF( pickupAmmoUzi );
	TEST_DEF( pickupFlamethrower );
	TEST_DEF( pickupAmmoGas );
	TEST_DEF( pickupMolotov );
	TEST_DEF( pickupMinirocketLauncher );
	TEST_DEF( pickupMinirocket );
	TEST_DEF( pickupShotgunShells );
	TEST_DEF( pickupEmergencyKit );
	TEST_DEF( pickupSmallMedkit );

	if(! ok ) return 0;

	pickup = (Inf_Pickup*) geActor_GetUserData(actor);
	if( pickup->enable ){
		if( geActor_Collision_Ex(pickup->actor, &ExtBox, from, to) ){
			if( pickup_handlePickup(pickup->type) ){
				pickup->enable = GE_FALSE;
				geWorld_SetActorFlags(world, pickup->actor, 0);
				add_blueFlash(PICKUP_FLASH);
			}
		}
	}

	return 1;
}