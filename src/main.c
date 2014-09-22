/***************************************************
*               main.c                             *
*      written by Gustav 'sirGustav' Jansson       *
*         for his first 3d game 'Infection'        *
*             started: 2002-07-19                  *
*             ended:                               *
*              mail: sir_gustav@passagen.se        *
*    URL: http://www.geocities.com/sir_gustavs     *
****************************************************/

#include <windows.h>
#include "resource.h"
#include "drawbbox.h"

#include "genesis.h"
#include "inf_message_system.h"
#include "hud.h"
#include "weapons.h"
#include "edit.h"
#include "player.h"
#include "keys.h"
#include "entity_iterate.h"

#include "time_damage.h"
#include "enemies.h"

#include "log.h"
#include "globalGenesis.h"
#include "decals.h"
#include "crosshair.h"
#include "console.h"
#include <time.h>
#include <stdlib.h>
#include "init.h"
#include "infection.h"
#include "damage.h"
#include "timefx.h"
#include "playerSounds.h"
#include "jackTalks.h"

#include "conversation.h"
#include "subtitles.h"
#include "time_out.h"
#include "inf_meny.h"
#include "sound_modder.h"
#include "libraries.h"

#define PLAYER_NORMAL_AIR 20.0f
#define CONSOLE_LINE_SIZE 80

#define VK_OEM_PERIOD	0xBE
#define VK_OEM_COMMA	0xBC

#define MAX_FPS			50.0f

//prototype of the WndPrc function
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);



//console varaibles
geBoolean doConsole;
geBoolean inConsole;

sEditBox inputBuffer;

//new second variables
int pre_sec;
int new_sec;
float lastUpdateTime;
//float realFps;


char needInput(){
	return inConsole;
}

void print_fps(){
	// update the each update fps
	fps = (delta_T == 0) ? 1 : 1/delta_T;


		if( cheats.printFps ){
		printExtended("Printing fps.\n");
		//Print fov
		sprintf(str, "FPS: %.1f", realFps);
		if( geEngine_Printf(Engine, Width/2, 0, str) == GE_FALSE )
		{
			run = 0;
			printLog("***geEngine_Printf() failed when printin fps.\n\n");
			error("geEngine_Printf() failed when printing fps");
		}
	}
}

// render all the basic stuff that alöways should be visible when plying the game
// like effects the world decals etc
void render_basic()
{
	//Render world
	printExtended("Rendering world.\n");
	if( geEngine_RenderWorld(Engine, World, Camera, 0.0f) == GE_FALSE )
	{
		run = 0;
		printLog("***geEngine_RenderWorld() failed.\n\n");
		error("geEngine_RenderWorld() failed");
	}

	if( cheats.bbox ) {
		DrawBoundBox(World, &(XForm.Translation), &(ExtBox.Min), &(ExtBox.Max) );
	}

	if( cheats.debug ){
		static float maxTIME = 0.0f;
		if( TIME > maxTIME ) maxTIME = TIME;
		geEngine_Printf(Engine, 10 , 30 , "%f, %f, %f", Angle.X, Angle.Y, Angle.Z);
		geEngine_Printf(Engine, 10 , 50 , "%f, %f", maxTIME, TIME);
	}
	
	if (EM)
	{
		printExtended("Rendering Effect manager.\n");
		if ( !EM_Tick(EM, TIME * enemyTime) )
		{
			run = 0;
			printLog("***EM_Tick() failed.\n\n");
			error("EM_Tick failed.");
		}
		
		Rain_Tick(EM);
		Sprite_Tick(EM);
		Spout_Tick(EM);
	}
	
	printExtended("sprintf() for fov.\n");
	
	// render global bitmaps: that hasnt a group
	// crosshair
	if( DrawCrossHair() == GE_FALSE )
	{
		run = 0;
		printLog("***DrawCrossHair() failed.\n");
		error("DrawCrossHair() failed");
	}
	
	if( weapon_render() == GE_FALSE )
	{
		run = 0;
		printLog("***weapon_render() failed.\n");
		error("weapon_render() failed");
	}
	
	//render hud
	if( render_hud() == GE_FALSE )
	{
		run = 0;
		printLog("***render_hud() failed when rendering hud.\n\n");
		error("render_hud() failed");
	}
	
	if( render_subTitle() == GE_FALSE )
	{
		run = 0;
		printLog("***render_subTitle() failed when rendering subtitle.\n\n");
		error("render_subTitle() failed");
	}
	//-----------------------------------------------------------------------------------//
}


