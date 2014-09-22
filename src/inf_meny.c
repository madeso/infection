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
#include "extra.h"

InfPage menyPages[30];
int pageId = 0;
char inMeny = 1;
int saveIndex=0;

int menyState = 0;
float menyTime=0.0f;
soundsys_sound menyBlip;
soundsys_sound menyTitle;
geWorld* menyWorld=0;
//geBitmap* 
#define BLIP()	soundsys_play_sound(&menyBlip, GE_FALSE, TYPE_STOP, SOUND_SURROUND, 1.0f)

#define UP()	BLIP()
#define DOWN()	BLIP()

// intro
#define MENY_STATE_BKGBIO									0
#define MENY_STATE_BKGBIOINF								1
#define MENY_STATE_ALLWBLOOD								2
#define MENY_STATE_MAINSHADER								3
#define MENY_STATE_DATTI									4
#define MENY_STATE_MENY										5


#define INF_X		100
#define INF_Y		20
#define PRESS_X		250
#define PRESS_Y		500
#define MENY_TIME	4.0f

// MenyImages
geBitmap* mi_bioblur;
geBitmap* mi_biosharp;
geBitmap* mi_blood;
geBitmap* mi_infection;
geBitmap* mi_lines;
geBitmap* mi_press;
geBitmap* mi_bkg;

