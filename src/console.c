#include <stdio.h>

#include "console.h"

#include "init.h"
#include "edit.h"
#include "globalGenesis.h"
#include "globals.h"
#include "log.h"
#include "weapons.h"
#include "inf_message_system.h"
#include "time_damage.h"
#include "damage.h"
#include "infection.h"
#include "sound_system.h"
#include "conversation.h"
#include "init.h"
#include "crosshair.h"
#include "time_out.h"
#include "inf_lua.h"
#include "ConsoleBuffer.h"
#include "inf_meny.h"

ConsoleBuffer* consoleBuffer;

#define COMMAND_FAILED "Command not accepted"

#define CONSOLE_LINES	20
#define INPUT_LENGTH	200
#define BUFFER_COUNT	10

sEditBox console_inputLine;
char lines[CONSOLE_LINES][INPUT_LENGTH];
char* currentBuffer;

void console_message(char* msg)
{
	int i;
	if( !msg ) return;
	for( i=CONSOLE_LINES-1; i>=1; i--)
	{
		strcpy(lines[i],lines[i-1]); 
	}
	strcpy(lines[0], msg);
}

void console_up(){
	currentBuffer = consoleBuffer_previous(consoleBuffer);
}
void console_down(){
	currentBuffer = consoleBuffer_next(consoleBuffer);
}

void clear_console()
{
	int i;
	for( i=CONSOLE_LINES-1; i>=0; i--)
	{
		lines[i][0] = 0;
	}
}

void render_console()
{
	int i;
	GE_RGBA console_color;
	
	console_color.r = console_color.g = console_color.b = ( !renderMeny() )?0.0f:255.0f;
	console_color.a = 120.0f;
	
	geEngine_FillRect(Engine, &Rect, &console_color);
	
	geEngine_Printf(Engine, 20 , 20 , ">");
	if( currentBuffer ){
		geEngine_Printf(Engine, 30 , 20 , currentBuffer);
	} else {
		EditBox_render(&console_inputLine);
	}
	
	for(i=0; i< CONSOLE_LINES; i++)
	{
		geEngine_Printf(Engine, 30 , 40+i*20 , lines[i]);
	}
}

void console_get(char* str)
{
	if( currentBuffer ){
		strcpy(str, currentBuffer);
		currentBuffer = consoleBuffer_resetCursor(consoleBuffer);
	} else {
		EditBox_clear(&console_inputLine, str);
	}
	consoleBuffer_add(consoleBuffer, str);
}

void console_backspace()
{
	if( currentBuffer ){
		EditBox_set(&console_inputLine, currentBuffer);
		currentBuffer = 0;
	}
	EditBox_pop( &console_inputLine );
}

void console_put(char c)
{
	if( currentBuffer ){
		EditBox_set(&console_inputLine, currentBuffer);
		currentBuffer = 0;
	}
	EditBox_push_char(&console_inputLine, c);
}

void console_delete(){
	consoleBuffer_delete(consoleBuffer );
	consoleBuffer = 0;
}

void console_init()
{
	EditBox_init(&console_inputLine, "", 30, 20);
	//clear the lines
	{
		int i;
		for( i= 0; i<CONSOLE_LINES; i++)
		{
			sprintf(lines[i], "");
		}
	}

	consoleBuffer = consoleBuffer_init(BUFFER_COUNT);
}

// get a line from a file
void fgetln(char* str, FILE* f)
{
	unsigned int cpos = 0;
	int ret;

	while( 1 )
	{
		ret = fgetc(f );
		if( ret == EOF ) break;
		if( ret == '\n' ) break;

		str[cpos] = (char) ret;
		cpos++;

		if( cpos == 199) break;
	}
	str[cpos] = 0;
}

// executes a script

void doExecute_scriptFile(FILE *f)
{
	char command[200];
	memset(command, 0, sizeof(char) * 200);// set it to a known value

	while( !feof(f) )
	{
		fgetln(command, f);
		execute_command(command);
	}
}

void execute_scriptFile(char* file){
	FILE* f = 0;

	char scriptFile[200];
	sprintf(scriptFile, ".\\levels\\%s\\InfScript\\%s", getLevelFileName(), file);
	f= fopen(scriptFile, "r");
	if( f )
	{
		doExecute_scriptFile(f );
		fclose(f);
	}
	else
	{
		sprintf(str, "Failed to open script file: \"%s\"", file );  console_message(str);
		sprintf(str, "Failed to open script file: \"%s\"\n", file ); printLog(str); error(str);
	}
}

