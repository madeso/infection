#include "inf_gui.h"
#include "extra_genesis.h"
#include <stdlib.h>
#include "log.h"
#include "memory.h"
#include "globalGenesis.h"

#define BS_UP	0
#define BS_DOWN	1
typedef struct _Button{
	geBitmap* normal;
	geBitmap* over;
	int32 maxx;
	int32 maxy;
	int32 x;
	int32 y;
	ON_CLICK_FUNCTION* fn_callback;
	int state;
} Button;

typedef struct _component {
	void* data;
	int type;
} Component;

struct _GUIHandler{
	Component components[NUMBER_OF_COMPONENTS];
	int index;
};

#define TYPE_UNUSED		0
#define TYPE_BUTTON		1

#define MOUSE_GOING_UP		-1
#define MOUSE_UP			0
#define MOUSE_GOING_DOWN	1
#define MOUSE_DOWN			2

GUIHandler* gui_init(){
	GUIHandler *handler=0;
	int i;

	handler = malloc(sizeof(GUIHandler) );
	memset(handler, 0, sizeof(GUIHandler) );

	for(i=0; i<NUMBER_OF_COMPONENTS;i++){
		handler->components[i].type = TYPE_UNUSED;
		handler->components[i].data = 0;
	}
	handler->index = 0;

	return handler;
}

geBoolean gui_updateButton(Button* button, int32 x, int32 y, int mousestate){
	geBoolean theReturn = GE_TRUE;
	geBoolean withinButton = GE_FALSE;

	if( x >= button->x && y >= button->y ){
		if( x <= button->maxx && y <= button->maxy ){
			withinButton = GE_TRUE;
		}
	}

	if( withinButton ){
		button->state = BS_DOWN;

		if( mousestate == MOUSE_GOING_DOWN ){
			theReturn = button->fn_callback();
		}

	} else {
		button->state = BS_UP;
	}

	return theReturn;
}

int mouseState = MOUSE_UP;

geBoolean gui_update(GUIHandler* handler, int32 x, int32 y, geBoolean mousePressed){
	int i = 0;
	geBoolean result;

	// decide mouse state
	static geBoolean previousMousePressed = GE_FALSE;
	if( mousePressed != previousMousePressed ){
		if(previousMousePressed){
			mouseState = MOUSE_GOING_UP;
		} else {
			mouseState = MOUSE_GOING_DOWN;
		}
	} else {
		if( previousMousePressed ){
			mouseState = MOUSE_DOWN;
		} else {
			mouseState = MOUSE_UP;
		}
	}
	previousMousePressed = mousePressed;

	// update everything
	for(i=0; i<handler->index; i++){
		switch(handler->components[i].type ){
		case TYPE_BUTTON:
			result = gui_updateButton(handler->components[i].data, x, y, mouseState);
			if( !result ) return GE_FALSE;
			break;
		case TYPE_UNUSED:
			return GE_TRUE;
		default:
			printLog("Data unknown in gui_delete\n");
			break;
		}
	}

	return GE_TRUE;
}

geBoolean gui_renderButton(Button* button, geEngine* Engine){
	geBitmap* bitmap=0;

	if( button->state == BS_DOWN ){
		bitmap = button->over;
	} else {
		bitmap = button->normal;
	}

	return geEngine_DrawBitmap(Engine, bitmap, NULL, button->x, button->y);
}

geBoolean gui_render(GUIHandler* handler, geEngine* Engine){
	int i=0;
	geBoolean result = GE_FALSE;

	// update everything
	for(i=0; i<handler->index; i++){
		switch(handler->components[i].type ){
		case TYPE_BUTTON:
			result = gui_renderButton(handler->components[i].data, Engine);
			if(! result ) return GE_FALSE;
			break;
		case TYPE_UNUSED:
			return GE_TRUE;
		default:
			printLog("Data unknown in gui_delete\n");
			break;
		}
	}

	return GE_TRUE;
}

void gui_deleteButton(Button* button){
	killBitmap(button->normal);
	killBitmap(button->over);
	free(button);
}

void gui_delete(GUIHandler* handler){
	int i = 0;

	for(i=0; i<handler->index; i++){
		switch(handler->components[i].type ){
		case TYPE_BUTTON:
			gui_deleteButton(handler->components[i].data);
			break;
		case TYPE_UNUSED:
			return;
		default:
			printLog("Data unknown in gui_delete\n");
			break;
		}
	}

	free(handler);
}

geBoolean gui_addButton(GUIHandler* handler, int32 x, int32 y, int32 width, int32 height, char* normalFile, char* overFile, geBoolean useAlpha, ON_CLICK_FUNCTION* onUse){
	Button* button=0;

	if( handler->index >= NUMBER_OF_COMPONENTS ){
		printLog("Gui won't add button since there is too many components in the container\n");
		return GE_FALSE;
	}

	button=malloc(sizeof(Button) );
	memset(button, 0, sizeof(Button) );
	button->x = x;
	button->y = y;
	button->maxx = x + width;
	button->maxy = y + height;
	button->fn_callback = onUse;

	if( useAlpha ){
		button->over = loadBitmapExColorKey(overFile, width, height, Engine, 0);
		button->normal = loadBitmapExColorKey(normalFile, width, height, Engine, 0);
		/*button->over = LoadBmp(overFile);
		button->normal = LoadBmp(normalFile);
		*/
	} else{
		/*button->over = LoadBmpNoColorKey(overFile);
		button->normal = LoadBmpNoColorKey(normalFile);*/

		button->over = loadBitmapEx(overFile, width, height, Engine);
		button->normal = loadBitmapEx(normalFile, width, height, Engine);
	}

	if( !button->over ){
		printLog("Failed to load over bitmap\n");
		return GE_FALSE;
	}
	if( !button->normal ){
		printLog("Failed to load bormal bitmap\n");
		return GE_FALSE;
	}

	handler->components[handler->index].type = TYPE_BUTTON;
	handler->components[handler->index].data = button;
	handler->index++;

	return GE_TRUE;
}