void meny_enterMeny(){
	inMeny = 1;
}
void meny_leaveMeny(){
	inMeny = 0;
	reset_cursor();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum menyStates{
	MS_MAIN,
		MS_GAME,
		MS_NEW,
		MS_EXIST,
		MS_NAME,
		MS_NAMEDEL, // confirm delete on name
		MS_CREDITS,
		MS_QUIT,
		MS_LOAD,
		MS_OPTIONS,
		MS_GRAPH, // craphics screen
		MS_DRIVER,
		MS_RES, // confirm resolution
		MS_DROK, // confirm driver
		MS_REOK, // confirm resolution
		MS_SOUND,
		MS_SGAME, // game settings
		MS_CONT, // controlls
		MS_MCONT, // mouse controlls
		MS_CP1, // controlls page 1
		MS_INGAME,
		MS_SAVE,
		MS_SNAME,
		MS_IGLOAD,
		MS_UNLOAD,
		MS_REPLACE
};


#define POS_ROW_Y(i)			(104+i*25)
#define POS_ROW_X				48

void meny_initPages();

geBitmap* meny_getBitmap(char *filename){
	//local variables
	//geVFile *File;
	geBitmap *Bitmap = 0;
	
	//printLog("Loading bitmap: ");
	//printLog(filename);
	//printLog("\n");
	
	/*
	File = geVFile_OpenNewSystem(NULL, GE_VFILE_TYPE_DOS, filename, NULL, GE_VFILE_OPEN_READONLY);
	if( !File )	{
	printLog("Couldn't open file.\n");
	return 0;
	}
	Bitmap = geBitmap_CreateFromFile(File);
	geVFile_Close(File);*/
	
	Bitmap = geBitmap_CreateFromFileName(NULL, filename); 
	
	/*
	if( Bitmap ){
	geEngine_AddBitmap(Engine, Bitmap);
}*/
	
	return Bitmap;
}

geBitmap* meny_loadBitamp(char* bmpFile, char* alphaFile){
	geBitmap* normal =0;
	geBitmap* alpha =0;
	
	//alphaFile = 0;
	
	normal = meny_getBitmap(bmpFile);
	if(! normal ) {
		return 0;
	}
	
	if( alphaFile ){
		alpha = meny_getBitmap(alphaFile);
		if(! alpha ){
			geBitmap_Destroy(&normal);
			return 0;
		}
		
		if(! geBitmap_SetAlpha(normal, alpha) ){
			geBitmap_Destroy(&normal);
			geBitmap_Destroy(&alpha);
			return 0;
		}
		
		// we don't need this anymore
		geBitmap_Destroy(&alpha);
	}
	
	//geEngine_AddBitmap(Engine, normal); 
	
	if(! geWorld_AddBitmap(menyWorld, normal ) ){
		geBitmap_Destroy(&normal);
		return 0;
	}
	
	return normal;
}

#define DELETE_BITMAP(bmp) {meny_deleteBitmap(bmp); bmp=0;}
void meny_deleteBitmap(geBitmap* bitmap){
	if( bitmap){
		geWorld_RemoveBitmap( menyWorld, bitmap );
		//geEngine_RemoveBitmap(Engine, bitmap);
		geBitmap_Destroy( &bitmap );
	}
}

void meny_renderBitmap(geBitmap* theBitmap, int x, int y, int w, int h, GE_RGBA* color, float alpha){
	geEngine* theEngine = Engine;
	geCamera* theCamera = Camera;
	GE_RGBA vertsColor[4];
	geRect r = {0,0,0,0};
	r.Left = x;
	r.Top = y;
	r.Right = r.Left + w;
	r.Bottom = r.Top + h;
	
	vertsColor[0] = *color;
	vertsColor[1] = *color;
	vertsColor[2] = *color;
	vertsColor[3] = *color;

	vertsColor[0].a = alpha;
	vertsColor[1].a = alpha;
	vertsColor[2].a = alpha;
	vertsColor[3].a = alpha;
	
	geEngine_DrawAlphaBitmap(theEngine, theBitmap, 0, theCamera, &r, 0, alpha, vertsColor);
	//geEngine_DrawBitmap(theEngine, theBitmap, 0, x, y);
}

geBoolean loadMenyBitmaps(){/*
	// if(!( */
	mi_bioblur = meny_loadBitamp(".\\gfx\\meny\\bioblur.bmp", ".\\gfx\\meny\\abioblur.bmp");// )) return GE_FALSE;
	// if(!( 
	mi_biosharp = meny_loadBitamp(".\\gfx\\meny\\biosharp.bmp", ".\\gfx\\meny\\abiosharp.bmp");// )) return GE_FALSE;
	// if(!( 
	mi_blood = meny_loadBitamp(".\\gfx\\meny\\blood.bmp", ".\\gfx\\meny\\ablood.bmp");// )) return GE_FALSE;
	// if(!( 
	mi_infection = meny_loadBitamp(".\\gfx\\meny\\infection.bmp", ".\\gfx\\meny\\ainfection.bmp");// )) return GE_FALSE;
	// if(!(
	
	// if(!(
	mi_press = meny_loadBitamp(".\\gfx\\meny\\press.bmp", ".\\gfx\\meny\\apress.bmp");// )) return GE_FALSE;
	// if(!( 
	
	mi_lines = meny_loadBitamp(".\\gfx\\meny\\lines.bmp", ".\\gfx\\meny\\alines.bmp");// )) return GE_FALSE;
	mi_bkg = meny_loadBitamp(".\\gfx\\meny\\bkg.bmp", 0) ;//)) return GE_FALSE;
	//mi_bkg = loadBitmapEx(".\\gfx\\initializing.bmp", Width, Height, Engine);
	return GE_TRUE;
}

void meny_deleteImages(){
	DELETE_BITMAP(mi_bioblur);
	DELETE_BITMAP(mi_biosharp);
	DELETE_BITMAP(mi_blood);
	DELETE_BITMAP(mi_infection);
	DELETE_BITMAP(mi_lines);
	DELETE_BITMAP(mi_press);
	DELETE_BITMAP(mi_bkg);
}

geBoolean meny_init(){
	geEngine* theEngine = Engine;
	
	meny_enterMeny();
	// MENY_STATE_BKGBIO MENY_STATE_MENY
	menyState = MENY_STATE_BKGBIO;
	soundsys_loadSound(".\\sfx\\blip.wav",1, &menyBlip, 0, TYPE_2D);
	soundsys_loadSound(".\\sfx\\maintitle.wav", 1, &menyTitle, 0, TYPE_UNAFFECTED);
	menyWorld = geWorld_Create( NULL );
	if(!menyWorld) return GE_FALSE;
	if(! geEngine_AddWorld(theEngine, menyWorld ) ) return GE_FALSE;
	
	if(! loadMenyBitmaps() ) return GE_FALSE;
	
	meny_initPages();
	
	return GE_TRUE;
}

geBoolean meny_mouse = GE_FALSE;
geBoolean meny_mbnew = GE_FALSE;

void meny_update(){
	float timePassed = TIME;
	int temp;
	
	switch( menyState ){
	case MENY_STATE_BKGBIO:
		menyTime += timePassed;
		if( menyTime > 1.5f ){
			menyTime = 0.0f;
			menyState = MENY_STATE_BKGBIOINF;
			soundsys_play_sound(&menyTitle, GE_FALSE, TYPE_STOP, SOUND_SURROUND, 1.0f);
		}
		break;
	case MENY_STATE_BKGBIOINF:
		menyTime += timePassed;
		if( menyTime > 2.2f ){
			menyTime = 0.0f;
			menyState = MENY_STATE_ALLWBLOOD;
			// @@@todo@@@ play sound get ready
		}
		break;
	case MENY_STATE_ALLWBLOOD:
		menyTime += timePassed;
		if( menyTime > 5.0f ){
			menyTime = 0.0f;
			//@@@todo@@@ add splat ???
		}
		break;
	case MENY_STATE_MAINSHADER:
		menyTime += timePassed;
		if( menyTime > MENY_TIME ){
			menyTime = 0.0f;
			menyState = MENY_STATE_MENY;
		}
		break;
	case MENY_STATE_DATTI:
		break;
	case MENY_STATE_MENY:
		InfPage_process(&menyPages[pageId]);
		// handle mouse click events and send the deeper into the logic
		
		temp = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
		meny_mbnew = ( meny_mouse != temp && temp!= 0) ? GE_TRUE : GE_FALSE;
		meny_mouse = temp;
		
		if( meny_mouse && meny_mbnew ){
			InfPage_sendClick(&(menyPages[pageId]));
		}
		break;
	}
}

void meny_onChar(char c){
	menyTime = 0.0f;
	switch( menyState ){
	case MENY_STATE_BKGBIO:
	case MENY_STATE_BKGBIOINF:
	case MENY_STATE_ALLWBLOOD:
		menyState = MENY_STATE_MAINSHADER;
		break;
	case MENY_STATE_MAINSHADER:
	case MENY_STATE_DATTI:
		menyState = MENY_STATE_MENY;
		break;
	case MENY_STATE_MENY:
		InfPage_sendCharacter(&menyPages[pageId], c);
	}
}

void meny_preRedner(){
}




void meny_render(){
	GE_RGBA color;
	color.r = color.g = color.b = 255.0f;
	color.a = 255.0f;
	meny_renderBitmap(mi_bkg, 0, 0, Width, Height, &color, 255.0f);
	
	switch( menyState ){
	case MENY_STATE_BKGBIO:	
		meny_renderBitmap(mi_biosharp, 0, 0, Width, Height, &color, 255);
		break;
	case MENY_STATE_BKGBIOINF:
		meny_renderBitmap(mi_biosharp, 0, 0, Width, Height, &color, 255);
		meny_renderBitmap(mi_infection, INF_X, INF_Y, 600, 100, &color, 255);
		break;
	case MENY_STATE_ALLWBLOOD:
		meny_renderBitmap(mi_biosharp, 0, 0, Width, Height, &color, 255);
		meny_renderBitmap(mi_infection, INF_X, INF_Y, 600, 100, &color, 255);
		meny_renderBitmap(mi_press, PRESS_X, PRESS_Y, 300, 50, &color, 255);
		break;
	case MENY_STATE_MAINSHADER:
		{
			float change = (menyTime/MENY_TIME * 255.0f);
			meny_renderBitmap(mi_biosharp, 0, 0, Width, Height, &color, 255 - change);
			meny_renderBitmap(mi_infection, INF_X, INF_Y, 600, 100, &color, 255 - change);
			meny_renderBitmap(mi_bioblur, 0, 0, Width, Height, &color, change);
			meny_renderBitmap(mi_lines, 0, 0, Width, Height, &color, change);
		}
		break;
	case MENY_STATE_DATTI:
		meny_renderBitmap(mi_bioblur, 0, 0, Width, Height, &color, 255);
		meny_renderBitmap(mi_lines, 0, 0, Width, Height, &color, 255);
		menyState = MENY_STATE_MENY;
		break;
	case MENY_STATE_MENY:
		meny_renderBitmap(mi_bioblur, 0, 0, Width, Height, &color, 255);
		meny_renderBitmap(mi_lines, 0, 0, Width, Height, &color, 255);
		InfPage_render(&menyPages[pageId]);
		break;
	}
	
	//geEngine_RenderWorld(Engine, menyWorld, Camera, 1.0f); 
}

void meny_delete(){
	geEngine* theEngine = Engine;
	
	meny_deleteImages();

	if( menyWorld ){
		geEngine_RemoveWorld( theEngine, menyWorld );
		geWorld_Free( menyWorld );
	}
	menyWorld = NULL;
}

void goBack(){
	int index =0;
	
	//exit_application();
	
	if( inMeny ){
		UP();
		if( pageId==MS_INGAME ) {
			meny_leaveMeny();
		}
		else {
			index = menyPages[pageId].goBackIndex;
			pageId = index;
		}
	}
	else {
		meny_enterMeny();
	}
}

char renderMeny(){
	return inMeny;
}


char player_canCreatePlayer(){
	return 1;
}
int playerIndex = 0;
void player_selectPlayer(int player){
	playerIndex = player;
}
void player_deletePlayer(int player){
}

void meny_userUpdateuserName(){
}
void meny_updateNewPlayerButton(){
}
void meny_updateExistingPlayers(){
}

// main
void menyfn_main_game(){
	UP();
	meny_updateNewPlayerButton();
	pageId = MS_GAME;
}
void menyfn_main_options(){
	UP();
	pageId = MS_OPTIONS;
}
void menyfn_main_credits (){
	UP();
	pageId = MS_CREDITS;
}
void menyfn_main_quit (){
	UP();
	pageId = MS_QUIT;
}

// game
void menyfn_game_new (){
	UP();
	pageId = MS_NEW;
}
void menyfn_game_existing (){
	UP();
	meny_updateExistingPlayers();
	pageId = MS_EXIST;
}
void menyfn_game_back (){
	DOWN();
	pageId = MS_MAIN;
}

// ngame
void menyfn_ngame_namechange (){
}
void menyfn_ngame_easy (){
	if( !menyPages[MS_NEW].list[2].data ){
		UP();
		menyPages[MS_NEW].list[1].data = 1;
		menyPages[MS_NEW].list[2].data = 0;
		menyPages[MS_NEW].list[3].data = 0;
	}
}
void menyfn_ngame_medium (){
	if( !menyPages[MS_NEW].list[3].data ){
		UP();
		menyPages[MS_NEW].list[1].data = 0;
		menyPages[MS_NEW].list[2].data = 1;
		menyPages[MS_NEW].list[3].data = 0;
	}
}
void menyfn_ngame_hard (){
	if( !menyPages[MS_NEW].list[4].data ){
		UP();
		menyPages[MS_NEW].list[1].data = 0;
		menyPages[MS_NEW].list[2].data = 0;
		menyPages[MS_NEW].list[3].data = 1;
	}
}
void menyfn_ngame_ok (){
	UP();
	setPlayerName(menyPages[MS_NEW].list[1].text);
	meny_userUpdateuserName();
	pageId = MS_NAME;
}
void menyfn_ngame_back (){
	DOWN();
	pageId = MS_GAME;
}

// xgame
void menyfn_xgame_game1 (){
	UP();
	player_selectPlayer(0);
	meny_userUpdateuserName();
	pageId = MS_NAME;
}
void menyfn_xgame_game2 (){
	UP();
	player_selectPlayer(1);
	meny_userUpdateuserName();
	pageId = MS_NAME;
}
void menyfn_xgame_game3 (){
	UP();
	player_selectPlayer(2);
	meny_userUpdateuserName();
	pageId = MS_NAME;
}
void menyfn_xgame_game4 (){
	UP();
	player_selectPlayer(3);
	meny_userUpdateuserName();
	pageId = MS_NAME;
}
void menyfn_xgame_game5 (){
	UP();
	player_selectPlayer(4);
	meny_userUpdateuserName();
	pageId = MS_NAME;
}
void menyfn_xgame_game6 (){
	UP();
	player_selectPlayer(5);
	meny_userUpdateuserName();
	pageId = MS_NAME;
}
void menyfn_xgame_back (){
	DOWN();
	pageId = MS_GAME;
}

// name
void menyfn_name_new (){
	UP();
	new_game();
	pageId = MS_INGAME;
	meny_leaveMeny();
}
void menyfn_name_load (){
	UP();
	pageId = MS_LOAD;
}
void menyfn_name_delete (){
	UP();
	pageId = MS_NAMEDEL;
}
void menyfn_name_back (){
	DOWN();
	meny_updateNewPlayerButton();
	pageId = MS_GAME;
}

// namedel
void menyfn_namedel_yes (){
	UP();
	player_deletePlayer(playerIndex);
	meny_updateNewPlayerButton();
	pageId = MS_GAME;
}
void menyfn_namedel_no (){
	DOWN();
	pageId = MS_NAME;
}

// load
void menyfn_load_number1 (){
	UP();
	load_game(1);
	pageId = MS_INGAME;
	meny_leaveMeny();
}
void menyfn_load_number2 (){
	UP();
	load_game(2);
	pageId = MS_INGAME;
	meny_leaveMeny();
}
void menyfn_load_number3 (){
	UP();
	load_game(3);
	pageId = MS_INGAME;
	meny_leaveMeny();
}
void menyfn_load_number4 (){
	UP();
	load_game(4);
	pageId = MS_INGAME;
	meny_leaveMeny();
}
void menyfn_load_number5 (){
	UP();
	load_game(5);
	pageId = MS_INGAME;
	meny_leaveMeny();
}
void menyfn_load_number6 (){
	UP();
	load_game(6);
	pageId = MS_INGAME;
	meny_leaveMeny();
}
void menyfn_load_quick (){
	UP();
	pageId = MS_INGAME;
	quick_load();
	meny_leaveMeny();
}
void menyfn_load_level (){
	UP();
	pageId = MS_INGAME;
	level_load();
	meny_leaveMeny();
}
void menyfn_load_back (){
	DOWN();
	if( World ) pageId = MS_INGAME;
	else pageId = MS_NAME;
}

// credits
void menyfn_credits_back (){
	DOWN();
	pageId = MS_MAIN;
}

// options
void menyfn_options_graph (){
	UP();
	pageId = MS_GRAPH;
}
void menyfn_options_sound (){
	UP();
	pageId = MS_SOUND;
}
void menyfn_options_game (){
	UP();
	pageId = MS_SGAME;
}
void menyfn_options_back (){
	DOWN();
	pageId = MS_MAIN;
}

// graphics
void menyfn_graphics_driver (){
	UP();
	pageId = MS_DRIVER;
}
void menyfn_graphics_resolution (){
	UP();
	pageId = MS_RES;
}
void menyfn_graphics_back (){
	DOWN();
	pageId = MS_OPTIONS;
}

// driver
void menyfn_drv_direct (){
	UP();
	ourdriver = '(';
	pageId = MS_DROK;
}
void menyfn_drv_opengl (){
	UP();
	ourdriver = 'O';
	pageId = MS_DROK;
}
void menyfn_drv_wire (){
	UP();
	ourdriver = 'W';
	pageId = MS_DROK;
}
void menyfn_drv_back (){
	DOWN();
	pageId = MS_GRAPH;
}

// resolution
void menyfn_res_low (){
	UP();
	Width = 640;
	Height = 480;
	pageId = MS_REOK;
}
void menyfn_res_medium (){
	UP();
	Width = 800;
	Height = 600;
	pageId = MS_REOK;
}
void menyfn_res_high (){
	UP();
	Width = 960;
	Height = 720;
	pageId = MS_REOK;
}
void menyfn_res_extreme (){
	UP();
	Width = 1024;
	Height = 768;
	pageId = MS_REOK;
}
void menyfn_res_back (){
	DOWN();
	pageId = MS_GRAPH;
}

// game
void menyfn_sgame_crosshair (){
	UP();
	// @@@todo@@@
}
void menyfn_sgame_pickup (){
	UP();
	// @@@todo@@@
}
void menyfn_sgame_controls (){
	UP();
	pageId = MS_CONT;
}
void menyfn_sgame_quicksaveatload (){
	UP();
	/// @@@todo@@@
}
void menyfn_sgame_back (){
	DOWN();
	pageId = MS_OPTIONS;
}

// menyfn
void menyfn_toGraph(){
	UP();
	pageId = MS_GRAPH;
}

// controls
void menyfn_cont_mouse (){
	UP();
	pageId = MS_MCONT;
}
void menyfn_cont_keys (){
	UP();
	pageId = MS_CP1;
}
void menyfn_cont_back (){
	DOWN();
	if( World ) pageId = MS_INGAME;
	else pageId = MS_GAME;
}

// mcont
void menyfn_mcont_invert (){
	UP();
}
// @@@todo@@@ more controls
void menyfn_mcont_back (){
	DOWN();
	pageId = MS_CONT;
}

// cp1
//@@@todo@@@ more pages to cover all controls
void menyfn_cp1_back (){
	DOWN();
	pageId = MS_CONT;
}

// sound
void menyfn_sound_volume (){
	//@@@todo@@@
	UP();
}
void menyfn_sound_mute (){
	//@@@todo@@@
	UP();
}
void menyfn_sound_back (){
	DOWN();
	pageId = MS_OPTIONS;
}
// @@@todo@@@ load music files?

// ingame
void menyfn_ingame_save (){
	UP();
	pageId = MS_SAVE;
}
void menyfn_ingame_load (){
	UP();
	pageId = MS_IGLOAD;
}
void menyfn_ingame_leave (){
	UP();
	pageId = MS_UNLOAD;
}
void menyfn_ingame_continue (){
	DOWN();
	meny_leaveMeny();
}

// save
void menyfn_save_number1 (){
	UP();
	saveIndex = 1;
	if( test_saveIndex(saveIndex) ) pageId = MS_SNAME;
	else pageId = MS_REPLACE;
}
void menyfn_save_number2 (){
	UP();
	saveIndex = 2;
	if( test_saveIndex(saveIndex) ) pageId = MS_SNAME;
	else pageId = MS_REPLACE;
}
void menyfn_save_number3 (){
	UP();
	saveIndex = 3;
	if( test_saveIndex(saveIndex) ) pageId = MS_SNAME;
	else pageId = MS_REPLACE;
}
void menyfn_save_number4 (){
	UP();
	saveIndex = 4;
	if( test_saveIndex(saveIndex) ) pageId = MS_SNAME;
	else pageId = MS_REPLACE;
}
void menyfn_save_number5 (){
	UP();
	saveIndex = 5;
	if( test_saveIndex(saveIndex)) pageId = MS_SNAME;
	else pageId = MS_REPLACE;
}
void menyfn_save_number6 (){
	UP();
	saveIndex = 6;
	if( test_saveIndex(saveIndex)) pageId = MS_SNAME;
	else pageId = MS_REPLACE;
}
void menyfn_save_quick (){
	UP();
	quick_save();
	meny_leaveMeny();
	pageId = MS_INGAME;
}
void menyfn_save_back (){
	DOWN();
	pageId = MS_INGAME;
}

// save name
void menyfn_sname_edit (){
}
void menyfn_sname_ok (){
	UP();
	save_game(saveIndex);
	meny_leaveMeny();
	pageId = MS_INGAME;
}
void menyfn_sname_cancel (){
	DOWN();
	pageId = MS_SAVE;
}

// igload
void menyfn_igload_continue (){
	UP();
	pageId = MS_LOAD;
}
void menyfn_igload_cancel (){
	DOWN();
	pageId = MS_INGAME;
}

// quit
void menyfn_quit_yes(){
	//DOWN();
	exit_application();
}
void menyfn_quit_no (){
	UP();
	pageId = MS_MAIN;
}

// unload
void menyfn_unload_yes (){
	UP();
	LeaveLevel();
	pageId = MS_MAIN;
}
void menyfn_unload_no (){
	DOWN();
	pageId = MS_INGAME;
}

// replace
void menyfn_replace_yes (){
	UP();
	//@@@todo@@@ replace save
	pageId = MS_SNAME;
}
void menyfn_replace_no (){
	DOWN();
	//@@@todo@@@
	pageId = MS_SAVE;
}




void meny_initPages(){
	InfPage* page;
	int index;
	
	page = &(menyPages[MS_MAIN]);
	InfPage_init(page);
	
	/// main /////////////////////////////////////////////////////
	page->title = "Main";
	page->goBackIndex = MS_QUIT;
	index = 0;
	InfPage_setGui(page, index, "Play", "Start playing Infection", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_main_game ); index++;
	InfPage_setGui(page, index, "Options", "Change the various options in Infection", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_main_options); index++;
	InfPage_setGui(page, index, "Credits", "Who created the Infection?", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_main_credits); index++;
	InfPage_setGui(page, index, "Quit", "Quit playing Infection", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_main_quit); index++;
	
	/// game /////////////////////////////////////////////////////
	page = &menyPages[MS_GAME];
	InfPage_init(page);
	page->title = "Players";
	page->goBackIndex = MS_QUIT;
	index = 0;
	InfPage_setGui(page, index, "New player", "Create a new player", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_game_new); index++;
	InfPage_setGui(page, index, "Existing ", "Choos an existing player", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_game_existing); index++;
	InfPage_setGui(page, index, "Back", "Go back to main menu", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_game_back); index++;
	
	/// new //////////////////////////////////////////////////////
	page = &menyPages[MS_NEW];
	InfPage_init(page);
	page->title = "New player";
	page->goBackIndex = MS_GAME;
	index = 0;
	InfPage_setGui(page, index, "Name:", "Type your name", POS_ROW_X, POS_ROW_Y(index), IG_EDIT, 0, menyfn_ngame_namechange); index++;
	InfPage_setGui(page, index, "Skill: easy", "Select the easy skill level", POS_ROW_X, POS_ROW_Y(index), IG_CHECK, 0, menyfn_ngame_easy); index++;
	InfPage_setGui(page, index, "Skill: medium", "Select the medium skill level", POS_ROW_X, POS_ROW_Y(index), IG_CHECK, 0, menyfn_ngame_medium); index++;
	InfPage_setGui(page, index, "Skill: hard", "Select the hard skill level", POS_ROW_X, POS_ROW_Y(index), IG_CHECK, 0, menyfn_ngame_hard); index++;
	InfPage_setGui(page, index, "Ok", "Create the player", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_ngame_ok); index++;
	InfPage_setGui(page, index, "Back", "Go back to the player menu", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_ngame_back); index++;
	
	/// exist ////////////////////////////////////////////////////
	page = &menyPages[MS_EXIST];
	InfPage_init(page);
	page->title = "Existing";
	page->goBackIndex = MS_GAME;
	index = 0;
	InfPage_setGui(page, index, "Player1", "Select the player", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_xgame_game1); index++;
	InfPage_setGui(page, index, "Player2", "Select the player", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_xgame_game2); index++;
	InfPage_setGui(page, index, "Player3", "Select the player", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_xgame_game3); index++;
	InfPage_setGui(page, index, "Player4", "Select the player", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_xgame_game4); index++;
	InfPage_setGui(page, index, "Player5", "Select the player", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_xgame_game5); index++;
	InfPage_setGui(page, index, "Player6", "Select the player", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_xgame_game6); index++;
	InfPage_setGui(page, index, "Back", "Go back to the player menu", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_xgame_back); index++;
	
	/// name /////////////////////////////////////////////////////
	page = &menyPages[MS_NAME];
	InfPage_init(page);
	page->title = "Name";
	page->goBackIndex = MS_GAME;
	index = 0;
	InfPage_setGui(page, index, "New", "Create a new game", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_name_new); index++;
	InfPage_setGui(page, index, "Load", "Load an existing game", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_name_load); index++;
	InfPage_setGui(page, index, "Delete", "Delete this user", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_name_delete); index++;
	InfPage_setGui(page, index, "Back", "Go back to the player menu", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_name_back); index++;
	
	/// namedel //////////////////////////////////////////////////
	page = &menyPages[MS_NAMEDEL];
	InfPage_init(page);
	page->title = "Are you sure you want to delete this player?";
	page->goBackIndex = MS_NAME;
	index = 0;
	InfPage_setGui(page, index, "Yes", "Confirm deleting of the current player(all data will be lost)", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_namedel_yes); index++;
	InfPage_setGui(page, index, "No", "Abort the deleting (no data will be lost)", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_namedel_no); index++;
	
	/// credits //////////////////////////////////////////////////
	page = &menyPages[MS_CREDITS];
	InfPage_init(page);
	page->title = "Credits";
	page->goBackIndex = MS_MAIN;
	index = 0;
	InfPage_setGui(page, index, "Programming: Gustav", "Gustav Jansson", POS_ROW_X, POS_ROW_Y(index), IG_LABEL, 0, 0); index++;
	InfPage_setGui(page, index, "Art: Gustav", "Gustav Jansson", POS_ROW_X, POS_ROW_Y(index), IG_LABEL, 0, 0); index++;
	InfPage_setGui(page, index, "3d: Various", "Various artists - see the readme", POS_ROW_X, POS_ROW_Y(index), IG_LABEL, 0, 0); index++;
	InfPage_setGui(page, index, "Back", "Go back to main", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_credits_back); index++;
	
	/// quit /////////////////////////////////////////////////////
	page = &menyPages[MS_QUIT];
	InfPage_init(page);
	page->title = "Quit?";
	page->goBackIndex = MS_MAIN;
	index = 0;
	InfPage_setGui(page, index, "Yes", "Plase quit the game", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_quit_yes); index++;
	InfPage_setGui(page, index, "No", "Continue playing infection", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_quit_no); index++;
	
	/// load /////////////////////////////////////////////////////
	page = &menyPages[MS_LOAD];
	InfPage_init(page);
	page->title = "Load a game";
	page->goBackIndex = MS_NAME;
	index = 0;
	InfPage_setGui(page, index, "Load1", "Load #1", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_load_number1); index++;
	InfPage_setGui(page, index, "Load2", "Load #2", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_load_number2); index++;
	InfPage_setGui(page, index, "Load3", "Load #3", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_load_number3); index++;
	InfPage_setGui(page, index, "Load4", "Load #4", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_load_number4); index++;
	InfPage_setGui(page, index, "Load5", "Load #5", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_load_number5); index++;
	InfPage_setGui(page, index, "Load6", "Load #6", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_load_number6); index++;
	InfPage_setGui(page, index, "Load quicksave", "Load the latest quicksave", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_load_quick); index++;
	InfPage_setGui(page, index, "Load levelsave", "Load the latest levelsave", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_load_level); index++;
	InfPage_setGui(page, index, "Back", "Go back to the previous menu", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_load_back); index++;
	
	/// options //////////////////////////////////////////////////
	page = &menyPages[MS_OPTIONS];
	InfPage_init(page);
	page->title = "Options";
	page->goBackIndex = MS_MAIN;
	index = 0;
	InfPage_setGui(page, index, "Graphics", "Change graphics options", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_options_graph); index++;
	InfPage_setGui(page, index, "Sound", "Change sound options", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_options_sound); index++;
	InfPage_setGui(page, index, "Game", "Change game options", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_options_game); index++;
	InfPage_setGui(page, index, "Back", "Go back to the previous menu", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_options_back); index++;
	
	/// graph ////////////////////////////////////////////////////
	page = &menyPages[MS_GRAPH];
	InfPage_init(page);
	page->title = "Graphics";
	page->goBackIndex = MS_OPTIONS;
	index = 0;
	InfPage_setGui(page, index, "Driver", "Change the Driver", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_graphics_driver); index++;
	InfPage_setGui(page, index, "Resolution", "Change the Resolution", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_graphics_resolution); index++;
	InfPage_setGui(page, index, "Back", "Go back to options menu", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_graphics_back); index++;
	
	/// driver ///////////////////////////////////////////////////
	page = &menyPages[MS_DRIVER];
	InfPage_init(page);
	page->title = "Select a driver:";
	page->goBackIndex = MS_GRAPH;
	index = 0;
	InfPage_setGui(page, index, "Direct X", "Select DirectX driver", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_drv_direct); index++;
	InfPage_setGui(page, index, "Open GL", "Select OpenGL wire", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_drv_opengl); index++;
	InfPage_setGui(page, index, "Driect X (wireframe)", "Select the wire driver", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_drv_wire); index++;
	InfPage_setGui(page, index, "Cancel", "Go back to Graphics", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_drv_back); index++;
	
	/// res //////////////////////////////////////////////////////
	page = &menyPages[MS_RES];
	InfPage_init(page);
	page->title = "Select a resolution";
	page->goBackIndex = MS_GRAPH;
	index = 0;
	InfPage_setGui(page, index, "Low", "Set resolution to low ()", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_res_low); index++;
	InfPage_setGui(page, index, "Medium", "Set resolution to medium (800*600)", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_res_medium); index++;
	InfPage_setGui(page, index, "High", "Set resolution to high ()", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_res_high); index++;
	InfPage_setGui(page, index, "Extreme", "Set resolution to extreme ()", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_res_extreme); index++;
	InfPage_setGui(page, index, "Cancel", "", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_res_back); index++;
	
	/// drok /////////////////////////////////////////////////////
	page = &menyPages[MS_DROK];
	InfPage_init(page);
	page->title = "Confirm driver..";
	page->goBackIndex = MS_DRIVER;
	index = 0;
	InfPage_setGui(page, index, "You'll have to restart infection for theese changes to be valid", "", POS_ROW_X, POS_ROW_Y(index), IG_LABEL, 0, 0); index++;
	InfPage_setGui(page, index, "Continue", "Continue playing infection", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_toGraph); index++;
	
	/// reok /////////////////////////////////////////////////////
	page = &menyPages[MS_REOK];
	InfPage_init(page);
	page->title = "Confirm resolution";
	page->goBackIndex = MS_RES;
	index = 0;
	InfPage_setGui(page, index, "Yo'll have to restart infections for theese changes to be valid", "", POS_ROW_X, POS_ROW_Y(index), IG_LABEL, 0, 0); index++;
	InfPage_setGui(page, index, "Continue", "Continue playing infection", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_toGraph); index++;
	
	/// sound ////////////////////////////////////////////////////
	page = &menyPages[MS_SOUND];
	InfPage_init(page);
	page->title = "Sound";
	page->goBackIndex = MS_OPTIONS;
	index = 0;
	InfPage_setGui(page, index, "Volume: 1", "Click to change", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_sound_volume); index++;
	InfPage_setGui(page, index, "Mute", "Click to change the use", POS_ROW_X, POS_ROW_Y(index), IG_CHECK, 0, menyfn_sound_mute); index++;
	InfPage_setGui(page, index, "Back", "Go back to options menu", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_sound_back); index++;
	
	/// sgame ////////////////////////////////////////////////////
	page = &menyPages[MS_SGAME];
	InfPage_init(page);
	page->title = "Game";
	page->goBackIndex = MS_OPTIONS;
	index = 0;
	InfPage_setGui(page, index, "Crosshair", "Change the crosshair", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_sgame_crosshair); index++;
	InfPage_setGui(page, index, "Change weapon on pickup", "Click to change", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_sgame_pickup); index++;
	InfPage_setGui(page, index, "Controls", "Change the controls", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_sgame_controls); index++;
	InfPage_setGui(page, index, "Quickave at levelchange", "Click to change", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_sgame_quicksaveatload); index++;
	InfPage_setGui(page, index, "Back", "Go back to options menu", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_sgame_back); index++;
	
	/// cont /////////////////////////////////////////////////////
	page = &menyPages[MS_CONT];
	InfPage_init(page);
	page->title = "Controls";
	page->goBackIndex = MS_GAME;
	index = 0;
	InfPage_setGui(page, index, "Mouse", "Change the mouse motion", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_cont_mouse); index++;
	InfPage_setGui(page, index, "Keys", "Change the key controls", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_cont_keys); index++;
	InfPage_setGui(page, index, "Back", "Go back to Game menu", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_cont_back); index++;
	
	/// mouse cont ///////////////////////////////////////////////
	page = &menyPages[MS_MCONT];
	InfPage_init(page);
	page->title = "Mouse";
	page->goBackIndex = MS_CONT;
	index = 0;
	InfPage_setGui(page, index, "Invert mouse", "Click to change", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_mcont_invert); index++;
	//InfPage_setGui(page, index, "", "", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_); index++;
	//InfPage_setGui(page, index, "", "", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_); index++;
	InfPage_setGui(page, index, "Back", "Go back to controls menu", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_mcont_back); index++;
	
	/// cp1 //////////////////////////////////////////////////////
	page = &menyPages[MS_CP1];
	InfPage_init(page);
	page->title = "Keys (1)";
	page->goBackIndex = MS_CONT;
	index = 0;
	//InfPage_setGui(page, index, "", "", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_); index++;
	//InfPage_setGui(page, index, "", "", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_); index++;
	//InfPage_setGui(page, index, "", "", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_); index++;
	InfPage_setGui(page, index, "Back", "Go back to controls menu", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_cp1_back); index++;
	
	//*
	/// ingame ///////////////////////////////////////////////////
	page = &menyPages[MS_INGAME];
	InfPage_init(page);
	page->title = "In game";
	page->goBackIndex = MS_UNLOAD;
	index = 0;
	InfPage_setGui(page, index, "Save", "Save the game", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_ingame_save); index++;
	InfPage_setGui(page, index, "Load", "Load a game", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_ingame_load); index++;
	InfPage_setGui(page, index, "Continue", "Continue playing infection", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_ingame_continue); index++;
	InfPage_setGui(page, index, "Exit", "Exit to main menu", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_ingame_leave); index++;
	
	/// save /////////////////////////////////////////////////////
	page = &menyPages[MS_SAVE];
	InfPage_init(page);
	page->title = "Save";
	page->goBackIndex = MS_INGAME;
	index = 0;
	InfPage_setGui(page, index, "Save1", "Save #1", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_save_number1); index++;
	InfPage_setGui(page, index, "Save2", "Save #2", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_save_number2); index++;
	InfPage_setGui(page, index, "Save3", "Save #3", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_save_number3); index++;
	InfPage_setGui(page, index, "Save4", "Save #4", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_save_number4); index++;
	InfPage_setGui(page, index, "Save5", "Save #5", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_save_number5); index++;
	InfPage_setGui(page, index, "Save6", "Save %6", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_save_number6); index++;
	InfPage_setGui(page, index, "Quicksave", "Quicksave this game", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_save_quick); index++;
	InfPage_setGui(page, index, "Back", "Go back to ingame menu", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_save_back); index++;
	
	/// sname ////////////////////////////////////////////////////
	page = &menyPages[MS_SNAME];
	InfPage_init(page);
	page->title = "Svae as..";
	page->goBackIndex = MS_SAVE;
	index = 0;
	InfPage_setGui(page, index, "Name:", "The name of the save", POS_ROW_X, POS_ROW_Y(index), IG_EDIT, 0, menyfn_sname_edit); index++;
	InfPage_setGui(page, index, "Ok", "Save the game", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_sname_ok); index++;
	InfPage_setGui(page, index, "Cancel", "Abort the saving and choos another", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_sname_cancel); index++;
	
	/// igload ///////////////////////////////////////////////////
	page = &menyPages[MS_IGLOAD];
	InfPage_init(page);
	page->title = "Current game will no be saved";
	page->goBackIndex = MS_INGAME;
	index = 0;
	InfPage_setGui(page, index, "Continue", "Continue the loading process (can be canseled)", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_igload_continue); index++;
	InfPage_setGui(page, index, "Abort", "Abort loading and go back to ingame menu", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_igload_cancel); index++;
	
	/// unload ///////////////////////////////////////////////////
	page = &menyPages[MS_UNLOAD];
	InfPage_init(page);
	page->title = "Are you want to leave?";
	page->goBackIndex = MS_INGAME;
	index = 0;
	InfPage_setGui(page, index, "Yes", "Leave the game (current game will not be saved)", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_unload_yes); index++;
	InfPage_setGui(page, index, "No", "Continue playing", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_unload_no); index++;
	
	/// replace //////////////////////////////////////////////////
	page = &menyPages[MS_REPLACE];
	InfPage_init(page);
	page->title = "Do you want to replace that save?";
	page->goBackIndex = MS_SAVE;
	index = 0;
	InfPage_setGui(page, index, "Yes", "Continue saving(The game will be replaced)", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_replace_yes); index++;
	InfPage_setGui(page, index, "No", "Go back to the save menu", POS_ROW_X, POS_ROW_Y(index), IG_BUTTON, 0, menyfn_replace_no); index++;
	//*/
}