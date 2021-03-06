#include "subtitles.h"
#include "globalGenesis.h"


// assume the charwidth is 10
#define CHAR_WIDTH 10
#define Y_OFFSET 30
char subtitle[300];
geBoolean enabled = GE_TRUE;
GE_RGBA subtitleColor;

void subtitle_newGame(){
	// make sure we know what the string contains!
	memset(subtitle, 0, 300);

	subtitleColor.r = subtitleColor.g = subtitleColor.b = 255.0f;
	subtitleColor.a = 255.0f;
}

char* subtitle_getString(){
	if( enabled )
		return subtitle;
	return "";
}

int subtitle_getPrintX(){
	int length=0;
	int printWidth=0;
	if( !enabled ) return -1;
	length = strlen(subtitle);
	printWidth = length * CHAR_WIDTH;
	return (int)( (Width-printWidth) / 2.0f );
}

void subtitle_setSubtitle(const char* newString){
	strcpy(subtitle, newString);
}

void subtitle_SetEnable(geBoolean newEnable){
	enabled = newEnable;
}

geBoolean render_subTitle(){
	int xpos = subtitle_getPrintX();
	if( xpos == -1 ) return GE_TRUE;
	//if(! geEngine_Printf(Engine, xpos, Height - Y_OFFSET , subtitle_getString() ) ) return GE_FALSE;

	XFontMgr_PrintAt(fntMgr, xpos, Height - Y_OFFSET , kFontSmall, subtitleColor, Camera, "%s", subtitle_getString() );
	return GE_TRUE;
}