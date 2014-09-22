#include "inf_gui.h"
#include <stdarg.h>
#include "xfont.h"
#include "globalGenesis.h"

#define POS_HEADER_X			45
#define POS_HEADER_Y			53
#define POS_DESC_X				35
#define POS_DESC_Y				570

void meny_renderDescription(char* str){
	GE_RGBA color;

	color.r = color.g = color.b = 0.0f;
	color.a = 255.0f;

	XFontMgr_PrintAt(fntMgr, POS_DESC_X, POS_DESC_Y, kFontSmall, color, Camera, "%s", str );
}
void meny_renderHeader(char* str){
	GE_RGBA color;

	color.r = color.g = color.b = 0.0f;
	color.a = 255.0f;

	XFontMgr_PrintAt(fntMgr, POS_HEADER_X, POS_HEADER_Y, kFontLarge, color, Camera, "%s", str );
}
void meny_renderText(int x, int y, int on, char* str){
	GE_RGBA color;

	if( on ){
		color.r = color.g = color.b = 255.0f;
	}
	else {
		color.r = color.g = color.b = 0.0f;
	}
	color.a = 255.0f;
	XFontMgr_PrintAt(fntMgr, x, y, kFontMedium, color, Camera, "%s", str );
}

int meny_testWithinRegion(int x, int y, int rx, int ry, int lx, int ly){
	if( x<rx ) return 0;
	if( y<ry ) return 0;
	if( x > (rx+lx) ) return 0;
	if( y > (ry+ly) ) return 0;
	return 1;
}
int meny_testWithinText(int x, int y, int mx, int my, char* str){
	// int XFont_GetStringWidth(XFont *fnt, char *str);
	// int XFont_GetStringHeight(XFont *fnt, char *str);
	XFont* font;
	int w;
	int h;

	font = XFontMgr_GetFont(fntMgr, kFontMedium);
	h = XFont_GetSize(font);//, str);
	w = XFont_GetStringWidth(font, str);

	return meny_testWithinRegion(mx, my, x, y, w, h);
}












void InfGui_sendCharacter(InfGui* to, char c){
	switch( to->type ){
	case IG_BUTTON:
		break;
	case IG_CHECK:
		break;
	case IG_EDIT:
		break;
	case IG_LABEL:
		break;
	}
}
void InfGui_render(InfGui* to){
	if( !to->used ) return;
	meny_renderText(to->x, to->y, to->data, to->text);
}
char InfGui_within(InfGui* to, int mx, int my){
	return meny_testWithinText(to->x, to->y, mx, my, to->text);
}


void InfPage_sendClick(InfPage* to){
	POINT mouse;
	int i;

	GetCursorPos(&mouse);

	for(i=0;i<10; i++){
		if( InfGui_within(&(to->list[i]), mouse.x, mouse.y) ){
			to->list[i].action();
			break;
		}
	}
}

void InfPage_sendCharacter(InfPage* to, char c){
	if(! to->countsOfEdits) return;
	if( to->countsOfEdits == 1 ){
		// single edit
		InfGui_sendCharacter(to->theEdit, c);
	}
	else {
		// multiple edits
	}
}

void InfPage_render(InfPage* to){
	int i;
	// render header 45*53
	meny_renderHeader(to->title );

	// render gui
	for(i=0; i<10; i++){
		InfGui_render( &(to->list[i]) );
	}

	// render description 35 * 570
	if( to->description ){
		meny_renderDescription(to->description);
	}
}

void InfPage_process(InfPage* to){
	POINT mouse;
	int i;
	int data=0;

	GetCursorPos(&mouse);
	to->description = 0;

	for(i=0;i<10; i++){
		data = 0;
		if( InfGui_within(&(to->list[i]), mouse.x, mouse.y) ){
			to->description = to->list[i].description;
			data = 1;
		}
		if( to->list[i].type == IG_BUTTON ) to->list[i].data = data;
	}
}

void InfPage_init(InfPage* to){
	int i = 0;
	to->countsOfEdits = 0;
	to->goBackIndex = 0;
	to->theEdit = 0;
	to->title = 0;
	for(i=0; i<10; i++){
		to->list[i].action = 0;
		to->list[i].data = 0;
		to->list[i].description = 0;
		to->list[i].text = 0;
		to->list[i].type = 0;
		to->list[i].used = 0;
		to->list[i].x = 0;
		to->list[i].y = 0;
	}
}

void InfPage_setGui(InfPage* p, int index, char* title, char* desc,  int x, int y, int type, int initData, void (*action)() ){
	if (index >= 10 ) return;
	if( p->list[index].used ) return;
	if( type == IG_BUTTON ){
		if( p->countsOfEdits == 0 ){
			p->theEdit = &(p->list[index]);
		}
		else {
			p->theEdit = 0;
		}
		p->countsOfEdits ++;
	}

	p->list[index].type = type;
	p->list[index].used = 1;

	p->list[index].text = title;
	p->list[index].description = desc;
	p->list[index].x = x;
	p->list[index].y = y;
	p->list[index].data = initData;
	p->list[index].action = action;
}