// execute a command
void execute_command(char *input)
{
	char c;
	char processingString = 0;
	int stringPos=0;
	int commandPos=0;
	int inputPos=0;

	char command[10][200];

	if( is_string_null(input) ) return;

	for( c= 0; c<10; c++)
		memset(&(command[c]), 0, 200 );
	
	sprintf(str, "Recieved command: %s", input);
	console_message(str);
	sprintf(str, "Recieved command: %s.\n", input);
	printLog(str);
	
	// stor input in the command string array
	while( 1 )
	{
		c = input[inputPos];
		inputPos++;
		
		if( c== 0 )
		{
			command[ commandPos ][ stringPos ] = 0;
			break;
		}
		
		if( !processingString )
		if( stringPos != 0 && c == ' ')
		{
			command[ commandPos ][ stringPos ] = 0;
			commandPos ++;
			stringPos = 0;
			if( commandPos == 10 ) break;
		}

		if(c != ' ' || processingString)
		{
			if(! processingString )
				if( c=='*' )
				{
					processingString = 1;
					continue;
				}
			command[commandPos][stringPos] = c;
			stringPos++;

			if( stringPos == 99 )
			{
				command[commandPos][stringPos] = 0;
				stringPos = 0;
				commandPos++;
				if( commandPos == 10 ) break;
			}
		}
	}

	if( _stricmp(command[0], "level")==0 ){
		if( LoadLevel( command[1], 0 ) )
		{
			console_message("Level loaded");
		}
		else
		{
			console_message("Failed to load level");
		}
	} else if( _stricmp(command[0], "weaponname")==0 ){
		cheats.doPrint = !cheats.doPrint;
		console_message( cheats.doPrint ? "WeaponName is on": "WeaponName is off" );
	} else if( _stricmp(command[0], "bouncy")==0 ){
		cheats.bouncy = !cheats.bouncy;
		console_message( cheats.bouncy? "Bouncy mode is on": "Bouncy mode is off" );
	} else if( _stricmp(command[0], "bbox")==0 ){
		cheats.bbox = !cheats.bbox;
		console_message( cheats.bbox? "BBox mode is on": "BBox mode is off" );
	} else if( _stricmp(command[0], "textureprint")==0 ){
		cheats.texturePrint = !cheats.texturePrint;
		console_message( cheats.texturePrint ? "texturePrint is on": "texturePrint is off" );
	} else if( _stricmp(command[0], "timeout")==0 ){
		float time=0.0f;
		char com = 0;
		sscanf(command[1], "%f", &time);

		if( _stricmp(command[2], "script")==0 ){
			com = 1;
		} else if( _stricmp(command[2], "conv")==0 ){
			com = 2;
		} else if( _stricmp(command[2], "lua")==0 ){
			com = 3;
		} else if( _stricmp(command[2], "luas")==0 ){
			com = 4;
		} else if( _stricmp(command[2], "com")==0 ){
			com = 5;
		}

		if( com ) timeout_add(time, command[3], com-1 );
		else console_message(COMMAND_FAILED);
		{
			char str[200];
			sprintf(str, "Command will be executed after %f seconds", time);
			console_message(str);
		}
	} else if( _stricmp(command[0], "weaponflash")==0 ){
		options.weaponFlash = !options.weaponFlash;
		// EPILEPSY
		if( options.weaponFlash )
			console_message("WARNING: USING THIS MODE MAY CAUSE EPILEPTIC SEIZURES, USE IT ON YOUR OWN RISK");
		console_message( options.weaponFlash ? "WeaponFlash is on": "WeaponFlash is off" );
	} else if( _stricmp(command[0], "god")==0 ){
		cheats.god = !cheats.god;
		console_message( cheats.god ? "God is on": "God is off" );
	} else if( _stricmp(command[0], "ammoking")==0 ){
		cheats.unlimited_ammo = !cheats.unlimited_ammo;
		console_message( cheats.unlimited_ammo ? "AmmoKing is on": "AmmoKing is off" );
	} else if( _stricmp(command[0], "magtotheteeth")==0 ){
		cheats.unlimited_mag = !cheats.unlimited_mag;
		console_message( cheats.unlimited_mag ? "You got magazines to the teeth": "You lost your precios magazines" );
	} else if( _stricmp(command[0], "ghost")==0 ){
		cheats.ghost = !cheats.ghost;
		console_message( cheats.ghost ? "Ghost is on": "Ghost is off" );
	} else if( _stricmp(command[0], "fps")==0 ){
		cheats.printFps = !cheats.printFps;
		console_message( cheats.printFps ? "FPS is on": "FPS is off" );
	} else if( _stricmp(command[0], "debug")==0 ){
		cheats.debug = !cheats.debug;
		console_message( cheats.debug ? "Debug is on": "Debug is off" );
	} else if( _stricmp(command[0], "log")==0 ){
		printLog(command[1] );
		printLog("\n");
	} else if( _stricmp(command[0], "materialprint")==0 ){
		cheats.materialPrint = !cheats.materialPrint;
		console_message( cheats.materialPrint ? "materialPrint is on": "materialPrint is off" );
	} else if( _stricmp(command[0], "give")==0 ){
		if( _stricmp(command[1], "all")==0 ){
			console_message("You got evrything you was supposed to have");
			weapon_give_all();
		} else if( _stricmp(command[1], "drug")==0 ){
			timedamage_add(TIMEDAMAGE_DRUG);
		} else if( _stricmp(command[1], "fire")==0 ){
			timedamage_add(TIMEDAMAGE_FIRE);
		} else if( _stricmp(command[1], "zbite")==0 ){
			timedamage_add(TIMEDAMAGE_ZOMBIEBITE);
		} else if( _stricmp(command[1], "ibite")==0 ){
			timedamage_add(TIMEDAMAGE_INFECTEDBITE);
		} else if( _stricmp(command[1], "lava")==0 ){
			timedamage_add(TIMEDAMAGE_LAVADAMAGE);
		} else if( _stricmp(command[1], "damage")==0 ){
			int i;
			i = atoi(command[2] );
			if( i > 255 ) i= 255;
			if( i < 0 ) i = 0;
			damage(i);
		} else if( _stricmp(command[1], "xdamage")==0 ){
			int i;
			i = atoi(command[2] );
			if( i > 255 ) i= 255;
			if( i < 0 ) i = 0;
			armor_piercing_damage(i);
		} else if( _stricmp(command[1], "knife")==0 ){
			weapon_give_knife();
		} else if( _stricmp(command[1], "hammer")==0 ){
			weapon_give_hammer();
		} else if( _stricmp(command[1], "axe")==0 ){
			weapon_give_axe();
		} else if( _stricmp(command[1], "glock")==0 ){
			weapon_give_glock();
		} else if( _stricmp(command[1], "deagle")==0 ){
			weapon_give_deagle();
		} else if( _stricmp(command[1], "tazer")==0 ){
			weapon_give_tazer();
		} else if( _stricmp(command[1], "tranqualizer")==0 ){
			weapon_give_tranqualizer();
		} else if( _stricmp(command[1], "shotgun")==0 ){
			weapon_give_shotgun();
		} else if( _stricmp(command[1], "mag7")==0 ){
			weapon_give_mag7();
		} else if( _stricmp(command[1], "sniper")==0 ){
			weapon_give_sniper();
		} else if( _stricmp(command[1], "barret")==0 ){
			weapon_give_barret();
		} else if( _stricmp(command[1], "smg")==0 ){
			weapon_give_smg();
		} else if( _stricmp(command[1], "uzi")==0 ){
			weapon_give_uzi();
		} else if( _stricmp(command[1], "ak47")==0 ){
			weapon_give_ak47();
		} else if( _stricmp(command[1], "commando")==0 ){
			weapon_give_commando();
		} else if( _stricmp(command[1], "lft")==0 ){
			weapon_give_lft();
		} else if( _stricmp(command[1], "flamethrower")==0 ){
			weapon_give_flamethrower();
		} else if( _stricmp(command[1], "molotov")==0 ){
			weapon_give_molotov();
		} else if( _stricmp(command[1], "signalpistol")==0 ){
			weapon_give_signalpistol();
		} else if( _stricmp(command[1], "launcher")==0 ){
			weapon_give_launcher();
		} else if( _stricmp(command[1], "grenade")==0 ){
			weapon_give_grenade();
		} else if( _stricmp(command[1], "c4")==0 ){
			weapon_give_c4();
		} else if( _stricmp(command[1], "mine")==0 ){
			weapon_give_mine();
		} else {
			console_message(COMMAND_FAILED);
		}
	} else if( _stricmp(command[0], "set")==0 ){
		if( _stricmp(command[1], "health")==0 ){
			int i;
			i = atoi(command[2] );
			if( i > 100 ) i= 100;
			if( i < 1 ) i = 1;
			hero_hit_points = i;
		} else if( _stricmp(command[1], "armor")==0 ){
			int i;
			i = atoi(command[2] );
			if( i > 100 ) i= 100;
			if( i < 0 ) i = 0;
			hero_armor_points = i;
		} else if( _stricmp(command[1], "gravity")==0 ){
			float f;
			sscanf(command[2], "%f", &f );
			gravity = f;
		} else if( _stricmp(command[1], "speed")==0 ){
			float f;
			sscanf(command[2], "%f", &f );
			real_speed = f;
		} else if( _stricmp(command[1], "jumpspeed")==0 ){
			float f;
			sscanf(command[2], "%f", &f );
			PLAYER_JUMP_SPEED = f;
		} else {
			console_message(COMMAND_FAILED);
		}
	} else if( _stricmp(command[0], "clear")==0 ){
		clear_console();
	} else if( _stricmp(command[0], "exit")==0 ){
		exit_application();
	} else if( _stricmp(command[0], "new")==0 ){
		new_game();
	} else if( _stricmp(command[0], "save")==0 ){
		int i;
		i = atoi( command[1] );
		if( i > 255 ) i= 255;
		if( i < 0 ) i = 0;
		save_game(i);
	} else if( _stricmp(command[0], "load")==0 ){
		int i;
		i = atoi( command[1] );
		if( i > 255 ) i= 255;
		if( i < 0 ) i = 0;
		load_game(i);
	} else if( _stricmp(command[0], "cover")==0 ){
		int i;
		i = atoi( command[1] );
		if( i < 0 ) i = 0;
		if( i > NUM_CROSSHAIRS-1 ) i = NUM_CROSSHAIRS-1;
		options.crosshairOver = i;
	} else if( _stricmp(command[0], "cnormal")==0 ){
		int i;
		i = atoi( command[1] );
		if( i < 0 ) i = 0;
		if( i > NUM_CROSSHAIRS-1 ) i = NUM_CROSSHAIRS-1;
		options.crosshairNormal = i;
	} else if( _stricmp(command[0], "foggywater")==0 ){
		options.fogInWater = !options.fogInWater;
		console_message( options.fogInWater ? "Foggywater is on": "Foggywater is off" );
	} else if( _stricmp(command[0], "clearscreen")==0 ){
		options.clearScreen = !options.clearScreen;
		console_message( options.clearScreen? "ClearScreen is on": "ClearScreen is off" );
/*	} else if( _stricmp(command[0], "fov")==0 ){
		float f;
		if( _stricmp(command[1], "normal") == 0 )
		{
			f = fov;
		}
		else
		{
			sscanf(command[1], "%f", &f );
		}
		geCamera_SetAttributes(Camera, f, &Rect);
	} else if( _stricmp(command[0], "timefx")==0 ){
		if( _stricmp(command[1], "slowmotion") == 0 ){
			float f; sscanf(command[2], "%f", &f );
			timefx_slowmotion(f);
		} else if( _stricmp(command[1], "drug") == 0 ){
			float f; sscanf(command[2], "%f", &f );
			timefx_drug(f);
		} else if( _stricmp(command[1], "bite") == 0 ){
			float f; sscanf(command[2], "%f", &f );
			timefx_bite(f);
		} else if( _stricmp(command[1], "normal") == 0 ){
			timefx_normal();
		} else {
			console_message(COMMAND_FAILED);
		}*/
	} else if( _stricmp(command[0], "mousex")==0 ){
		sscanf(command[1], "%f", &real_mouse_x_sensitivity);
		mouse_x_sensitivity = real_mouse_x_sensitivity;
	} else if( _stricmp(command[0], "mousey")==0 ){
		sscanf(command[1], "%f", &real_mouse_y_sensitivity);
		mouse_y_sensitivity = real_mouse_y_sensitivity;
	} else if( _stricmp(command[0], "reportsense")==0 ){
		char str[ 200];
		sprintf(str, "mouse y: %f", real_mouse_y_sensitivity); console_message(str);
		sprintf(str, "mouse x: %f", real_mouse_x_sensitivity); console_message(str);
	} else if( _stricmp(command[0], "watermeter")==0 ){
		sscanf(command[1], "%f", &(options.meterfog));
	} else if( _stricmp(command[0], "consolemessage")==0 ){
		console_message( command[1] );
	} else if( _stricmp(command[0], "gamemessage")==0 ){
		game_message( command[1] );
	} else if( _stricmp(command[0], "systemmessage")==0 ){
		system_message( command[1] );
	} else if( _stricmp(command[0], "gplayso")==0 ){
		char fileName[200];
		sprintf(fileName, ".\\sfx\\%s", command[1]);
		if(! soundsys_play_once_sound( fileName ) ){
			char errorString[300];
			sprintf(errorString, "Failed to play sound %s", fileName);
			console_message(errorString);
		}
	} else if( _stricmp(command[0], "playso")==0 ){
		char fileName[200];
		sprintf(fileName, ".\\levels\\%s\\Sounds\\%s", getLevelFileName(), command[1]);
		if(! soundsys_play_once_sound( fileName ) ){
			char errorString[300];
			sprintf(errorString, "Failed to play sound %s", fileName);
			console_message(errorString);
		}
	} else if( _stricmp(command[0], "conv")==0 ){
		// ( _stricmp(command[2],"true")==0 )?GE_TRUE:GE_FALSE
		startConversation(getLevelFileName(), command[1], GE_FALSE);
	} else if( _stricmp(command[0], "enable")==0 ){
		enableByName(World, command[1]);
	} else if( _stricmp(command[0], "disable")==0 ){
		disableByName(World, command[1]);
	} else if( command[0][0] == '#' ){
		// dont do anything
	} else if( _stricmp(command[0], "loose")==0 ){
		if( _stricmp(command[1], "all")==0 ){
			weapon_strip();
		} else if( _stricmp(command[1], "knife")==0 ){
			weapon_loose_knife();
		} else if( _stricmp(command[1], "hammer")==0 ){
			weapon_loose_hammer();
		} else if( _stricmp(command[1], "axe")==0 ){
			weapon_loose_axe();
		} else if( _stricmp(command[1], "glock")==0 ){
			weapon_loose_glock();
		} else if( _stricmp(command[1], "deagle")==0 ){
			weapon_loose_deagle();
		} else if( _stricmp(command[1], "tazer")==0 ){
			weapon_loose_tazer();
		} else if( _stricmp(command[1], "tranqualizer")==0 ){
			weapon_loose_tranqualizer();
		} else if( _stricmp(command[1], "shotgun")==0 ){
			weapon_loose_shotgun();
		} else if( _stricmp(command[1], "mag7")==0 ){
			weapon_loose_mag7();
		} else if( _stricmp(command[1], "sniper")==0 ){
			weapon_loose_sniper();
		} else if( _stricmp(command[1], "barret")==0 ){
			weapon_loose_barret();
		} else if( _stricmp(command[1], "smg")==0 ){
			weapon_loose_smg();
		} else if( _stricmp(command[1], "uzi")==0 ){
			weapon_loose_uzi();
		} else if( _stricmp(command[1], "ak47")==0 ){
			weapon_loose_ak47();
		} else if( _stricmp(command[1], "commando")==0 ){
			weapon_loose_commando();
		} else if( _stricmp(command[1], "lft")==0 ){
			weapon_loose_lft();
		} else if( _stricmp(command[1], "flamethrower")==0 ){
			weapon_loose_flamethrower();
		} else if( _stricmp(command[1], "molotov")==0 ){
			weapon_loose_molotov();
		} else if( _stricmp(command[1], "signalpistol")==0 ){
			weapon_loose_signalpistol();
		} else if( _stricmp(command[1], "launcher")==0 ){
			weapon_loose_launcher();
		} else if( _stricmp(command[1], "grenade")==0 ){
			weapon_loose_grenade();
		} else if( _stricmp(command[1], "c4")==0 ){
			weapon_loose_c4();
		} else if( _stricmp(command[1], "mine")==0 ){
			weapon_loose_mine();
		} else {
			console_message(COMMAND_FAILED);
		}
	} else if( _stricmp(command[0], "script")==0 ){
		//FILE *f=0;
		if( command[1][0] != 0 )
		{
			execute_scriptFile( command[1] );
		}
		else
		{
			console_message("Second parameter should be a script file to execute");
		}
	} else if( _stricmp(command[0], "lua")==0 ){
		if( command[1][0] != 0 )
		{
			if(! inf_luafile( command[1] ) ){
				char str[200];
				sprintf(str, "Failed to execute lua file: %s", command[1]);
				console_message(str);
			}
		}
		else
		{
			console_message("Second parameter should be a lua-script file to execute");
		}
	} else if( _stricmp(command[0], "luas")==0 ){
		if( command[1][0] != 0 )
		{
			if(! inf_luacommand( command[1] ) ){
				char str[200];
				sprintf(str, "Failed to execute lua command: %s", command[1]);
				console_message(str);
			}
		}
		else
		{
			console_message("Second parameter should be a lua-script file to execute");
		}
	} else {
		console_message(COMMAND_FAILED);
	}
}