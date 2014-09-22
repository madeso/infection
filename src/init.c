/***************************************************
*               init.c                             *
*      written by Gustav 'sirGustav' Jansson       *
*         for his first 3d game 'Infection'        *
*             started: 2002-07-19                  *
*             ended:                               *
*              mail: sir_gustav@passagen.se        *
*    URL: http://www.geocities.com/sir_gustavs     *
****************************************************/

#include <windows.h>

#include "genesis.h"
#include "init.h"
#include "sound_system.h"
#include "extra_genesis.h"
#include "globalGenesis.h"
#include "log.h"
#include "playerSounds.h"
#include "decals.h"
#include "inf_message_system.h"
#include "crosshair.h"
#include "hud.h"
#include "keys.h"
#include "time_damage.h"
#include "enemies.h"
#include "entity_iterate.h"
#include "weapons.h"
#include "timefx.h"
#include "options.h"
#include "level.h"
#include "console.h"
#include "globals.h"
#include "time_out.h"
#include "conversation.h"
#include "fxbitmaps.h"
#include "infection.h"
#include "inf_lua.h"
#include "inventory.h"
#include "inf_meny.h"

#define START_SCREEN_NUMBER				0

geBitmap* loadingImage=0;
char levelFileName[200];

char* getLevelFileName(){
	return levelFileName;
}

int handle_Level(SaveFile* file){
	char temp_levelFileName[200];
	strcpy(temp_levelFileName, levelFileName);

 	STRING(levelFileName, "Failed to handle levelFileName");

	if( strcmp(temp_levelFileName, levelFileName) != 0 ){
		if( LoadLevel(levelFileName, 1) == GE_FALSE ){
			cerror("Failed to load level");
			return 0;
		}
		//level_enableLevelFog();
	}

	return 1;
}

