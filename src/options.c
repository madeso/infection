#include "options.h"
#include "genesis.h"
#include "decals.h"
#include "log.h"
#include "globalGenesis.h"
#include "player.h"
#include "fxbitmaps.h"


int kill_fx()
{
	if( EM )
	{
		printLog("Destroying Rain.\n");
		Rain_Destroy(EM);
		printLog("Destroying Sprite.\n");
		Sprite_Destroy(EM);
		printLog("Destroying Spout.\n");
		Spout_Destroy(EM);

		printLog("Destroying EffectManager\n");
		EM_Destroy(EM);
		printLog("EM_Destroy done\n");
		EM = 0;

		fxbitmaps_free();
	}

	return 1;
}

int init_fx()
{
	printLog("Creating effect manager");

	if( EM ) return 1;

	// Set up the new Effects System
	EM = EM_Create(Engine, Camera, World);
		
	if(!EM)
	{
		printLog("init_fx: EM_Create failed.\n");
		return 0;
	}
		
	if(!Rain_Create(EM->Engine, EM))
	{
		printLog("init_fx: Rain_Create failed.\n");
		return 0;
	}
		
	if(!Sprite_Create(EM->Engine, EM))
	{
		printLog("init_fx: Sprite_Create failed.\n");
		return 0;
	}
		
	if(!Spout_Create(EM->Engine, EM))
	{
		printLog("init_fx: Spout_Create failed.\n");
		return 0;
	}

	if(! fxbitmaps_load() ){
		printLog("init_fx: fxbitmaps_load() failed\n");
		return 0;
	}

	return 1;
}

int enable_fx()
{
	options.enable_effects = 1;
	return init_fx();
}

int disable_fx()
{
	options.enable_effects = 0;
	return kill_fx();
}

int kill_decals()
{
	if( dMgr )
	{
		printLog("Destroying the DecalManager\n");
		DecalMgr_Destroy(dMgr);
		dMgr = 0;
	}

	return 1;
}

int init_decals()
{
	if( dMgr ) return 1;

	printLog("Creating DecalManager.\n");
	dMgr = DecalMgr_Create(Engine, World);
	
	if( !dMgr )
	{
		printLog("Failed to load deacal manager");
		return 0;
	}
	return 1;
}

int enable_decals()
{
	options.enable_decals = 1;
	return init_decals();
}

int disable_decals()
{
	options.enable_decals = 0;
	return kill_decals();
}