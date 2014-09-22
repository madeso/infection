#include "inf_meny.h"
#include "globalGenesis.h"
#include "inf_actor.h"
#include "genesis.h"
#include "extra_genesis.h"
#include "log.h"
#include "inf_gui.h"
#include "math.h"
#include "player.h"
#include "infection.h"
#include "init.h"
#include "sound_system.h"

#define MAX_GUIS	8

#define GUI_MAIN		0
#define GUI_QUIT		1
#define GUI_GAME_NEW	2
#define GUI_GAME_LOAD	3
#define GUI_GAME_SAVE	4
#define GUI_CREDITS		5
#define GUI_OVERWRITE	6
#define GUI_SKILLSELECT	7

char inGameMeny=0;
geActor* collidedActor = 0;

geBitmap *menyBkg=0;
geBitmap *overlay=0;
geBitmap *menyLogo=0;

geBitmap *lblOptions=0;
geBitmap *lblQuit=0;
geBitmap *lblInfo=0;
geBitmap *lblGame=0;
geBitmap *lblLeave=0;
geBitmap *lblResume=0;

int logoWidth;
GUIHandler* gui[MAX_GUIS];
int currentGui = GUI_MAIN;
soundsys_sound menyBlip;

#define BLIP()	soundsys_play_sound(&menyBlip, GE_FALSE)

geBoolean fn_game(){
	printLog("Game pressed\n");
	BLIP();
	if( World ){
		inGameMeny = 0;
	} else {
		new_game();
		update_timing();
	}
	reset_cursor();
	return GE_TRUE;
}

geBoolean fn_options(){
	printLog("Options pressed\n");
	BLIP();
	return GE_TRUE;
}

geBoolean fn_info(){
	printLog("Info pressed\n");
	BLIP();
	return GE_TRUE;
}

geBoolean fn_quit(){
	if( World ){
		BLIP();
		LeaveLevel();
	} else {
		exit_application();
	}
	return GE_TRUE;
}

geBoolean meny_init(){
	int i=0;
	int32 bw = (int32)(ceil(Width * 0.08f));
	int32 bh = (int32)(ceil(Height * (64.0f/600.0f)) );
	
	menyBkg = loadBitmapEx(".\\gfx\\meny\\bkg.bmp", Width, Height, Engine);
	overlay = loadBitmapExColorKey(".\\gfx\\meny\\overlay.bmp", Width, Height, Engine, 0);
	menyLogo = loadBitmapExColorKey(".\\gfx\\meny\\infection.bmp", 548, 48, Engine, 0);
	logoWidth = 546;//geBitmap_Width(menyLogo);

	if( !menyBkg ) return GE_FALSE;
	if( !overlay ) return GE_FALSE;
	if( !menyLogo ) return GE_FALSE;

	lblOptions = loadBitmapExColorKey(".\\gfx\\meny\\lblOptions.bmp", 136, 20, Engine, 0);
	lblQuit = loadBitmapExColorKey(".\\gfx\\meny\\lblQuit.bmp", 80, 24, Engine, 0);
	lblInfo = loadBitmapExColorKey(".\\gfx\\meny\\lblInfo.bmp", 76, 20, Engine, 0);
	lblGame = loadBitmapExColorKey(".\\gfx\\meny\\lblGame.bmp", 80, 22, Engine, 0);
	lblResume = loadBitmapExColorKey(".\\gfx\\meny\\lblResume.bmp", 116, 18, Engine, 0);
	lblLeave = loadBitmapExColorKey(".\\gfx\\meny\\lblLeave.bmp", 96, 18, Engine, 0);

	if( !lblOptions ) return GE_FALSE;
	if( !lblQuit ) return GE_FALSE;
	if( !lblInfo ) return GE_FALSE;
	if( !lblGame ) return GE_FALSE;

	soundsys_loadWaw(".\\sfx\\blip.wav", &menyBlip);


	for(i=0; i< MAX_GUIS; i++){
		gui[i] = 0;
		gui[i] = gui_init();

		if( !gui[i] ) return GE_FALSE;
	}

	gui_addButton(gui[GUI_MAIN], 220, 346, bw, bh, ".\\gfx\\meny\\btnGame.bmp", ".\\gfx\\meny\\btnpGame.bmp", GE_FALSE, fn_game);
	gui_addButton(gui[GUI_MAIN], 259, 415, bw, bh, ".\\gfx\\meny\\btnOptions.bmp", ".\\gfx\\meny\\btnpOptions.bmp", GE_FALSE, fn_options);
	gui_addButton(gui[GUI_MAIN], 517, 345, bw, bh, ".\\gfx\\meny\\btnInfo.bmp", ".\\gfx\\meny\\btnpInfo.bmp", GE_FALSE, fn_info);
	gui_addButton(gui[GUI_MAIN], 470, 426, bw, bh, ".\\gfx\\meny\\btnQuit.bmp", ".\\gfx\\meny\\btnpQuit.bmp", GE_FALSE, fn_quit);
	
	return GE_TRUE;
}