// shows the initializing infection image
geBoolean showInitImage()
{
	geBitmap *initImage=0;

	initImage = loadBitmapEx(".\\gfx\\initializing.bmp", Width, Height, Engine);

	if( !initImage )
	{
		printLog("Failed to load the initializing image\n");
	}

	if( geEngine_BeginFrame(Engine, Camera, GE_FALSE) == GE_FALSE )
	{
		printLog("***geEngine_BeginFrame() failed.\n\n");
		return GE_FALSE;
	}
	
	if( !geEngine_DrawBitmap(Engine, initImage, NULL, 0, 0) )
	{
		printLog("Failed to draw the initializing image.\n");
		return GE_FALSE;
	}

	//End frame
	if( geEngine_EndFrame(Engine) == GE_FALSE )
	{
		printLog("***geEngine_EndFrame() failed.\n\n");
		return GE_FALSE;
	}

	if( initImage )
	{
		printLog("Removing initImage.\n");
		geEngine_RemoveBitmap(Engine, initImage);
		geBitmap_Destroy(&initImage);
	}

	return GE_TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		Init
//			setups the engine and calls all the other init functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
geBoolean Init(HWND hWnd)
{
	//set golbal variable
	//i = 1;

	shotIndex = START_SCREEN_NUMBER;

	World = 0;
	Engine = 0;
	DrvSys = 0;
	Camera = 0;
	dMgr = 0;
	EM = 0;

	strcpy(levelFileName, "");
	fxbitmaps_init(); // make sure it's zero

	printLog("IEntitySet and IEntity is now null\n");
	
	IEntity = 0;
	IEntitySet = 0;
	
	//initialize the engine
	Engine = geEngine_Create(hWnd, "Infection", ".\\drivers\\");
	
	if( !Engine )
	{
		MessageBox(hWnd, "Could not create the Genesis Engine Object", "Infection error", 48);
		return GE_FALSE;
	}


	printLog("Setting the gamma.\n");
	if( !geEngine_SetGamma(Engine, options.gamma ) )
	{
		printLog("Failed to set gamma.\n");
		return GE_FALSE;
	}

	printLog("Genesis3d engine initialized.\n");

	//No frame rate counter
	geEngine_EnableFrameRateCounter(Engine, GE_FALSE);

	//Enble the driver system
	DrvSys = geEngine_GetDriverSystem(Engine);
	if( !DrvSys)
	{
		MessageBox(hWnd, "Could not create the Genesis Driver System", "Infection error", 48);
		return GE_FALSE;
	}

	printLog("Genesis3d driver system initialized.\n");

	//Find the driver
	if ( FindDriver() == GE_FALSE )
		return GE_FALSE;//Something went wrong

	printLog("Genesis3d driver and mode found.\n");


	///////////////////////////////////////////////////////////////////////////////////////////////////////
	//show initializing image
	if( !showInitImage() )
	{
		printLog("Failed to show init image, returning false\n");
		return GE_FALSE;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	//Setting up the camera rect
	Rect.Left = 0;
	Rect.Right = Width - 1;
	Rect.Top = 0;
	Rect.Bottom = Height - 1;

	printLog("Camera rect is initialized.\n");

	//2.0f is 90 degree FOV
	fov = 2.0f;

	//Create the camera
	Camera = geCamera_Create(fov, &Rect);

	if(!Camera)
	{
		MessageBox(hWnd, "Could not create the Genesis Camera Object", "Infection Error", 48);
		printLog("Couldn't create Genesis3d camera.\n");
		return GE_FALSE;
	}

	printLog("Genesis3d camera is initialized.\n");

	//Clear the XForm
	geXForm3d_SetIdentity(&XForm);

	//Rotate
	geXForm3d_RotateX(&XForm, 0.0f);
	geXForm3d_RotateY(&XForm, 0.0f);
	geXForm3d_RotateZ(&XForm, 0.0f);

	printLog("XForm rotated.\n");

	printLog("Setting up your speed.\n");
	real_speed = 50.0f;

	printLog("Setting up your initial angle.\n");
	Angle.X = 0.0f;
	Angle.Y = 0.0f;
	Angle.Z = 0.0f;

	//Set XForm to the camera
	geCamera_SetWorldSpaceXForm(Camera, &XForm);

	printLog("Setting XForm to camera finished.\n");

	//Init the bounding box
	EXTBOX_SET(-135.0f);
	geExtBox_Set(&stateBox, -20.0f, -40.0f, -20.0f, 20.0f, 20.0f, 20.0f);
	geExtBox_Set(&minBox, -0.1f, -0.1f, -0.1f, 0.1f, 0.1f, 0.1f);
	

	printLog("ExtBox has been initiated.\n");

	if( !soundsys_load(hWnd ) )
	{
		printLog("Failed to load soundsys.\n\n");
		return GE_FALSE;
	}

	//init messages
	printLog("Initializing messages.\n");
	init_messages();

	//load up the bitmaps
	printLog("Loading bitmaps.\n");
	if (! LoadCrossHair() )
	{
		return GE_FALSE;
	}

	printLog("Loading hud.\n");
	//load the hud
	if (! load_hud() )
	{
		return GE_FALSE;
	}

	printLog("Initializing material storage.\n");
	materialStorage = HT_Create(200);
	if( !materialStorage ){
		error("Failed to allocate material storage");
		return GE_FALSE;
	}
	printLog("Loading material defs.\n");
	if(! HT_Load(materialStorage, "materials.txt") ){
		error("Failed to load data from materials.txt");
		return GE_FALSE;
	}
	printLog("Loading finished.\n");

	// I really should check the return values of theense functions
	//precashe player sounds
	soundsys_loadWaw(".\\sfx\\player\\jump1.wav", &jump[0] );
	soundsys_loadWaw(".\\sfx\\player\\jump2.wav", &jump[1] );
	soundsys_loadWaw(".\\sfx\\player\\jump3.wav", &jump[2] );
	
	soundsys_loadWaw(".\\sfx\\player\\hurt1.wav", &hurt[0] );
	soundsys_loadWaw(".\\sfx\\player\\hurt2.wav", &hurt[1] );
	soundsys_loadWaw(".\\sfx\\player\\hurt3.wav", &hurt[2] );
	soundsys_loadWaw(".\\sfx\\player\\hurt4.wav", &hurt[3] );
	soundsys_loadWaw(".\\sfx\\player\\hurt5.wav", &hurt[4] );
	soundsys_loadWaw(".\\sfx\\player\\hurt6.wav", &hurt[5] );
	soundsys_loadWaw(".\\sfx\\player\\hurt7.wav", &hurt[6] );

	soundsys_loadWaw(".\\sfx\\player\\die1.wav", &die[0] );
	soundsys_loadWaw(".\\sfx\\player\\die2.wav", &die[1] );
	soundsys_loadWaw(".\\sfx\\player\\die3.wav", &die[2] );
	soundsys_loadWaw(".\\sfx\\player\\die4.wav", &die[3] );
	soundsys_loadWaw(".\\sfx\\player\\die5.wav", &die[4] );

	soundsys_loadWaw(".\\sfx\\player\\armor.wav", &armor[0] );
	soundsys_loadWaw(".\\sfx\\player\\armor2.wav", &armor[1] );
	soundsys_loadWaw(".\\sfx\\player\\armor3.wav", &armor[2] );
	soundsys_loadWaw(".\\sfx\\player\\armor4.wav", &armor[3] );
	soundsys_loadWaw(".\\sfx\\player\\armor5.wav", &armor[4] );

	soundsys_loadWaw(".\\sfx\\player\\land1.wav", &land_normal[0] );
	soundsys_loadWaw(".\\sfx\\player\\land2.wav", &land_normal[1] );
	soundsys_loadWaw(".\\sfx\\player\\land3.wav", &land_normal[2] );

	soundsys_loadWaw(".\\sfx\\player\\splash0.wav", &splash[0] );
	soundsys_loadWaw(".\\sfx\\player\\splash1.wav", &splash[1] );
	soundsys_loadWaw(".\\sfx\\player\\splash2.wav", &splash[2] );
	soundsys_loadWaw(".\\sfx\\player\\splash3.wav", &splash[3] );
	soundsys_loadWaw(".\\sfx\\player\\splash4.wav", &splash[4] );
	soundsys_loadWaw(".\\sfx\\player\\splash5.wav", &splash[5] );

	soundsys_loadWaw(".\\sfx\\player\\breathing_again.wav", &breathing_again );
	soundsys_loadWaw(".\\sfx\\player\\outofwater.wav", &outofwater );

	//load bob sounds
	soundsys_loadWaw(".\\sfx\\player\\bob\\_walk1.wav", &bob_normal_right[0] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\_walk3.wav", &bob_normal_right[1] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\_walk5.wav", &bob_normal_right[2] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\_walk7.wav", &bob_normal_right[3] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\_walk9.wav", &bob_normal_right[4] );

	soundsys_loadWaw(".\\sfx\\player\\bob\\_walk0.wav", &bob_normal_left[0] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\_walk2.wav", &bob_normal_left[1] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\_walk4.wav", &bob_normal_left[2] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\_walk6.wav", &bob_normal_left[3] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\_walk8.wav", &bob_normal_left[4] );

	soundsys_loadWaw(".\\sfx\\player\\bob\\water1.wav", &bob_water_right[0] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\water3.wav", &bob_water_right[1] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\water5.wav", &bob_water_right[2] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\water7.wav", &bob_water_right[3] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\water9.wav", &bob_water_right[4] );

	soundsys_loadWaw(".\\sfx\\player\\bob\\water0.wav", &bob_water_left[0] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\water2.wav", &bob_water_left[1] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\water4.wav", &bob_water_left[2] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\water6.wav", &bob_water_left[3] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\water8.wav", &bob_water_left[4] );

	soundsys_loadWaw(".\\sfx\\player\\bob\\ladder0.wav", &bob_ladder_right[0] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\ladder2.wav", &bob_ladder_right[1] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\ladder4.wav", &bob_ladder_right[2] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\ladder6.wav", &bob_ladder_right[3] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\ladder8.wav", &bob_ladder_right[4] );

	soundsys_loadWaw(".\\sfx\\player\\bob\\ladder1.wav", &bob_ladder_left[0] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\ladder3.wav", &bob_ladder_left[1] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\ladder5.wav", &bob_ladder_left[2] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\ladder7.wav", &bob_ladder_left[3] );
	soundsys_loadWaw(".\\sfx\\player\\bob\\ladder9.wav", &bob_ladder_left[4] );

	soundsys_loadWaw(".\\sfx\\boing.wav", &boing );

	// the mugshots
	soundsys_loadWaw(".\\sfx\\player\\mugshots\\cool.wav", &mugshots[0] );
	soundsys_loadWaw(".\\sfx\\player\\mugshots\\iamtheking.wav", &mugshots[1] );
	soundsys_loadWaw(".\\sfx\\player\\mugshots\\iliveforthis.wav", &mugshots[2] );
	soundsys_loadWaw(".\\sfx\\player\\mugshots\\yeah.wav", &mugshots[3] );

#ifdef NDEBUG
	cheats.god = GE_FALSE;
	cheats.printFps = GE_FALSE;
	cheats.debug = GE_FALSE;
#pragma message("Disabling cheats.god and cheats.printFps")
#else
	cheats.god = GE_TRUE;
	cheats.printFps = GE_TRUE;
	cheats.debug = GE_FALSE;
	#pragma message("Enabling cheats.god and cheats.printFps")
#endif
	cheats.ghost = GE_FALSE;
	cheats.unlimited_ammo = GE_FALSE;
	cheats.unlimited_mag = GE_FALSE;
	cheats.materialPrint = GE_FALSE;
	cheats.texturePrint = GE_FALSE;
	cheats.bouncy = GE_FALSE;

	hero_hit_points = 100;
	hero_armor_points = 100;
	PLAYER_JUMP_SPEED = 600.0f;
	landed = GE_FALSE; //true only one round - when the player has landed
	gravity = 1400.0; //the gravity

	//Decal color
	RGBA_Array.r = 0;
	RGBA_Array.g = 0;
	RGBA_Array.b = 0;
	RGBA_Array.a = 180;

	keys[ controls.jump]= GE_FALSE;

	player_is_alive = GE_TRUE;

	loadingImage = loadBitmapEx(".\\gfx\\loading.bmp", Width, Height, Engine);
	if( !loadingImage )
	{
		printLog("Failed to load loading image.\n");
		return GE_FALSE;
	}

	timedamage_init();

	printLog("Initializing weapons.\n");
	if( !weapon_init() )
	{
		printLog("Weapon initializing failed.\n");
		return GE_FALSE;
	}

	printLog("Initializing enemies\n");
	if(! enemy_init() )
	{
		printLog("Enemy initializing failed.\n");
		return GE_FALSE;
	}

	timefx_normal();

	setPlayerName("DefaultPlayer");

	timeout_init();

	if(! inventory_create() ){
		printLog("Failed to create inventory\n");
		return GE_FALSE;
	}

	if(! inf_luaInit() ){
		printLog("Failed to init lua\n");
		return GE_FALSE;
	}

	if( !meny_init() ){
		printLog("Failed to init meny");
		return GE_FALSE;
	}

	
	/*//Load level
	if (LoadLevel( options.startLevel, 0 ) == GE_FALSE)
	{
		return GE_FALSE;
	}*/

	printLog("Init ok.\n");

	return GE_TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////













///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		FindDriver
//			finds a driver and a resolution from the data from the cfg file
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
geBoolean FindDriver()
{
	//temporary vaiables
	char *drvname;
	long tempwidth;
	long tempheight;
	int i = 1;
	
	printLog("Attempting to find a driver.\n");

	//Get the first driver
	Driver = geDriver_SystemGetNextDriver(DrvSys, NULL);

	//Loop through the driver list and find that matches what we loaded into our variables
	while(1)
	{
		//no first driver
		if( !Driver)
		{
			MessageBox(hWnd, "Could not find a valid driver", "Infection Eroor", 48);
			printLog("No valid driver was found.\n");
			return GE_FALSE;
		}
		
		//get the name of the driver
		geDriver_GetName(Driver, (const char**)&drvname); //C++ convertion style

		//Check if the first character matches the driver we loaded into before. If it is, then break the loop
		if(drvname[0] == ourdriver)
			break;

		//if not, get the next one
		Driver = geDriver_SystemGetNextDriver(DrvSys, Driver);
	}

	printLog("Driver found.\n");

	//Get the first mode
	Mode = geDriver_GetNextMode(Driver, NULL);

	printLog("Attempting to find mode.\n");

	//Loop through the list and find that mode that matches the width and height of our variable
	while(1)
	{
		//No mode
		if( !Mode )
		{
			MessageBox(hWnd, "Could not find a valid mode", "Infection error", 48);
			printLog("Could not find a valid mode.\n");
			sprintf(str, "Iterator: %d.\n", i);
			printLog(str);
			return GE_FALSE;
		}
		
		//Get the width and the height of the mode
		geDriver_ModeGetWidthHeight(Mode, &tempwidth, &tempheight);

		//See if it's a match, if so we'll break the loop
		if( fullscreen )
		{
			if(tempwidth == Width && tempheight == Height)
				break;
		}
		else
		{
			if(tempwidth == -1 && tempheight == -1)
				break;
		}

		//if not get the next mode
		i++;
		Mode = geDriver_GetNextMode(Driver, Mode);
	}

	printLog("Mode found.\n");

	if ( !geEngine_SetDriverAndMode(Engine, Driver, Mode) )
	{
		MessageBox(hWnd, "Could not apply driver and mode to the engine", "Infection error", 48);
		printLog("Could not apply mode and driver to the Genesis3d engine.\n");
		return GE_FALSE;
	}
	printLog("Applied driver and mode to the engine successfull.\n");
	printLog("FindDriver() is done.\n");
	return GE_TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////













///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		LoadPrefs
//			Load up alot of preferences if they are aviable, otherwise it uses the default
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
geBoolean LoadPrefs(char *drv, int *width, int *height, char* fullscreen, Options *options)
{
	FILE *f;

	printLog("Trying to open infection.cfg.\n");

	*drv = '(';
	*width = 800;
	*height = 600;
	*fullscreen = 0;

	options->enable_decals = 1;
	options->enable_effects = 1;
	options->extended_log = 1;
	options->log = 1;
	options->gamma = 1.7f;
	options->clearScreen = GE_FALSE;
	options->fogInWater = 0;
	options->meterfog = 5.0f;
	sprintf(options->startLevel, "start.bsp" );
	options->weaponFlash = GE_FALSE;
	options->currentCrosshair = 0;
	options->invertSpeakers = 0;

	mouse_x_sensitivity = real_mouse_x_sensitivity = 1.45f;//mouse_sensitivity
	mouse_y_sensitivity = real_mouse_y_sensitivity = 1.45f;//mouse_sensitivity

	controls.back = VK_ESCAPE;
	controls.backward = 'S';
	controls.console = VK_F1;
	controls.crouch = VK_CONTROL;
	controls.forward = 'W';
	controls.holdwalk = VK_SHIFT;
	controls.jet = 'Q';
	controls.jump = VK_SPACE;
	controls.left = 'A';
	controls.next = INF_KEY_MW_DOWN;
	controls.previous = INF_KEY_MW_UP;
	controls.reload = 'R';
	controls.right = 'D';
	controls.screenshot = VK_F12;
	controls.toggleFlashlight = 'F';
	controls.wc1 = '1';
	controls.wc2 = '2';
	controls.wc3 = '3';
	controls.wc4 = '4';
	controls.wc5 = '5';
	controls.wc6 = '6';
	controls.use = 'E';
	controls.quickSave = VK_F5;
	controls.quickLoad = VK_F6;
	controls.primaryFire = VK_LBUTTON ;
	controls.secondaryFire = VK_RBUTTON;
	

	cheats.doPrint = 0;

	//Open the miniapp.cfg file
	if ( (f = fopen("infection.cfg", "r")) == NULL )
	{
		//MessageBox(hWnd, "Failed to read infection.cfg", "No prefs", MB_OK);
		printLog("Couldn't load infection.cfg, generating standard values.\n");
	}
	else
	{
		char type[200];
		char data[200];
		
		printLog("File open:\n");

		while( !feof(f) )
		{
			// get the type and data

			//type string, one word
			fscanf(f, " %s", type);

			printLog("Got type\n");

			// ignore comments
			if( strcmp(type, "#") == 0 )
			{
				char c;

				// skip to newline
				printLog("Comment: ");
				fscanf(f, "%c", &c);
				while( c!= '\n' && !feof(f) )
				{
					sprintf(str, "%c", c);
					printLog(str);

					fscanf(f, "%c", &c);
				}
				printLog(".\n");

				continue;
			}

			//data string, the rest of the line
			fscanf(f, " %s", data);

			//we got our needed data filled in the strings type and data, now lets process the data.

			//log the instruction
			printLog("Instruction recieved: ");
			sprintf(str, "[%s][%s].\n", type, data);
			printLog(str);

			if( strcmp(type, "<DRIVER>") == 0) {
				sscanf(data, " %c", drv);
				sprintf(str, "Got the driver: %c.\n", *drv);
				printLog(str);
			} else if( strcmp(type, "<WIDTH>") == 0) {
				sscanf(data, "%d", width);
				sprintf(str, "Got the width: %d\n", *width);
				printLog(str);
			} else if( strcmp(type, "<HEIGHT>") == 0) {
				sscanf(data, "%d", height);
				sprintf(str, "Got the height: %d.\n", *height);
				printLog(str);
			} else if( strcmp(type, "<FULLSCREEN>") == 0) {
				int t=0; sscanf(data, " %d", &t );
				*fullscreen = t?1:0;
				sprintf(str, "Got the fullscreen: %d.\n", *fullscreen);
				printLog(str);
			} else if( strcmp(type, "<ENABLE_EFFECTS>") == 0) {
				int t=0; sscanf(data, " %d", &t );
				options->enable_effects = t?1:0;
				sprintf(str, "Got the enable effects: %d.\n", options->enable_effects);
				printLog(str);
			} else if( strcmp(type, "<ENABLE_DECALS>") == 0) {
				int t=0; sscanf(data, " %d", &t );
				options->enable_decals = t?1:0;
				sprintf(str, "Got the enable decals: %d.\n", options->enable_decals);
				printLog(str);
			} else if( strcmp(type, "<EXTENDED_LOG>") == 0) {
				int t=0; sscanf(data, " %d", &t );
				options->extended_log = t?1:0;
				sprintf(str, "* * * * *  Got the extended log: %d * * * * *.\n", options->extended_log);
				printLog(str);
			} else if( strcmp(type, "<LOG>") == 0) {
				int t=0; sscanf(data, " %d", &t );
				options->log = t?1:0;
				sprintf(str, "Got the log: %d.\n", options->log);
				printLog(str);
			} else if( strcmp(type, "<CROSSHAIR>") == 0) {
				sscanf(data, "%d", &(options->currentCrosshair));
				sprintf(str, "Got the crosshair: %d.\n", options->currentCrosshair);
				printLog(str);
			} else if( strcmp(type, "<GAMMA>") == 0) {
				sscanf(data, "%f", &(options->gamma));
				if( options->gamma > 3.0f )
					options->gamma = 3.0f;
				if( options->gamma < 0.0f )
					options->gamma = 0.0f;
				sprintf(str, "Got the gamma: %f.\n", options->gamma);
				printLog(str);
			} else if( strcmp(type, "<CLEAR_SCREEN>") == 0) {
				char cl;
				sscanf(data, "%d", &cl);
				sprintf(str, "Got the clearScreen: %d.\n", cl);
				options->clearScreen = cl?GE_TRUE:GE_FALSE;
				printLog(str);
			} else if( strcmp(type, "<FOG_IN_WATER>") == 0) {
				sscanf(data, "%d", &(options->fogInWater));
				sprintf(str, "Got the fog in water: %d.\n", options->fogInWater);
				printLog(str);
			} else if( strcmp(type, "<INVERT_SPEAKERS>") == 0) {
				sscanf(data, "%d", &(options->invertSpeakers));
				sprintf(str, "Got the invert speakers: %d.\n", options->invertSpeakers);
				printLog(str);
			} else if( strcmp(type, "<START_LEVEL>") == 0) {
				sscanf(data, "%s", options->startLevel); // @@@
				sprintf(str, "Got the startLevel: %s.\n", options->startLevel);
				printLog(str);
			} else if( strcmp(type, "<MOUSE_X_SENSITIVITY>") == 0) {
				sscanf(data, "%f", &real_mouse_x_sensitivity);
				mouse_x_sensitivity  = real_mouse_x_sensitivity;
			} else if( strcmp(type, "<MOUSE_Y_SENSITIVITY>") == 0) {
				sscanf(data, "%f", &real_mouse_y_sensitivity);
				mouse_y_sensitivity = real_mouse_y_sensitivity ;
			} else if( strcmp(type, "<WEAPON_FLASH>") == 0) {
				char cl;
				sscanf(data, "%d", &cl);
				sprintf(str, "Got the weaponFlash: %d.\n", cl);
				options->weaponFlash = cl?GE_TRUE:GE_FALSE;
				printLog(str);
			} else {
				//error message
				printLog("\n\n\n\n\nUnrecognized instruction.\n");
				//close the file
				//fclose(f);
				//return false
				//return GE_FALSE;
				continue;
				// somehow the application recognized the last
				// line in the file as a command in release build,
				// but not in debug - strange
			}
		}

		//close the file
		fclose(f);
		
		printLog("Loading prefs done..\n");
	}

	printLog("\nPrefs:\n");
	sprintf(str, "Driver: %c %d \n", *drv, *drv);
	printLog(str);
	sprintf(str, "Mode: %d", *width);
	printLog(str);
	sprintf(str, "x %d\n\n", *height);
	printLog(str);

	memset(&keys, GE_FALSE, sizeof(geBoolean)*258 );

	return GE_TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






// clears our data, if we have used it
void LeaveLevel()
{
	conversation_endGame();
	delete_entities(World);
	enemy_clearWorld(World);
	Crosshair_onLevelLeave();

	weapon_deapply();

	player_end();

	kill_decals();

	kill_fx();

	if(World)
	{
		printLog("Clearing world\n");
		//geEngine_RemoveWorld(Engine, World);
		//With the previous line uncommented geWorld_Free sometimes crashes: I found this post by Jeff in a search on the g3d board, and when commenting that line there was no crash.
		/*
		This might not be the problem but the geEngine_RemoveWorld() function calls geWorld_Free() so you don't need to call it right after that function. Also, the geEngine_Free() function calls all these functions: 
		geEngine_RemoveAllWorlds(Engine); 
		geEngine_ShutdownFonts(Engine); 
		geEngine_ShutdownDriver(Engine); 
		geEngine_BitmapListShutdown(Engine); 
		geRam_Free(Engine); 
		So you don't need to remove the worlds or shutdown the driver before calling the geEngine_Free() function because it will do it for you.

		Jeff
		http://www.genesis3d.com/forum/viewtopic.php?topic=1002836&forum=3
		*/
		
		printLog("World has been removed from the engine.\n");
		geWorld_Free(World);
		printLog("World is free.\n");

		World = 0;
		global_levelHeader = 0;
	}
}


// moves the player to the start of the level
void LoadLevel_PlayerStart(geWorld *World )
{
	printLog("Loading a EntitySet for Inf_PlayerStart.\n");
	IEntitySet = geWorld_GetEntitySet(World, "Inf_PlayerStart");//search for Inf_PlayerStart s
	
	if( IEntitySet )
	{
		printLog("Loading a entity from the set of Inf_PlayerStart set.\n");
		IEntity = geEntity_EntitySetGetNextEntity(IEntitySet, NULL);//get the next entity
		
		if( IEntity )
		{
			
			Inf_PlayerStart *ps;	
			printLog("-----------------------------\n");
			printLog("Code found a Inf_PlayerStart.\n");
			printLog("\n");
			ps = (Inf_PlayerStart* ) geEntity_GetUserData(IEntity); //Get the data from the IEntity

			if( ps )
			{	
				geVec3d temporaryPosition;
				printLog("Clearing XForm.\n");
				geXForm3d_SetIdentity(&XForm);
				
				//Rotate
				geXForm3d_RotateX(&XForm, 0.0f);
				geXForm3d_RotateY(&XForm, 0.0f);
				geXForm3d_RotateZ(&XForm, 0.0f);
				
				temporaryPosition = ps->origin;
				if( ps->trace ){
					GE_Collision lCol;
					int result = 0;
					
					temporaryPosition.Y -= MAX_TRACE_DIST;

					result = geWorld_Collision(World, &(ExtBox.Min), &(ExtBox.Max), &(ps->origin),&temporaryPosition,GE_CONTENTS_SOLID_CLIP,GE_COLLIDE_ALL,0xffffffff, 0 ,NULL, &lCol);

					if( result ){
						temporaryPosition = lCol.Impact;
					}
				}
				geXForm3d_Translate(&XForm,
					temporaryPosition.X,
					temporaryPosition.Y,
					temporaryPosition.Z);
				Pos = temporaryPosition;
				
				if(! ps->keepWeapons )
				{
					weapon_strip();
				}

				Angle.X = ps->rotation.X;
/////////////////////////////////////////////////////////////////////
				// Does this fix the release-build-rotation bug ?
/////////////////////////////////////////////////////////////////////
#ifndef NDEBUG
				Angle.Y = ps->rotation.Y - 90.0f;
#else
				Angle.Y = ps->rotation.Y - 90.0f;
#endif
////////////////////////////////////////////////////////////////////
				//Angle.Z = 0.0f;

				// Make the Angle valid
				
				if( Angle.X >  80.0f)			Angle.X =  80.0f;
				if( Angle.X < -80.0f)			Angle.X = -80.0f;
				while( Angle.Y >= 360.0f )		Angle.Y -= 360.0f;
				while( Angle.Y <  0.0f   )		Angle.Y += 360.0f;

				
				sprintf(str, "Orientation is (X, Y, Z) = ( %f, %f , %f ).\n", ps->rotation.X, ps->rotation.Y, ps->rotation.Z);
				printLog(str);
				
				
				printLog("-----------------------------\n\n");
				geCamera_SetWorldSpaceXForm( Camera, &XForm );
			}
			else
			{
				error("Inf_PlayerStart(ps) is null: perhaps a old version of the level");
			}
			
		}
		else
		{
			printLog("Couldn't find a Inf_PlayerStart entity.\n");
		}
	}
	else
	{
		printLog("Couldn't find the Inf_PlayerStart set.\n");
	}

}


// loads in the level header
void LoadLevel_LevelHeader(geWorld *World , int fromLoad)
{
	IEntitySet = 0;
	printLog("Loading a EntitySet for Inf_LevelHeader.\n");
	IEntitySet = geWorld_GetEntitySet(World, "Inf_LevelHeader");//search for Inf_LevelHeader s
	
	if( IEntitySet )
	{
		IEntity = 0;
		printLog("Loading a entity from the set.\n");
		IEntity = geEntity_EntitySetGetNextEntity(IEntitySet, NULL);//get the next entity
		
		if( IEntity )
		{
			Inf_LevelHeader *lh = 0;
			lh = (Inf_LevelHeader* ) geEntity_GetUserData(IEntity);

			if( lh )
			{
				global_levelHeader = lh;
				if( !fromLoad ){
					if( !is_string_null(lh->name) )
					{
						sprintf(str, "Jack entered: %s.", lh->name );
						game_message(str);
					}
				}

				if( !is_string_null(lh->author) )
				{
					sprintf(str, "Author: %s.", lh->author );
					console_message(str);
				}

				if( ! fromLoad )
					execute_command(lh->console_command);
			}
		}
		else
		{
			printLog("Couldn't find a Inf_LevelHeader entity.\n");
		}
	}
	else
	{
		printLog("Couldn't find the Inf_LevelHeader set.\n");
	}
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		LoadLevel
//			Loads up a level specified by the filename
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
geBoolean LoadLevel(char *baseFileName, int fromLoad)
{
	geVFile *Level;
	char Filename[200];
	
	sprintf(Filename, ".//levels//%s", baseFileName ); // levels must be in the level directory

	//-------------------------
	//Loading level into engine
	//-------------------------

	sprintf(str, "Attempting to load: %s.\n", Filename);
	printLog(str);

	//show the loading image

	if( geEngine_BeginFrame(Engine, Camera, GE_FALSE) == GE_FALSE )
	{
		printLog("***geEngine_BeginFrame() failed.\n\n");
		return GE_FALSE;
	}
	
	if( !geEngine_DrawBitmap(Engine, loadingImage, NULL, 0, 0) )
	{
		printLog("Failed to draw the loading image image.\n");
		return GE_FALSE;
	}

	//End frame
	if( geEngine_EndFrame(Engine) == GE_FALSE )
	{
		printLog("***geEngine_EndFrame() failed.\n\n");
		return GE_FALSE;
	}


	//Load that bsp file
	Level = geVFile_OpenNewSystem(NULL, GE_VFILE_TYPE_DOS, Filename, NULL, GE_VFILE_OPEN_READONLY);
	if(!Level)
	{
		if( !fullscreen )
			MessageBox(hWnd, "Could not load file", "Infection Error", 48);
		console_message("Couldn't load that level file");
		sprintf(str, "Could not load file: %s.\n", Filename);
		printLog(str); error(str);
		return GE_FALSE;
	}

	printLog("Level file found.\n");

	LeaveLevel();

	printLog("Creating world.\n");


	//Create the world
	World = geWorld_Create(Level);
	printLog("World created.\n");
	if(!World)
	{
		MessageBox(hWnd, "Could not create the world.", "Infection Error", 48);
		printLog("Couldn't create world.\n");
		geVFile_Close(Level);
		return GE_FALSE;
	}
	printLog("Closing file.\n");
	geVFile_Close(Level);

	printLog("Creating world succeeded.\nAttempting to add world.\n");

	//Add the world to the engine
	if( !geEngine_AddWorld(Engine, World) )
	{
		MessageBox(hWnd, "Could not add the world to the engine", "Infection Error", 48);
		printLog("Couldn't add the world to the engine.\n");
		return GE_FALSE;
	}

	//------------------------
	//Set up the camera/player
	//------------------------
	// Setup the player position and direction

	if( !fromLoad ){
		printLog("PLayerStart\n");
		LoadLevel_PlayerStart( World );
	}
	printLog("LevelHeader\n");
	LoadLevel_LevelHeader( World, fromLoad );

	printLog("Loading Enemy\n");
	enemy_newWorld( World );
	printLog("Initializing entities");
	new_entities(World);


	printLog("Enabeling decals.\n");
	if( options.enable_decals )
	{
		if( !init_decals() )
			return GE_FALSE;
		
	}

	printLog("Enabaling effects.\n");
	if( options.enable_effects )
	{
		if( !init_fx() )
			return GE_FALSE;
	}

	printLog("Changing current speed to zero.\n");
	geVec3d_Set(&CurrentSpeed, 0.0f, 0.0f, 0.0f); 
	ySpeed = 0.0f;

	player_start();

	weapon_apply();

	level_enableLevelFog();
	Crosshair_onLevelLoad();

	strcpy(levelFileName, baseFileName);

	timeout_clear();

	ySpeed = 0.0f;

	if( !fromLoad )
		level_save();

	printLog("LoadLevel returned successful.\n");

	return GE_TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
















///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		ShutDown
//			cleans up all the memory
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ShutDown()
{
	printLog("Shutting down Infection...\n");

	printLog("Leaving level");
	LeaveLevel();

	meny_delete();

	inventory_delete();

	inf_luaExit();

	//fre all bitmaps and destrom 'em
	weapon_destroy();
	DestroyCrosshair();
	destroy_hud();
	soundsys_destroy();
	console_delete();

	if( loadingImage )
	{
		printLog("Removing loadingImage.\n");
		geEngine_RemoveBitmap(Engine, loadingImage);
		geBitmap_Destroy(&loadingImage);
	}

	enemy_erase(); // remove the enemy defs

	if(Camera)
	{
		geCamera_Destroy(&Camera);
		printLog("Camera has been destroyed.\n");
	}

	if(Engine)
	{
		//has hanged on the following line: 2003-01-14
		geEngine_ShutdownDriver(Engine);
		printLog("The driver has been released from the engine.\n");
		geEngine_Free(Engine);
		printLog("The Engine is free.\n");
	}

	ShowCursor(TRUE); // show the curser to the user
	printLog("Cursor is now visible.\n");

	// save all the prefs. If the user has made some changes then it will be remembered.
	if( SavePrefs() == GE_FALSE )
	{
		printLog("Failed to save prefs.\n");
	}

	HT_DESTROY(materialStorage);

	printLog("ShutDown() complete.\n");
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// simple function when quiting the game
void exit_application()
{
	printLog("Sending a quit messsage.\n");
	PostMessage(hWnd, WM_QUIT, 0, 0);
}


// the function that saves the preferences
geBoolean SavePrefs()
{
	FILE *f;
	
	printLog("Preparing to save infection.cfg .\n");
	
	//Open the miniapp.cfg file
	f = fopen("infection.cfg", "w");
	
	if( !f )
	{
		//MessageBox(hWnd, "Failed to read infection.cfg", "No prefs", MB_OK);
		printLog("Couldn't load infection.cfg, generating standard values.\n");
		return GE_FALSE;
	}

	fprintf(f, "# infection.cfg file generated by infection.exe\n");
	fprintf(f, "# based on the options made by the user in the different menus.\n");
	fprintf(f, "\n\n");

	fprintf(f, "# The driver to use \n");				fprintf(f, "<DRIVER> %c \n", ourdriver);
	fprintf(f, "# The width of the aplication \n");		fprintf(f, "<WIDTH> %d \n", Width);
	fprintf(f, "# The height of the application \n");	fprintf(f, "<HEIGHT> %d \n", Height);
	fprintf(f, "# Fullscreen variable \n");				fprintf(f, "<FULLSCREEN> %d \n", fullscreen);
	fprintf(f, "# Enable/Use effects? \n");				fprintf(f, "<ENABLE_EFFECTS> %d \n", options.enable_effects );
	fprintf(f, "# Enable/Use decals\n");					fprintf(f, "<ENABLE_DECALS> %d \n", options.enable_decals );
	fprintf(f, "# Extended log? only for extreme debugging, and crash determing.\n");
	fprintf(f, "<EXTENDED_LOG> %d \n", options.extended_log );
	fprintf(f, "# Should we log? \n");	fprintf(f, "<LOG> %d \n", options.log );
	fprintf(f, "# The current crosshair? \n");	fprintf(f, "<CROSSHAIR> %d \n", options.currentCrosshair );
	fprintf(f, "# The gamma of the application \n");		fprintf(f, "<GAMMA> %f \n", options.gamma );
	fprintf(f, "# Clear screen to black before each render. \n");	fprintf(f, "<CLEAR_SCREEN> %d \n", options.clearScreen == GE_TRUE );
	fprintf(f, "# Fog in water? Disabled by default\n");	fprintf(f, "<FOG_IN_WATER> %d \n", options.fogInWater );
	fprintf(f, "# Invert speakers? Disabled by default\n");	fprintf(f, "<INVERT_SPEAKERS> %d \n", options.invertSpeakers );
	fprintf(f, "# The start level\n");					fprintf(f, "<START_LEVEL> %s \n", options.startLevel);

	fprintf(f, "# Mouse x sensitivity\n");					fprintf(f, "<MOUSE_X_SENSITIVITY> %f \n", real_mouse_x_sensitivity);
	fprintf(f, "# Mouse y sensitivity\n");					fprintf(f, "<MOUSE_Y_SENSITIVITY> %f \n", real_mouse_y_sensitivity);

	fprintf(f, "# Enable/Disable weaponFlash. \n");	fprintf(f, "<WEAPON_FLASH> %d \n", options.weaponFlash == GE_TRUE );
	
	//close the file
	fclose(f);
	
	printLog("Saved prefs done\n");

	return GE_TRUE;
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// end of file
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////