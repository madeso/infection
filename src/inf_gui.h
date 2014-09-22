#include "genesis.h"

#ifndef __INF_GUI_H
#define __INF_GUI_H

typedef geBoolean ON_CLICK_FUNCTION();
typedef struct _GUIHandler GUIHandler;

#define NUMBER_OF_COMPONENTS	10

GUIHandler* gui_init();
geBoolean gui_update(GUIHandler* handler, int32 x, int32 y, geBoolean mousePressed);
void gui_delete(GUIHandler* handler);
geBoolean gui_render(GUIHandler* handler, geEngine* Engine);

geBoolean gui_addButton(GUIHandler* handler, int32 x, int32 y, int32 width, int32 height, char* normalFile, char* overFile, geBoolean useAlpha, ON_CLICK_FUNCTION* onUse);

#endif