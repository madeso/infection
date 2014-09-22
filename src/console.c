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
#include "movie.h"

ConsoleBuffer* consoleBuffer;
GE_RGBA console_fontColor;

#define COMMAND_FAILED "Command not accepted"

#define CONSOLE_LINES	20
#define INPUT_LENGTH	200
#define BUFFER_COUNT	10

sEditBox console_inputLine;
char lines[CONSOLE_LINES][INPUT_LENGTH];
char* currentBuffer;

void console_message(const char* msg)
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
	
	console_fontColor.r = console_fontColor.g = console_fontColor.b = ( !renderMeny() )? 255.0f : 0.0f;
	console_fontColor.a = 255.0f;
	
	console_color.r = console_color.g = console_color.b = ( !renderMeny() )?0.0f:255.0f;
	console_color.a = 120.0f;
	
	geEngine_FillRect(Engine, &Rect, &console_color);
	
	//geEngine_Printf(Engine, 20 , 20 , ">");
	if( currentBuffer ){
		//geEngine_Printf(Engine, 30 , 20 , currentBuffer);
		XFontMgr_PrintAt(fntMgr, 20, 20, kFontSmall, console_fontColor, Camera, "> %s", currentBuffer);
	} else {
		EditBox_render(&console_inputLine, console_fontColor);
	}
	
	for(i=0; i< CONSOLE_LINES; i++)
	{
		//geEngine_Printf(Engine, 30 , 40+i*20 , lines[i]);
		XFontMgr_PrintAt(fntMgr, 30, 40+i*20, kFontSmall, console_fontColor, Camera, "%i: %s", i+1, lines[i]);
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
	EditBox_init(&console_inputLine, "", 20, 20);
	console_inputLine.font = kFontSmall;
	console_inputLine.printExtraString = 1;
	
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

void execute_scriptFile(const char* file){
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
void execute_command(const char *input)
{
	if( is_string_null(input) ) return;
	
	sprintf(str, "Recieved command: %s", input);
	console_message(str);
	sprintf(str, "Recieved command: %s.\n", input);
	printLog(str);

	if( strcmp(input, "exit")==0 ) {
		exit_application();
	}
	else if( strcmp(input, "new")==0 ) {
		new_game();
	}
	else {
		inf_luacommand( input );
	}
}

char levelToLoad[200];
char loadLevel = 0;
void checkToLoadLevel() {
	if( loadLevel ) {
		if( LoadLevel(levelToLoad, 0) ) {
			console_message("Level loaded");
		}
		else {
			console_message("Failed to load level");
		}
		loadLevel = 0;
	}
}

LUA_FUNC(loadLevel) {
	int args;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	strcpy(levelToLoad, lua_tostring(luaVM, -1));
	loadLevel = 1;

	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(weaponname){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	cheats.doPrint = !cheats.doPrint;
	console_message( cheats.doPrint ? "WeaponName is on": "WeaponName is off" );
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(bouncy){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	cheats.bouncy = !cheats.bouncy;
	console_message( cheats.bouncy? "Bouncy mode is on": "Bouncy mode is off" );
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(bbox){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	cheats.bbox = !cheats.bbox;
	console_message( cheats.bbox? "BBox mode is on": "BBox mode is off" );
	
	lua_pushnumber(luaVM, 1);
	return 1;
} 

LUA_FUNC(textureprint){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	cheats.texturePrint = !cheats.texturePrint;
	console_message( cheats.texturePrint ? "texturePrint is on": "texturePrint is off" );
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(movie){
	int args;
	if( (args = lua_gettop(luaVM)) != 3 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	if( movie_execute(lua_tostring(luaVM, -3), lua_tostring(luaVM, -2), (float) lua_tonumber(luaVM, -1)) ) {
		console_message("movie ok");
		lua_pushnumber(luaVM, 1);
	}
	else {
		console_message("movie failed, check log");
		lua_pushnumber(luaVM, 0);
	}
	
	return 1;
}

LUA_FUNC(timeout){
	int args;
	if( (args = lua_gettop(luaVM)) != 2 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	timeout_add((float) lua_tonumber(luaVM, -1), lua_tostring(luaVM, -2));
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(weaponflash){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	options.weaponFlash = !options.weaponFlash;
	// EPILEPSY
	if( options.weaponFlash )
		console_message("WARNING: USING THIS MODE MAY CAUSE EPILEPTIC SEIZURES, USE IT ON YOUR OWN RISK");
	console_message( options.weaponFlash ? "WeaponFlash is on": "WeaponFlash is off" );
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(god){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	cheats.god = !cheats.god;
	console_message( cheats.god ? "God is on": "God is off" );
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(ammoking){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	cheats.unlimited_ammo = !cheats.unlimited_ammo;
	console_message( cheats.unlimited_ammo ? "AmmoKing is on": "AmmoKing is off" );
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(magtotheteeth){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	cheats.unlimited_mag = !cheats.unlimited_mag;
	console_message( cheats.unlimited_mag ? "You got magazines to the teeth": "You lost your precios magazines" );
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(ghost){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	cheats.ghost = !cheats.ghost;
	console_message( cheats.ghost ? "Ghost is on": "Ghost is off" );
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(fps){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	cheats.printFps = !cheats.printFps;
	console_message( cheats.printFps ? "FPS is on": "FPS is off" );
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(debug){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	cheats.debug = !cheats.debug;
	console_message( cheats.debug ? "Debug is on": "Debug is off" );
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(log){
	int args;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	printLog(lua_tostring(luaVM, -1) );
	printLog("\n");
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(materialprint){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	cheats.materialPrint = !cheats.materialPrint;
	console_message( cheats.materialPrint ? "materialPrint is on": "materialPrint is off" );
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(giveAll){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	console_message("You got evrything you was supposed to have");
	weapon_give_all();

	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(giveDrug){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	timedamage_add(TIMEDAMAGE_DRUG, 5);
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(giveFire){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	timedamage_add(TIMEDAMAGE_FIRE, 5);
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(giveBite){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	timedamage_add(TIMEDAMAGE_INFECTED, 5);
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(giveDamage){
	int args;
	int i;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}
	
	i = (int) lua_tonumber(luaVM, -1);
	if( i > 255 ) i= 255;
	if( i < 0 ) i = 0;
	damage(i);
	
	lua_pushnumber(luaVM, i);
	return 1;
}

LUA_FUNC(giveXdamage){
	int args;
	int i;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	i = (int)lua_tonumber(luaVM, -1);
	if( i > 255 ) i= 255;
	if( i < 0 ) i = 0;
	armor_piercing_damage(i);
	
	lua_pushnumber(luaVM, i);
	return 1;
}

LUA_FUNC(giveKnife){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	weapon_give_knife();
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(giveGlock){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	weapon_give_glock(2);
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(giveDeagle ){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	weapon_give_deagle(2);
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(giveShotgun){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	weapon_give_shotgun(2);
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(giveSniper){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	weapon_give_sniper(2);
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(giveSmg){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	weapon_give_smg(2);
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(giveUzi){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	weapon_give_uzi(2);
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(giveLft){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	weapon_give_lft(2);
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(giveMolotov){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	weapon_give_molotov(2);
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(giveGrenade){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	weapon_give_grenade(2);
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(giveMinirocket){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	weapon_give_minirocket(2);
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(setHealth){
	int args;
	int i;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	i = (int) lua_tonumber(luaVM, -1);
	if( i > 100 ) i= 100;
	if( i < 1 ) i = 1;
	hero_hit_points = i;
	
	lua_pushnumber(luaVM, i);
	return 1; 
}

LUA_FUNC(setArmor){
	int args;
	int i;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	i = (int) lua_tonumber(luaVM, -1);
	if( i > 100 ) i= 100;
	if( i < 0 ) i = 0;
	hero_armor_points = i;
	
	lua_pushnumber(luaVM, i);
	return 1;
}

LUA_FUNC(setGravity){
	int args;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	gravity = (float) lua_tonumber(luaVM, -1);
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(setSpeed){
	int args;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	real_speed = (float) lua_tonumber(luaVM, -1);
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(setJumpspeed){
	int args;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	PLAYER_JUMP_SPEED = (float) lua_tonumber(luaVM, -1);
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(clear){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	clear_console();
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(cover){
	int args;
	int i;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	i = (int) lua_tonumber(luaVM, -1);
	if( i < 0 ) i = 0;
	if( i > NUM_CROSSHAIRS-1 ) i = NUM_CROSSHAIRS-1;
	options.crosshairOver = i;
	
	lua_pushnumber(luaVM, 1);
	return 1;
} 

LUA_FUNC(cnormal){
	int args;
	int i;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	i = (int) lua_tonumber(luaVM, -1);
	if( i < 0 ) i = 0;
	if( i > NUM_CROSSHAIRS-1 ) i = NUM_CROSSHAIRS-1;
	options.crosshairNormal = i;
	
	lua_pushnumber(luaVM, 1);
	return 1;
} 

LUA_FUNC(foggywater){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	options.fogInWater = !options.fogInWater;
	console_message( options.fogInWater ? "Foggywater is on": "Foggywater is off" );
	
	lua_pushnumber(luaVM, 1);
	return 1;
} 

LUA_FUNC(clearscreen){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	options.clearScreen = !options.clearScreen;
	console_message( options.clearScreen? "ClearScreen is on": "ClearScreen is off" );
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(mousex) {
	int args;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	mouse_x_sensitivity = (float) lua_tonumber(luaVM, -1);
	
	lua_pushnumber(luaVM, 1);
	return 1;
} 

LUA_FUNC(mousey){
	int args;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	mouse_y_sensitivity = (float) lua_tonumber(luaVM, -1);
	
	lua_pushnumber(luaVM, 1);
	return 1;
} 

LUA_FUNC(reportsense){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}
	
	{
	char str[ 200];
	sprintf(str, "mouse y: %f", real_mouse_y_sensitivity); console_message(str);
	sprintf(str, "mouse x: %f", real_mouse_x_sensitivity); console_message(str);
	}
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(watermeter){
	int args;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	options.meterfog = (float) lua_tonumber(luaVM, -1);
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(consolemessage){
	int args;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}
	console_message( lua_tostring(luaVM, -1) );
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(gamemessage){
	int args;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}
	game_message( lua_tostring(luaVM, -1) );
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(systemmessage){
	int args;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}
	system_message( lua_tostring(luaVM, -1) );
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(gplayso){
	int args;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	{
		char fileName[200];
		sprintf(fileName, ".\\sfx\\%s", lua_tostring(luaVM, -1));
		if(! soundsys_play_once_sound( fileName ) ){
			char errorString[300];
			sprintf(errorString, "Failed to play sound %s", fileName);
			console_message(errorString);
			lua_pushnumber(luaVM, 0);
		}
		else {
			lua_pushnumber(luaVM, 1);
		}
	}
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(playso2d){
	int args;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	{
		char fileName[200];
		sprintf(fileName, ".\\levels\\%s\\Sounds\\%s", getLevelFileName(), lua_tostring(luaVM, -1) );
		if(! soundsys_play_once_sound( fileName ) ){
			char errorString[300];
			sprintf(errorString, "Failed to play sound %s", fileName);
			console_message(errorString);
			lua_pushnumber(luaVM, 0);
		}
		else {
			lua_pushnumber(luaVM, 1);
		}
	}
	return 1;
}

LUA_FUNC(playso3d){
	int args;
	if( (args = lua_gettop(luaVM)) != 3 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	{
		char fileName[200];
		geVec3d* location;
		float min = (float) lua_tonumber(luaVM, -2);
		sprintf(fileName, ".\\levels\\%s\\Sounds\\%s", getLevelFileName(), lua_tostring(luaVM, -3));
		location = findLocationByName(World, lua_tostring(luaVM, -1));
		
		if(! soundsys_play_once_3dsound(fileName, location, min, GE_FALSE) ){
			char errorString[300];
			sprintf(errorString, "Failed to play sound %s", fileName);
			console_message(errorString);
			lua_pushnumber(luaVM, 0);
		}
		else {
			lua_pushnumber(luaVM, 1);
		}
	}
	return 1;
} 

LUA_FUNC(splay){
	int args;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	{
	char fileName[200];
	sprintf(fileName, ".\\levels\\%s\\Songs\\%s", getLevelFileName(), lua_tostring(luaVM, -1));
	if( soundsys_playMusic(fileName) )
		lua_pushnumber(luaVM, 1);
	else 
		lua_pushnumber(luaVM, 0);
	}
	return 1;
}

LUA_FUNC(schange){
	int args;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}
	{
	char fileName[200];
	sprintf(fileName, ".\\levels\\%s\\Songs\\%s", getLevelFileName(), lua_tostring(luaVM, -1));
	if( soundsys_changeMusic(fileName) )
		lua_pushnumber(luaVM, 1);
	else
		lua_pushnumber(luaVM, 0);
	}
	return 1;
}

LUA_FUNC(sstop){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}
	soundsys_stopMusic();
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(conv){
	int args;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	startConversation(getLevelFileName(), lua_tostring(luaVM, -1), GE_FALSE);
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(enable){
	int args;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}
	enableByName(World, lua_tostring(luaVM, -1));
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(disable){
	int args;
	if( (args = lua_gettop(luaVM)) != 1 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}
	
	// call function
	disableByName(World, lua_tostring(luaVM, -1));
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(looseKnife){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	weapon_loose_knife();
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(looseGlock){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	weapon_loose_glock();
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(looseDeagle){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	weapon_loose_deagle();

	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(looseShotgun){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	weapon_loose_shotgun();

	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(looseSniper){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	weapon_loose_sniper();

	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(looseSmg){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	weapon_loose_smg();
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(looseUzi){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	weapon_loose_uzi();

	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(looseLft){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}


	weapon_loose_lft();

	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(looseMolotov){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	weapon_loose_molotov();
	
	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(looseGrenade){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	weapon_loose_grenade();

	lua_pushnumber(luaVM, 1);
	return 1;
}

LUA_FUNC(looseMinirocket){
	int args;
	if( (args = lua_gettop(luaVM)) != 0 ){
		for(;args>0; args--)
			lua_tostring(luaVM, -1);
		// todo
		lua_error(luaVM);
		return 0;
	}

	weapon_loose_minirocket();

	lua_pushnumber(luaVM, 1);
	return 1;
}

void console_initLua() {
	// good thing there are such cool stuff as macros
	LOAD_FUNC(loadLevel);
	LOAD_FUNC(weaponname);
	LOAD_FUNC(bouncy);
	LOAD_FUNC(bbox);
	LOAD_FUNC(textureprint);
	LOAD_FUNC(movie);
	LOAD_FUNC(timeout);
	LOAD_FUNC(weaponflash);
	LOAD_FUNC(god);
	LOAD_FUNC(ammoking);
	LOAD_FUNC(magtotheteeth);
	LOAD_FUNC(ghost);
	LOAD_FUNC(fps);
	LOAD_FUNC(debug);
	LOAD_FUNC(log);
	LOAD_FUNC(materialprint);
	LOAD_FUNC(giveAll);
	LOAD_FUNC(giveDrug);
	LOAD_FUNC(giveFire);
	LOAD_FUNC(giveBite);
	LOAD_FUNC(giveDamage);
	LOAD_FUNC(giveXdamage);
	LOAD_FUNC(giveKnife);
	LOAD_FUNC(giveGlock);
	LOAD_FUNC(giveDeagle);
	LOAD_FUNC(giveShotgun);
	LOAD_FUNC(giveSniper);
	LOAD_FUNC(giveSmg);
	LOAD_FUNC(giveUzi);
	LOAD_FUNC(giveLft);
	LOAD_FUNC(giveMolotov);
	LOAD_FUNC(giveGrenade);
	LOAD_FUNC(giveMinirocket);
	LOAD_FUNC(setHealth);
	LOAD_FUNC(setArmor);
	LOAD_FUNC(setGravity);
	LOAD_FUNC(setSpeed);
	LOAD_FUNC(setJumpspeed);
	LOAD_FUNC(clear);
	LOAD_FUNC(cover);
	LOAD_FUNC(cnormal);
	LOAD_FUNC(foggywater);
	LOAD_FUNC(clearscreen);
	LOAD_FUNC(mousex) ;
	LOAD_FUNC(mousey);
	LOAD_FUNC(reportsense);
	LOAD_FUNC(watermeter);
	LOAD_FUNC(consolemessage);
	LOAD_FUNC(gamemessage);
	LOAD_FUNC(systemmessage);
	LOAD_FUNC(gplayso);
	LOAD_FUNC(playso2d);
	LOAD_FUNC(playso3d);
	LOAD_FUNC(splay);
	LOAD_FUNC(schange);
	LOAD_FUNC(sstop);
	LOAD_FUNC(conv);
	LOAD_FUNC(enable);
	LOAD_FUNC(disable);
	LOAD_FUNC(looseKnife);
	LOAD_FUNC(looseGlock);
	LOAD_FUNC(looseDeagle);
	LOAD_FUNC(looseShotgun);
	LOAD_FUNC(looseSniper);
	LOAD_FUNC(looseSmg);
	LOAD_FUNC(looseUzi);
	LOAD_FUNC(looseLft);
	LOAD_FUNC(looseMolotov);
	LOAD_FUNC(looseGrenade);
	LOAD_FUNC(looseMinirocket);
}