#include "crosshair.h"

#include "log.h"
#include "globalGenesis.h"
#include "player.h"
#include "extra_genesis.h"

#define CROSSHAIR_2D

// bitmaps
geBitmap *CrossHair[NUM_CROSSHAIRS];



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		LoadCrossHair
//			Func that loads the crosshair(s)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
geBoolean LoadCrossHair()
{	
	geBoolean ok = GE_TRUE;
	char file[300];
	int i;

	for( i= 0; i< NUM_CROSSHAIRS; i++ ) CrossHair[i] = 0;

	printLog("Loading bitmaps...\n");

	for( i= 0; i< NUM_CROSSHAIRS; i++ ) {
		sprintf(file, ".\\gfx\\crosshair\\inf_crosshair_%d.bmp", i+1);
		CrossHair[i] = LoadBmp(file);
		if(! CrossHair[i] )
		{
			ok = GE_FALSE;
			sprintf(file, "Failed to load crosshair #%d\n", i+1);
			printLog(file);
			error(file);
		}
	}

	if( ok )
	{
		printLog("Crosshair ok.\n");
	}
	else
	{
		printLog("-Failed to load crosshair.\n");
		run = 0;
		return GE_FALSE;
	}

	return GE_TRUE;
}


float getCrosshairScale(){
	int iw=0, ih=0;
	float wantedScale;

	iw = geBitmap_Width(CrossHair[currentCrosshair]);
	ih = geBitmap_Height(CrossHair[currentCrosshair]);
	wantedScale = (float)400 / (float)(iw*ih);
	return wantedScale/3.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		DrawCrossHair
//			Function that draws the crosshair
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef CROSSHAIR_2D
geBoolean DrawCrossHair()
{
	uint32 x;
	uint32 y;
	x = (Width - geBitmap_Width(CrossHair[currentCrosshair]))/2;
	y = (Height - geBitmap_Height(CrossHair[currentCrosshair]))/2;

	//NULL says that we want to render the whole bitmap
	if( !geEngine_DrawBitmap(Engine, CrossHair[currentCrosshair], NULL, x, y) )
	{
		printLog("Failed to draw crosshair.\n");
		return GE_FALSE;
	}
	return GE_TRUE;
}
#else
geBoolean DrawCrossHair()
{
	GE_Collision Col;
	gePoly* poly=0;
	GE_LVertex v;
	geVec3d crosshairPos;
	geVec3d in;

	geXForm3d_GetIn(&XForm, &in);
	geVec3d_AddScaled(&(XForm.Translation), &in, 80.0f, &crosshairPos); 

	
	if( geWorld_Collision( World, NULL, NULL, 
		&( XForm.Translation ),	&crosshairPos,
		GE_CONTENTS_SOLID_CLIP, GE_COLLIDE_ALL, 0xffffffff,
		NULL, NULL, &Col)//end of function call
		)//end of if
	{
		geVec3d_AddScaled(&(Col.Impact), &(Col.Plane.Normal), 2.0f, &crosshairPos);
	}

	v.u = v.v = 0.0f;
	v.b = v.r = v.g = 255.0f;
	v.a= 200.0f;
	v.X = crosshairPos.X; v.Y = crosshairPos.Y; v.Z = crosshairPos.Z;
	poly = geWorld_AddPolyOnce(World,
		&v,
		1,
		CrossHair[currentCrosshair],
		GE_TEXTURED_POINT,
		GE_RENDER_DEPTH_SORT_BF,
		getCrosshairScale() ); 

	return GE_TRUE;
}
#endif

void Crosshair_onLevelLoad(){
#ifndef CROSSHAIR_2D
	int i=0;
	if( !World) return;
	for(i=0; i< NUM_CROSSHAIRS; i++ ){
		geWorld_AddBitmap(World, CrossHair[i] );
	}
#endif
}

void Crosshair_onLevelLeave(){
#ifndef CROSSHAIR_2D
	int i=0;
	if( !World) return;
	for(i=0; i< NUM_CROSSHAIRS; i++ ){
		geWorld_RemoveBitmap(World, CrossHair[i] );
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		DestraoyBitmaps
//			Function that destroys the cursor bitmap
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DestroyCrosshair()
{
	int i;
	printLog("Destroying bitmaps.\n");
	for( i= 0; i< NUM_CROSSHAIRS; i++ ) {
		if( CrossHair[i] )
		{
			printLog("Removing a crosshair.\n");
			geEngine_RemoveBitmap(Engine, CrossHair[i]);
			geBitmap_Destroy(&CrossHair[i]);
		}
	}
}
