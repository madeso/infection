#include "xfont.h"
#include "font.h" // G3D SDK

#include <malloc.h>
#include "g3dfonthack.c"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define SAFE_DELETE(x) { if(x){free(x); x=0;} }
#define SAFE_ALLOC(type) ((type*) malloc(sizeof(type)))


/*
typedef struct _Character {
	geBitmap *bitmap;
	int width, height;
	int offsetY;
} XCharacter;

typedef struct _Font {
	int m_size;
	char m_name[kNameLength];
	geBoolean m_anti;
	XCharacter m_chars[256];
	geWorld *m_world;
	int m_spaceWidth;
} XFont;

typedef struct _FontMgr { 
	XFont *m_fonts[kFontMax];
} XFontMgr;*/

XFontMgr* XFontMgr_FontMgr(geEngine* theEngine){
	XFontMgr* mgr = 0;
	int i;

	mgr = SAFE_ALLOC(XFontMgr);
	for ( i = 0; i < kFontMax; i++ )
		mgr->m_fonts[i] = 0;

	XFontMgr_Create(mgr, theEngine);

	return mgr;
}
XFontMgr_Destructor(XFontMgr* mgr){
	XFontMgr_Destroy(mgr);
	free(mgr);
}
void XFontMgr_Create(XFontMgr* mgr, geEngine* theEngine){
	XFontMgr_LoadFont(mgr, "Arial Black", 16, GE_TRUE, theEngine );
	XFontMgr_LoadFont(mgr, "Verdana", 18, GE_TRUE, theEngine );
	XFontMgr_LoadFont(mgr, "Verdana", 20, GE_TRUE, theEngine );
}
void XFontMgr_Destroy(XFontMgr* mgr){
	int i;
	if( !mgr ) return;
	for ( i = 0; i < kFontMax; i++ ){
		SAFE_DELETE( mgr->m_fonts[i] );
	}
}
int XFontMgr_LoadFont(XFontMgr* mgr, char *name, int size, geBoolean anti, geEngine* theEngine){
	int index;
	geBoolean found = GE_FALSE;
	int i;

	for ( i = 0; i < kFontMax; i++ ){
		// do we already have this font?
		if ( mgr->m_fonts[i] ){
			if ( strcmp( XFont_GetName(mgr->m_fonts[i]), name ) == 0 )
			if ( XFont_GetSize(mgr->m_fonts[i]) == size )
			if ( XFont_IsAntiAliased(mgr->m_fonts[i]) == anti )
				return i; // we've already loaded this one...
		}
	}

	for ( index = 0; index < kFontMax; index++ ){
		if ( !mgr->m_fonts[index] ){
			found = GE_TRUE;
			break;
		}
	}

	if (!found) return -1;

	mgr->m_fonts[index] = XFont_Font();
	if( XFont_Create(mgr->m_fonts[index], name, size, anti, theEngine )
		== GE_FALSE ) return -1;

	return index;
}

// begin here the next time
void XFontMgr_UnloadFont(XFontMgr* mgr, int index){
	if ( mgr->m_fonts[index] )
	{
		SAFE_DELETE( mgr->m_fonts[index] );
	}
}
XFont* XFontMgr_GetFont(XFontMgr* mgr, int index){
	if ( index >= 0 && index <= kFontMax ) return mgr->m_fonts[index]; else return NULL;
}
void XFontMgr_PrintAt(XFontMgr* mgr, int x, int y, int font, GE_RGBA color, geCamera* camera, char *format, ...){
	va_list			args;
    char			str[1024];

	assert( font < kFontMax );
	if ( !mgr->m_fonts[font] )
		return;

	va_start(args, format);
    vsprintf(str, format, args);
	va_end(args);

	XFont_PrintAt(mgr->m_fonts[font], x, y, color, camera, str );
}