// render the data that only should be visible when running the game
void render_game()
{
	if (dMgr)
	{
		printExtended("Rendering decals.\n");
		//system_message("Ticking");
		DecalMgr_Tick(dMgr, TIME);
	}

	renderFlash();
	
	//print messages
	if( render_messages() == GE_FALSE )
	{
		run = 0;
		printLog("***geEngine_Printf() failed when printing messages.\n\n");
		error("render_messages() failed");
	}
	
	//-----------------------------------------------------------------------------------//
	//temporary print - before completing hud and the weapons
	//-----------------------------------------------------------------------------------//
	
	// if we need print the weapon name
	if( weapon_ok() && cheats.doPrint)
	{
		geEngine_Printf(Engine, 80 , 50 , weapon_getName());
	}
	
	
	if( cheats.debug ) {
		char str[100];
		sprintf(str, "Pos: %f %f %f ", Pos.X, Pos.Y, Pos.Z);
		geEngine_Printf(Engine, 10 , 10 , str);
	}
	
	
	if( !player_is_alive ) geEngine_Printf(Engine, 210 , 210 , "You died!");
	/*if( keys[VK_SHIFT] ) geEngine_Printf(Engine, 80 , 90 , "Shift");
	else geEngine_Printf(Engine, 80 , 90 , "No shift");*/
	//-----------------------------------------------------------------------------------//
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		render_manager
//			takes care of all the rendering calls
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void render_manager()
{
	if( inConsole ){
		TIME = 0.0f;
	}
	
	printExtended("render basic.\n");
	
	if( !renderMeny() )
		render_basic();
	
	print_fps();

	if( renderMeny() ) {
		meny_render();
	}

	if( inConsole )
	{
		printExtended("render console.\n");
		render_console();
	}
	else if( !renderMeny() )
	{
		printExtended("render game.\n");
		render_game();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned char pre_state;
WNDCLASS wc;
MSG msg;
//time varaibles
struct tm *today;
time_t ltime;
float new_T;
float old_T;
HINSTANCE hGlobalInstance;

void main_init(int nShowCmd){
	//-------------------------------------------
	//          Genesis variables
	//-------------------------------------------
	
	//Time variables
	//new_T = (geFloat) timeGetTime()/1000;
	//old_T = (geFloat) timeGetTime()/1000;
	delta_T = 1.0f;
	
	// run variable, make sure it's on
	run = 1;
	
	//-------------------------------------------
	//          Start the log: error_log.txt
	//-------------------------------------------
	startLog();
	
	printLog("Infection game initialized.\n");
	
	//-------------------------------------------------
	//                   Starting to set up.
	//--------------------------------------------------
	
	//----------------
	//Setting up old_T
	//----------------
	old_T = (geFloat) timeGetTime()/1000;
	printLog("old_T updated.\n");
	
	//load up the preferences
	if (LoadPrefs(&ourdriver, &Width, &Height, &fullscreen, &options) == GE_FALSE)
	{
		ShutDown();
		_exit(-1);
	}
	
	printLog("Genesis3d variables created.\n");
	
	//---------------------------------------------------
	//        Setting up what the app looks like
	//---------------------------------------------------
	
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	
	//Setting up the background
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	
	//Cursor and Icon
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);//Is this nesesarry?
	wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE( IDI_ICON1 ));
	//let our app know the wndproc function
	wc.lpfnWndProc = WndProc;
	
	wc.lpszClassName = "Infection";
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = hGlobalInstance;
	
	printLog("Window class created.\n");
	
	// make sure we are only running this once, and not creating a load of infection-games
	if( FindWindow(wc.lpszClassName, INFECTION_NAME_AND_VERSION) != NULL )
	{
		MessageBox(NULL, "The application is already in action, probably still launching", "Already active", MB_OK | MB_ICONINFORMATION);
		_exit(-1);
	}
	
	//Register the class
	RegisterClass(&wc);
	printLog("Window class registered.\n");
	
	//Create the window
	hWnd = CreateWindowEx(0, wc.lpszClassName, INFECTION_NAME_AND_VERSION, 0, CW_USEDEFAULT, CW_USEDEFAULT, Width, Height, NULL, NULL, hGlobalInstance, NULL);
	
	if(!hWnd)
	{
		MessageBox(NULL, "Could not create main window", "Infection Init error...", 48);
		printLog("Could not create main window.\n");
		error("Couldn't create main window");
		ShutDown();
		_exit(-1);
	}
	
	printLog("Window created.\n");
	
	//Show window
	ShowWindow(hWnd, nShowCmd);
	
	printLog("Showing Window.\n");
	
	//-------------
	//Hiding cursor
	//-------------
	
	
	//We dont wanna se the cursor
	ShowCursor(FALSE);
	printLog("Cursor is now hidden.\n");
	
	//Update Window
	UpdateWindow(hWnd);
	
	printLog("Window updated.\n");
	
	printLog("Calling the Init() function.\n\n");
	
	console_init(); // initialize the console
	
	//Initialize the genesis engine
	if(Init(hWnd) == GE_FALSE)
	{
		//ooops, something went wrong
		error("Failed in init()");
		ShutDown();
		_exit(-1);
	}
	
	printLog("Genesis3d Engine initialized.\n");
	
	//----------------
	//Setting up new_T
	//----------------
	new_T = (geFloat) timeGetTime()/1000;
	printLog("new_T updated.\n");
	
	dojump = GE_TRUE;
	
	printLog("\nEntering main-game loop.\n");
	
	//-----------------
	//update seconds
	//-----------------
	
	time(&ltime);
	today = localtime(&ltime);
	
	new_sec = today->tm_sec;
	pre_sec = today->tm_sec;
	
	
	printLog("Reseting the cursor at the middle.\n");
	SetCursorPos( Width/2, Height/2);
	
	air = PLAYER_NORMAL_AIR;
	doConsole = GE_FALSE;
	inConsole = GE_FALSE;
	
	EditBox_init(&inputBuffer, "", 20, 20);
	
	pre_state = player_getState();
	old_T = new_T = (geFloat) timeGetTime()/1000;
	
	//-----------------------------------------------------
	//                    Main game loop
	//-----------------------------------------------------
	
	fps = 0.0f;
	realFps = 0.0f;
	lastUpdateTime = 0.0f;
}

void update_timing(){
	//-----------------
	//Calculate delta_T
	//-----------------
	
	printExtended("Calculating delta_T.\n");
	
	old_T = new_T;
	new_T = (geFloat) timeGetTime()/1000;
	
	delta_T = new_T - old_T;
	
	
	// delta_T goes down below zero the first few rounds on my testing system
	// this will prevent the stupid/odd things happening to gravity, weapons and
	// other things depending on time, and this will prevent getting damage when
	// smashing the ground in the beginning(has happend sometimes) and other crazy
	// time dependent things...
	// 30 may 2004 - hasn't happend anymore but this should be here just to be on the safe side.
	if( delta_T < 0.0f ) delta_T = 0.0f;
	
	//------------
	//Update new_T
	//------------
	
	printExtended("Updating new_T.\n");
	
	new_T = (geFloat) timeGetTime()/1000;
}

char need_to_render(){
	float current;
	float diff;

	current = timeGetTime()/1000.0f;
	diff = current - lastUpdateTime;
	realFps = 1.0f / diff;
	if( realFps > MAX_FPS ) return 0;
	return 1;
}

void each_second_update(){
	//----------------
	//Update the time
	//----------------
	
	// this is probably bad, but it'll do for now
	
	time(&ltime);
	today = localtime(&ltime);
	
	//update sec
	new_sec = today->tm_sec;
	
	//it has passed a second
	if( pre_sec != new_sec )
	{
		if( !inConsole )
		{
			weapon_evry_sec();
			
			tick_messages();
			
			if( air <= 0.0f && player_is_alive)
			{
				armor_piercing_damage( rand()%6 +1 );
				game_message("Seek air");
			}
		}
		
		//----------------
		pre_sec = new_sec;
	}
}
void beginFrame(){
	//Begin frame
	if( geEngine_BeginFrame(Engine, Camera, (!renderMeny())?options.clearScreen:
	GE_TRUE
	#pragma message("Meny clearing screen by default")
		) == GE_FALSE )	{
		run = 0;
		printLog("***geEngine_BeginFrame() failed.\n\n");
		error( "geEngine_BeginFrame() failed." );
	}
}

void endFrame(){
	//End frame
	if( geEngine_EndFrame(Engine) == GE_FALSE )	{
		run = 0;
		printLog("***geEngine_EndFrame() failed.\n\n");
		error("geEngine_EndFrame() failed");
	}
}

void messagePump(){
	// message pump
	while( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE) ) {
		//save and check if 0, then shutdown is ok, else error has been detected
		if( GetMessage(&msg, NULL, 0, 0) <= 0) {
			run = 0;
			printLog("Recieved shutdown message.\n");
			break;
		}
		
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void changeXForm(){
	printExtended("Xform = I.\n");
	
	//Set xform matrix to the identity matrix
	geXForm3d_SetIdentity( &XForm );
	
	printExtended("Rotating xform.\n");
	
	//Set up the rotation
	geXForm3d_RotateX( &XForm, Angle.X * DEG);
	geXForm3d_RotateY( &XForm, Angle.Y * DEG);
	
	if( hero_hit_points == 0 && player_is_alive) {
		player_is_alive = GE_FALSE;
		Angle.Z = ((rand()%2)?1.0f:-1.0f) * (rand()%450)/10.0f ;
	}
	
	geXForm3d_RotateZ( &XForm, Angle.Z * DEG);
	
	printExtended("XForm = pos.\n");
	
	//set up the position
	XForm.Translation = Pos;
	
	printExtended("Move the camera.\n");
	
	//Set up the camera to the XForm
	geCamera_SetWorldSpaceXForm( Camera, &XForm );
}

void handleGameUpdate(){
	moved = 0;

	apply_gravity();
	player_is_on_ground_setup();
	player_crouchOrStand();
	user_interaction();
	player_frame();
	weapon_bob(moved);
	timedamage_iterate();
	enemy_iterate(World, TIME * enemyTime);
	iterate_entities(&Pos, World);
	conversation_iterate(World, &Pos, TIME *enemyTime );
	talkPower_Change( -1 * TIME * enemyTime ); // changed from heroTime to enemyTime because we want jack to talk more when we are killing more
	timeout_iterate(TIME * heroTime );
	iterateFlash();
	sm_iterate(TIME * heroTime );
}

void handleConsoleUpdate(){
	char c=' ';
	// there should only be valid characters in the inputBuffer,
	// so we should process all of them, and don't look at them
	while( EditBox_xpop(&inputBuffer, &c) )	{
		// backspace ascii
		if( c==0x08 ) {
			console_backspace();
		}
		else {
			if( c != '\n' )	{
				console_put(c);
			}
			else {
				char input[200];
				console_get(input);
				execute_command(input);
			}
		}
	}
}

void handleScreenshotButton(){
	if(keys[ controls.screenshot ] ) {
		if( screenshot() ) {
			printLog("-- Screenshot taken: ");
			printLog(str);
			printLog("!\n");
			sprintf(str, "Screenshot saved as Infection_%d.bmp", shotIndex-1);
			system_message(str);
		}
		else {
			printLog("-- Screenshot not taken, filename: ");
			printLog(str);
			printLog("\n");
			system_message("Screenshot failed");
			error("screenshot() failed");
		}
		keys[ controls.screenshot ] = FALSE;
	}
}

void handleConsoleButton(){
	if( keys[ controls.console ] ) {
		if( !doConsole ) {
			inConsole = !inConsole;
			reset_cursor();
		}
		doConsole = GE_TRUE;
	}
	else {
		doConsole = GE_FALSE;
	}
}
void handlePlayerAir(){
	if( player_getState() == STATE_WATER ) {
		if( air >= 0.00f )	{
			air -= TIME * heroTime;
		}
	}
	else{
		if( air < PLAYER_NORMAL_AIR )
		{
			air+= TIME * heroTime;
		}
	}
}

void handleCursorVisibility(){
	static char previousInMeny=-1;
	char currentMenyState = renderMeny();

	if( currentMenyState != previousInMeny ){
		if( currentMenyState ){
			ShowCursor(TRUE);
		} else {
			ShowCursor(FALSE);
		}
	}

	previousInMeny = currentMenyState;
}

void updateState() {
	unsigned char temp;
	
	temp = player_getState();
	
	if( temp != pre_state ) {
		switch(temp) {
		case STATE_NORMAL:
			switch( pre_state ) {
			case STATE_NORMAL:
				printLog("\nError in state change, temp and pre_state are the same\n\n");
				run = 0;
				break;
			case STATE_WATER:
				if( air > 0.2f ) {
					soundsys_play_sound(&outofwater, GE_FALSE);
				}
				else {
					soundsys_play_sound(&breathing_again, GE_FALSE);
				}
				leave_water();
				break;
			case STATE_LADDER:
				game_message("Ladder release");
				break;
			default:
				break;
			}
			break;
			case STATE_WATER:
				soundsys_play_sound(&splash[ rand()%6 ], GE_FALSE);
				enter_water();
				break;
			case STATE_LADDER:
				game_message("Grabbing ladder");
				/*
				///////////////////////////////////////////////////////////////////////////////////////////////////////////
				geVec3d_Subtract(&Pos, &XForm.Translation, &inLadder);
				inLadder.Y = 0.0f;
				geVec3d_Normalize(&inLadder);
				///////////////////////////////////////////////////////////////////////////////////////////////////////////
				*/
				break;
			default:
				run = 0;
				printLog("\nError in state change:\n");
				printLog("default case of temp variable\n\n");
				break;
		}
		
		pre_state = temp;
	}
}

#define TRACE_DISTANCE 100.0f
void updateCursor(){
	GE_Collision Col;
	geVec3d In;
	geVec3d farPos;

	geXForm3d_GetIn(&XForm,  &In);
	//geCamera_ScreenPointToWorld(Camera, Width/2, Height/2, &In);
	geVec3d_AddScaled(&(XForm.Translation), &In, TRACE_DISTANCE, &farPos); 
	currentCrosshair = options.crosshairNormal;

	if( geWorld_Collision(
		World,
		NULL,
		NULL,
		&( XForm.Translation ),
		&farPos,
		GE_CONTENTS_SOLID_CLIP,
		GE_COLLIDE_ALL,
		0xffffffff,
		NULL,
		NULL,
		&Col
		)//end of function call
		)//end of if
	{
		geBoolean result = 0;

		if(! Col.Model ){
			return;
		}

		result = can_use_entity(Col.Model);
		if( result ) {
			currentCrosshair = options.crosshairOver;
		}
	}
}
#undef TRACE_DISTANCE


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		WinMain
//			Application entry:
//			call all init functions,their launchers, or set the variables
//			do the loop
//			do cleanup
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	geBoolean backKeyDown = GE_FALSE;
	hGlobalInstance = hInstance;
	main_init(nShowCmd);
	
	while(run){
		// limmit the fps
		while( !need_to_render() );

		update_timing();
		handleCursorVisibility();

		if( !renderMeny() ){
			update_player_state();
			set_speed();
			each_second_update();
			updateCursor();
		} else {
			meny_preRedner();
		}
		
		//---------
		//Rendering
		//---------

		// This is the latest update now
		lastUpdateTime = new_T;

		beginFrame();
		render_manager();
		endFrame();
		
		//Update the position
		if( !renderMeny() )
			Pos = XForm.Translation;
		
		//----------------
		// handle updating
		//----------------
		messagePump();
		
		// TEMPORARY
		// handle quiting application..
		if( keys[ controls.back ] ) {
			if( !backKeyDown ){
				goBack();
			}
			backKeyDown = GE_TRUE;
		} else {
			backKeyDown = GE_FALSE;
		}
		// end TEMPROARY
		
		if( !inConsole ) {
			if( !renderMeny() ){
				handleGameUpdate();
				handlePlayerAir();
				updateState();
			} else {
				meny_update();
			}
		}
		else {
			handleConsoleUpdate();
		}
		
		handleScreenshotButton();
		handleConsoleButton();
		
		if( !renderMeny() )
			changeXForm();
		
		printExtended("Mainloop end.\n");
	}//End of main-loop
	
	
	printLog("\nAfter main game-loop:\nAttenpting to shut down:\n");
	
	//Shut down the engine and clean up
	ShutDown();
	
	printLog("Ending application.");
	//End application
	return 1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////







///////////////////////////////////////////////////////////////////////////////////////////////////////////
//		VK2char
//			WndProc helping function
//			converts the VK to a char
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// returns 0 if genesis doesn't accept the parameter, returns c if it does
char getAllowedAscii(char c)
{
	if( 0x20 <= c )
	{
		if( c <= 0x7e )
		{
			return c;
		}
	}
	
	if( c == 0x08 ) return c; // backspace
	
	return 0;
}

/**
MsVC++ gave some compiler errors and warnings on this piece of code I got from gamedev.net, probably since 
the code was written for DirectInput which gets scancodes. I on the other hand get virtual key codes.
I fixed them, but I wonder if it a nice fix or a hack fix. I hope it's the first.
In case anyone wonder here's the url where I got the :
http://www.gamedev.net/reference/programming/features/scan2asc/
**/
static int scan2ascii(DWORD scancode, USHORT* result)
{
    WPARAM vk;
	static unsigned char State[256];
	HKL layout=GetKeyboardLayout(0);
	
	
	if (GetKeyboardState(State)==FALSE)
		return 0;
	vk=MapVirtualKeyEx(scancode,1,layout);
	return ToAsciiEx(vk,scancode,State,result,0,layout);
}

char VK2char(WPARAM vk)
{
	int r;
	unsigned short c[2];
	
	if( vk == VK_RETURN ) return '\n';// return key is pressed
	
	vk = MapVirtualKey(vk, 0);
	if( vk == 0 ) return 0;
	
	r = scan2ascii( vk, c);
	
	if( r )
	{
		if( c[0] > 255 ) return 0; // we don't want to use characters outside of the char range
		return getAllowedAscii( (char) c[0] );
	}
	return 0;
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		WndProc
//			update global key values
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch(iMessage)
	{
	case WM_KEYDOWN:
	/*{
	char str[200];
	sprintf(str, "%i", wParam);
	system_message(str);
	}*/
		keys[wParam] = GE_TRUE;
		
		if( needInput() )
		{
			char c = 'a';
			
			if( wParam == VK_UP ){
				console_up();
			} else if (wParam==VK_DOWN){
				console_down();
			} else {
				c = VK2char(wParam);
				if(c != 0 )	EditBox_push_char(&inputBuffer, c );
			}
		}
		
		return 0;
	case WM_KEYUP:
		keys[wParam] = GE_FALSE;
		return 0;
	case 0x020A://WM_MOUSEWHEEL
		if( (short)HIWORD(wParam) > 0)
		{
			keys[INF_KEY_MW_UP] = TRUE;
			keys[INF_KEY_MW_DOWN] = FALSE;
		} else
			if( (short)HIWORD(wParam) < 0)
			{
				keys[INF_KEY_MW_UP] = FALSE;
				keys[INF_KEY_MW_DOWN] = TRUE;
			} else
			{
				keys[INF_KEY_MW_UP] = FALSE;
				keys[INF_KEY_MW_DOWN] = FALSE;
			}
			return 0;
			// NEHE add-on
	case WM_SYSCOMMAND: // Intercept System Commands
		switch (wParam) // Check System Calls
		{
		case SC_SCREENSAVE:	 // Screensaver Trying To Start?
		case SC_MONITORPOWER: // Monitor Trying To Enter Powersave?
			return 0; // Prevent From Happening
		}
		break;
		
		default:
			return DefWindowProc(hWnd, iMessage, wParam, lParam);
	}
	
	return 1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

