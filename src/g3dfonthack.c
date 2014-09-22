// geFont TYPE DEFINITIONS (so we can get at its naughty bits)
	// Yes, this is a little hacky, but I really don't want to rewrite all of that code
	//	since the G3D font loading works just fine and leaves the bitmaps conveniently in
	//	geBitmap format... it's just that geFont_DrawText is far too slow.

	// Fonts will now be loaded using the geFont routines, but their bitmaps will be added
	//	to a geWorld and the font will be rendered with geEngine_DrawAlphaBitmap

//*******************************************************************************
typedef struct geFontBitmap
{
   void *next;
   geBitmap *map;
   int16 freeX, freeY;  // place the next character here.

} geFontBitmap;

//*******************************************************************************
typedef struct geFontCharacterRecord
{
   geFontBitmap *bitmapUsed;  // NULL == not initialized yet.
   GE_Rect bitmapRect;
   int32 offsetX, offsetY, fullWidth;

} geFontCharacterRecord;

//*******************************************************************************
typedef struct geFont 
{
   char fontNameString[64];
   int16 fontSize;
   int16 fontWeight;
   geFontBitmap *bitmapList;
   geFontCharacterRecord characterArray[256];
   const geEngine *Engine;
   geBitmap *buffer;
   geBoolean antialiased;
   int32 refCount;
/////////
   GE_RGBA color;
   geBitmap_Palette *palette;
/////////
} geFont;