#ifndef __INFGUI_H
#define __INFGUI_H

#define IG_EDIT		1
#define IG_BUTTON	2
#define IG_CHECK	3
#define IG_LABEL	4

typedef struct {
	char used;
	char* text;
	char* description;
	int x;
	int y;
	int type;

	int data;
	void (*action)();
} InfGui;

typedef struct _InfPage InfPage;

struct _InfPage{
	char* title;
	InfGui list[10];
//	InfPage* owner;
	int countsOfEdits;
	InfGui* theEdit;
	int goBackIndex;
	char* description;
};

void InfPage_sendClick(InfPage* to);
void InfPage_sendCharacter(InfPage* to, char c);
void InfPage_render(InfPage* to);
void InfPage_process(InfPage* to);
void InfPage_init(InfPage* to);
void InfPage_setGui(InfPage* p, int index, char* title, char* desc,  int x, int y, int type, int initData, void (*action)() );

#endif // __INFGUI_H