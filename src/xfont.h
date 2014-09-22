#ifndef XFONT_H
#define XFONT_H

#include "genesis.h"

#define kFontLowRange		33
#define kFontHighRange		126
#define kFontMax			24
#define kFontSmall			0
#define kFontMedium			1
#define kFontLarge			2
#define kNameLength			80


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
	geEngine* m_engine;
	int m_spaceWidth;
} XFont;

typedef struct _FontMgr { 
	XFont *m_fonts[kFontMax];
} XFontMgr;



XFontMgr* XFontMgr_FontMgr(geEngine* theEngine);
XFontMgr_Destructor(XFontMgr* mgr);
void XFontMgr_Create(XFontMgr* mgr, geEngine* theEngine);
void XFontMgr_Destroy(XFontMgr* mgr);
int XFontMgr_LoadFont(XFontMgr* mgr, char *name, int size, geBoolean anti, geEngine* theEngine);
void XFontMgr_UnloadFont(XFontMgr* mgr, int index);
XFont* XFontMgr_GetFont(XFontMgr* mgr, int index);
void XFontMgr_PrintAt(XFontMgr* mgr, int x, int y, int font, GE_RGBA color, geCamera* camera, char *format, ...);


XFont* XFont_Font();
void XFont_Destructor(XFont* fnt);
geBoolean XFont_Create(XFont* fnt, char *name, int size, geBoolean anti, geEngine* theEngine);
void XFont_Destroy(XFont* fnt);
void XFont_PrintAt(XFont* fnt, int x, int y, GE_RGBA color, geCamera* camera, char *format,...);
int XFont_GetStringWidth(XFont *fnt, char *str);
int XFont_GetStringHeight(XFont *fnt, char *str);
XCharacter XFont_GetCharacter(XFont* fnt, char c);
char* XFont_GetName(XFont* fnt);
int XFont_GetSize(XFont* fnt);
geBoolean XFont_IsAntiAliased(XFont* fnt);



#endif