XFont* XFont_Font(){
	XFont* font;
	int i;

	font = SAFE_ALLOC(XFont);

	font->m_world = NULL;
	font->m_size = 0;
	font->m_anti = GE_FALSE;
	strcpy( font->m_name, "" );
	for ( i = kFontLowRange; i <= kFontHighRange; i++ )
		font->m_chars[i].bitmap = NULL;
	font->m_spaceWidth = 0;
	return font;
}
void XFont_Destructor(XFont* fnt){
	XFont_Destroy(fnt);
	free(fnt);
}
geBoolean XFont_Create(XFont* fnt, char *name, int size, geBoolean anti, geEngine* theEngine){
	geFont *font;
	geRect cellRect;
	geBitmap *fontbmp;
	geFontCharacterRecord charRec;
	int i;

	assert( name );
	assert( size > 0 );

	strcpy( fnt->m_name, name );
	fnt->m_size = size;
	fnt->m_anti = anti;

	fnt->m_engine = theEngine;

	font = geFont_Create(theEngine, name, size, 0, anti );
	if ( font == NULL ) return GE_FALSE;

	geFont_AddCharacters( font, kFontLowRange, kFontHighRange ); // bang, font bitmap is built

	// create world to store the font bitmaps
	fnt->m_world = geWorld_Create( NULL ); // create an empty world
	if ( !fnt->m_world ) return GE_FALSE;

	// add the world to the engine
	if ( GE_FALSE == geEngine_AddWorld(theEngine, fnt->m_world ) )
		return GE_FALSE;

	// now build a set of individual character bitmaps and add each to the world
	
	fontbmp = font->bitmapList->map; // the big font bitmap containing all glyphs
	assert( fontbmp );

	for ( i = kFontLowRange; i <= kFontHighRange; i++ )
	{
		charRec = font->characterArray[i];
		cellRect = charRec.bitmapRect;

		fnt->m_chars[i].width = cellRect.Right - cellRect.Left;
		fnt->m_chars[i].height = cellRect.Bottom - cellRect.Top;
		fnt->m_chars[i].offsetY = fnt->m_size - fnt->m_chars[i].height;
		//assert( fnt->m_chars[i].offsetY >= 0 );

		fnt->m_chars[i].bitmap = geBitmap_Create( fnt->m_chars[i].width, fnt->m_chars[i].height,
			0/*<-- mip count*/, GE_PIXELFORMAT_8BIT );
		assert( fnt->m_chars[i].bitmap );

		if ( GE_FALSE == geBitmap_Blit( fontbmp,
			//charRec.offsetX, charRec.offsetY,
			cellRect.Left, cellRect.Top,
			fnt->m_chars[i].bitmap, 0, 0, fnt->m_chars[i].width, fnt->m_chars[i].height ) )
		{
			return GE_FALSE;
		}

		if ( GE_FALSE == geWorld_AddBitmap( fnt->m_world, fnt->m_chars[i].bitmap ) )
			return GE_FALSE;
	}

	geFont_Destroy( &font ); // we're done with the font now

	fnt->m_spaceWidth = fnt->m_size / 3;

	return GE_TRUE;
}
void XFont_Destroy(XFont* fnt){
	int i;
	if ( fnt->m_world )
	{
		// remove the bitmaps from the world
		for ( i = kFontLowRange; i <= kFontHighRange; i++ )
		{
			if ( fnt->m_chars[i].bitmap )
			{
				geWorld_RemoveBitmap( fnt->m_world, fnt->m_chars[i].bitmap );
				geBitmap_Destroy( &(fnt->m_chars[i].bitmap) );
			}
		}

		geEngine_RemoveWorld( fnt->m_engine, fnt->m_world );
		geWorld_Free( fnt->m_world );
		fnt->m_world = NULL;
	}
}
void XFont_PrintAt(XFont* fnt, int x, int y, GE_RGBA color, geCamera* camera, char *format,...){
	va_list			args;
    char			str[1024];
	// render the bitmaps corresponding to the characters in the string
	unsigned int ch = 0;
	geRect r = {0,0,0,0}; // the screen area to draw into
	int drawX = 0, drawY = 0; // drawing position
	GE_RGBA vertsColor[4];

	va_start(args, format);
    vsprintf(str, format, args);
	va_end(args);

	if ( !str ) return;

	

	// color of the vertices
	
	vertsColor[0] = color;
	vertsColor[1] = color;
	vertsColor[2] = color;
	vertsColor[3] = color;

	drawX = x;
	drawY = y;

	for ( ch = 0; ch < strlen(str); ch++ ){
		// support spaces even if they weren't added
		if ( str[ch] == ' ' || str[ch] == '	' /* <- tab */ )
		{
			// add a space
			drawX += fnt->m_spaceWidth;
		}

		// support carriage-returns
		if ( str[ch] == '\n' )
		{
			drawX = x;
			drawY += fnt->m_size;
		}

		if ( str[ch] < kFontLowRange || str[ch] > kFontHighRange ) // omit chars out of range
			continue;

		if ( fnt->m_chars[str[ch]].bitmap == NULL ) // omit chars not initialized
			continue;

		// calculate the screen area to draw into
		r.Left = drawX;

		// half the offset for particular chars (the offset makes them sit too low)
		if ( str[ch] == '~' ||
			 str[ch] == '=' || 
			 str[ch] == '`' ||
			 str[ch] == '\"' ||
			 str[ch] == '^' ||
			 str[ch] == '*' ||
			 str[ch] == '+'	|| 
			 str[ch] == 39 ) // apostrophe
		{
			r.Top = drawY + fnt->m_chars[str[ch]].offsetY / 2;
		}
		else
			r.Top = drawY + fnt->m_chars[str[ch]].offsetY;

		// drop the 'Q' down a little (the only capital that hangs)
		if ( str[ch] == 'Q' )
			r.Top += (int)((float)fnt->m_chars[str[ch]].height * 0.15f);

		// drop the hanging chars down a little
		if ( str[ch] == 'y' || str[ch] == 'g' || str[ch] == 'j' 
				|| str[ch] == 'p' || str[ch] == 'q' )
		{
			r.Top += (int)((float)fnt->m_chars[str[ch]].height * 0.33f); // by 1/3 their height
		}

		r.Right = r.Left + fnt->m_chars[str[ch]].width;
		r.Bottom = r.Top + fnt->m_chars[str[ch]].height;



		geEngine_DrawAlphaBitmap(
			fnt->m_engine,
			fnt->m_chars[str[ch]].bitmap,
			0,//geVec3d* VertUVArray
			camera,
			&r,
			0,//GE_Rect* PercentRect,
			255,//geFloat Alpha,
			vertsColor //GE_RGBA* RGBA_Array
			);

		// move to where we're going to draw the next char
		drawX += fnt->m_chars[str[ch]].width + 1; // 1 pixel space between chars
		//if ( drawX >= globals.theScreenRect.Right )
		//	break;// early out
	}
}
int XFont_GetStringWidth(XFont *fnt, char *str){
	int width = 0;
	XCharacter ch;
	int i;

	if ( !str ) return 0;
	for ( i = 0; i < (signed)strlen(str); i++ )
	{
		if ( str[i] == ' ' )// space
		{
			width += fnt->m_spaceWidth;
		}
		else
		{
			ch = fnt->m_chars[ str[i] ];
			width += ch.width+1; // 1 pixel between letters
		}
	}

	return width;
}
int XFont_GetStringHeight(XFont *fnt, char *str){
	int height = 0;// return the height of the tallest char
	XCharacter ch;
	int i;
	if ( !str ) return 0;
	
	for ( i = 0; i < (signed)strlen(str); i++ )
	{
		if ( str[i] == ' ' )//space
			continue;

		ch = fnt->m_chars[ str[i] ];
		if ( height < (ch.height - ch.offsetY) )
			height = ch.height - ch.offsetY;
	}

	return height;
}
XCharacter XFont_GetCharacter(XFont* fnt, char c){
	return fnt->m_chars[c];
}
char* XFont_GetName(XFont* fnt){
	return fnt->m_name;
}
int XFont_GetSize(XFont* fnt){
	return fnt->m_size;
}
geBoolean XFont_IsAntiAliased(XFont* fnt){
	return fnt->m_anti;
}