void meny_update(){
	POINT CursorPos;
	GetCursorPos(&CursorPos);
	if( !fullscreen ){
		ScreenToClient(hWnd,&CursorPos); 
	}
	gui_update(gui[currentGui], CursorPos.x, CursorPos.y, GetAsyncKeyState(VK_LBUTTON) & 0x8000);
}

void meny_preRedner(){
}

void meny_render(){
	if( !geEngine_DrawBitmap(Engine, menyBkg, NULL, 0, 0) )
	{
		printLog("Failed to draw the meny image.\n");
	}
	if( !geEngine_DrawBitmap(Engine, overlay, NULL, 0, 0) )
	{
		printLog("Failed to draw the meny image.\n");
	}

	if( !geEngine_DrawBitmap(Engine, menyLogo, NULL, (int32)((Width-logoWidth)/2.0f), (int32)(0.15f * Height) ) )
	{
		printLog("Failed to draw the meny image.\n");
	}

	if( World ){
		if( !geEngine_DrawBitmap(Engine, lblResume, NULL, 105, 366) ){
			printLog("Failed to draw lblGame.\n");
		}
		if( !geEngine_DrawBitmap(Engine, lblLeave, NULL, 535, 446) ){
			printLog("Failed to draw lblQuit.\n");
		}
	}
	else {
		if( !geEngine_DrawBitmap(Engine, lblGame, NULL, 130, 366) ){
			printLog("Failed to draw lblGame.\n");
		}
		if( !geEngine_DrawBitmap(Engine, lblQuit, NULL, 535, 446) ){
			printLog("Failed to draw lblQuit.\n");
		}
	}

	if( !geEngine_DrawBitmap(Engine, lblOptions, NULL, 120, 445) ){
		printLog("Failed to draw lblOptions.\n");
	}
	if( !geEngine_DrawBitmap(Engine, lblInfo, NULL, 581, 365) ){
		printLog("Failed to draw lblInfo.\n");
	}

	gui_render(gui[currentGui], Engine);
}

#define REMOVE_BITMAP(x) killBitmap(x); x=0

void meny_delete(){
	int i=0;
	if( menyBkg )
	{
		printLog("Removing initImage.\n");
		geEngine_RemoveBitmap(Engine, menyBkg);
		geBitmap_Destroy(&menyBkg);
	}

	if( overlay )
	{
		printLog("Removing initImage.\n");
		geEngine_RemoveBitmap(Engine, overlay);
		geBitmap_Destroy(&overlay);
	}

	REMOVE_BITMAP( lblGame );
	REMOVE_BITMAP( lblInfo );
	REMOVE_BITMAP( lblLeave );
	REMOVE_BITMAP( lblOptions );
	REMOVE_BITMAP( lblQuit );
	REMOVE_BITMAP( lblResume );


	for(i=0; i<MAX_GUIS; i++){
		if( gui[i] ){
			gui_delete(gui[i]);
			gui[i]=0;
		}
	}
}

void goBack(){
#if 0
	if( World ){
		LeaveLevel();
		return;
	}
#endif
	if( World ){
		inGameMeny = !inGameMeny;
		reset_cursor();
	} else {
		exit_application();
	}
}

char renderMeny(){
	if( !World ) return 1;
	if( inGameMeny ) return 1;

	return 0;
}