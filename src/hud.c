#include "hud.h"
#include "weapons.h"
#include "extra_genesis.h"
#include "log.h"
#include "globalGenesis.h"
#include "multiint.h"
#include "player.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		load_hud
//			load images that will be used in "the hud"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
geBoolean load_hud()
{
	int i= 0;

	geBoolean ret = GE_TRUE;

	if( Hud = LoadBmp(".\\gfx\\hud\\inf_hud.bmp") )
	{
		printLog("Hud ok.\n");
	}
	else
	{
		printLog("-Failed to load hud.\n");
		run = 0;
		ret = GE_FALSE;
	}

	for( i = 0; i< 11; i++)
	{
		nf1[i] = 0;
	}

	for( i = 0; i< 11; i++)
	{
		nf2[i] = 0;
	}

	if( !(nf1[0] = LoadBmp(".\\gfx\\hud\\nf1\\n0.bmp") ) ) ret = GE_FALSE;
	if( !(nf1[1] = LoadBmp(".\\gfx\\hud\\nf1\\n1.bmp") ) ) ret = GE_FALSE;
	if( !(nf1[2] = LoadBmp(".\\gfx\\hud\\nf1\\n2.bmp") ) ) ret = GE_FALSE;
	if( !(nf1[3] = LoadBmp(".\\gfx\\hud\\nf1\\n3.bmp") ) ) ret = GE_FALSE;
	if( !(nf1[4] = LoadBmp(".\\gfx\\hud\\nf1\\n4.bmp") ) ) ret = GE_FALSE;
	if( !(nf1[5] = LoadBmp(".\\gfx\\hud\\nf1\\n5.bmp") ) ) ret = GE_FALSE;
	if( !(nf1[6] = LoadBmp(".\\gfx\\hud\\nf1\\n6.bmp") ) ) ret = GE_FALSE;
	if( !(nf1[7] = LoadBmp(".\\gfx\\hud\\nf1\\n7.bmp") ) ) ret = GE_FALSE;
	if( !(nf1[8] = LoadBmp(".\\gfx\\hud\\nf1\\n8.bmp") ) ) ret = GE_FALSE;
	if( !(nf1[9] = LoadBmp(".\\gfx\\hud\\nf1\\n9.bmp") ) ) ret = GE_FALSE;
	if( !(nf1[10] = LoadBmp(".\\gfx\\hud\\nf1\\null.bmp") ) ) ret = GE_FALSE;

	if( !ret ) return ret;

	if( !(nf2[0] = LoadBmp(".\\gfx\\hud\\nf2\\n0.bmp") ) ) ret = GE_FALSE;
	if( !(nf2[1] = LoadBmp(".\\gfx\\hud\\nf2\\n1.bmp") ) ) ret = GE_FALSE;
	if( !(nf2[2] = LoadBmp(".\\gfx\\hud\\nf2\\n2.bmp") ) ) ret = GE_FALSE;
	if( !(nf2[3] = LoadBmp(".\\gfx\\hud\\nf2\\n3.bmp") ) ) ret = GE_FALSE;
	if( !(nf2[4] = LoadBmp(".\\gfx\\hud\\nf2\\n4.bmp") ) ) ret = GE_FALSE;
	if( !(nf2[5] = LoadBmp(".\\gfx\\hud\\nf2\\n5.bmp") ) ) ret = GE_FALSE;
	if( !(nf2[6] = LoadBmp(".\\gfx\\hud\\nf2\\n6.bmp") ) ) ret = GE_FALSE;
	if( !(nf2[7] = LoadBmp(".\\gfx\\hud\\nf2\\n7.bmp") ) ) ret = GE_FALSE;
	if( !(nf2[8] = LoadBmp(".\\gfx\\hud\\nf2\\n8.bmp") ) ) ret = GE_FALSE;
	if( !(nf2[9] = LoadBmp(".\\gfx\\hud\\nf2\\n9.bmp") ) ) ret = GE_FALSE;
	if( !(nf2[10] = LoadBmp(".\\gfx\\hud\\nf2\\null.bmp") ) ) ret = GE_FALSE;

	return ret;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		destroy_hud
//			unload all the images used in "the hud"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void destroy_hud()
{
	int i;

	if( Hud )
	{
		printLog("Removing hud.\n");
		geEngine_RemoveBitmap(Engine, Hud);
		geBitmap_Destroy(&Hud);
	}

	for( i = 0; i< 11; i++)
	{
		if( nf1[i] )
		{
			printLog("Removing nf1.\n");
			geEngine_RemoveBitmap(Engine, nf1[i]);
			geBitmap_Destroy( &(nf1[i]) );
		}
	}

	for( i = 0; i< 11; i++)
	{
		if( nf2[i] )
		{
			printLog("Removing nf2.\n");
			geEngine_RemoveBitmap(Engine, nf2[i]);
			geBitmap_Destroy( &(nf2[i]) );
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		get_number(internal function)
//			returns the input maximum 10, for securing out of boundary array problem/crash
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned char get_number(unsigned char number)
{
	if( number > 9 )
		return 10;
	return number;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////










///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		render_hud
//			renders the hud, that is the right bottom numbers and background
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// nf1 - number font 1 (small)
// nf2 - number font 2 (big)
geBoolean render_hud()
{
	int2 ammo;
	int2 mag;
	int3 hp;
	int3 ap;


	// width x height
	// 800, 600
	// 248 x 48  
	if( !renderBitmap(Hud, Width-248, Height-48) )
	{
		printLog("Failed to draw hud.\n");
		return GE_FALSE;
	}

	// print only if the weapon is valid(it should be)
	if( weapon_ok() )
	{
		ammo = toInt2(weapon_getAmmonution());
		mag = toInt2(weapon_getMagasines());
		hp = toInt3(hero_hit_points );
		ap = toInt3( hero_armor_points );


		if( weapon_hasUnlimitedAmmo() )
		{
			// shaded
			//mag
			if( !renderBitmap(nf1[10], Width-210, Height-20) )	return GE_FALSE;
			if( !renderBitmap(nf1[10], Width-200, Height-20) )	return GE_FALSE;
			//ammo
			if( !renderBitmap(nf1[10], Width-175, Height-35) )	return GE_FALSE;
			if( !renderBitmap(nf1[10], Width-165, Height-35) )	return GE_FALSE;
		}
		else
		{
			//mag
			if( !renderBitmap(nf1[ get_number(mag.ten) ], Width-210, Height-20) )	return GE_FALSE;
			if( !renderBitmap(nf1[ get_number(mag.one) ], Width-200, Height-20) )	return GE_FALSE;

			if( weapon_hasBar() )//draw bar?
			{
				//draw bar
				if( !weapon_drawBar(Width-175, Height-38) ) return GE_FALSE;
			}
			else
			{
				//draw numbers 
				//ammo
				if( !renderBitmap(nf1[ get_number(ammo.ten) ], Width-175, Height-35) )	return GE_FALSE;
				if( !renderBitmap(nf1[ get_number(ammo.one) ], Width-165, Height-35) )	return GE_FALSE;
			}
			
		}

	}

	//render armor
	
	if( !renderBitmap(nf2[ get_number(ap.hundred)], Width-125, Height-27) )	return GE_FALSE;
	if( !renderBitmap(nf2[ get_number(ap.ten) ], Width-110, Height-27) )	return GE_FALSE;
	if( !renderBitmap(nf2[ get_number(ap.one) ], Width-95, Height-27) )	return GE_FALSE;

	//hitpoints
	if( !renderBitmap(nf2[ get_number(hp.hundred) ], Width-55, Height-27) )	return GE_FALSE;
	if( !renderBitmap(nf2[ get_number(hp.ten) ], Width-40, Height-27) )	return GE_FALSE;
	if( !renderBitmap(nf2[ get_number(hp.one) ], Width-25, Height-27) )	return GE_FALSE;

	return GE_TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////









///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// end of file
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////