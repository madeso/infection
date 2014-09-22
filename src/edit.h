/*
edit.h
This is a basic implementation of an editbox(like the file->new: "file name" box).
It is basicly a stack that has the operations push and pop.
Then it has the operation xpop(extensive pop) that returns the pop-ed character.
*/


#ifndef EDIT_H
#define EDIT_H

#include "genesis.h"

// maximum number of characters on the stack
#define MAX_CHAR	60

typedef struct
{
	char data[MAX_CHAR]; // the stack
	unsigned char cPos;//currentPos in string
	int x; // x position
	int y; // y position
}sEditBox;

void EditBox_push_char(sEditBox *seb, char c); // pushes a character on the edit-box stack
void EditBox_pop(sEditBox *seb); // pops a character from the edit-box's stack
void EditBox_init(sEditBox *seb, char* str, int x, int y); // initializes all the data of the edixbox
void EditBox_render(sEditBox *seb); // renders the edit box
geBoolean EditBox_xpop(sEditBox *seb, char* c); // pops and returns a character from the edit-box's stack
void EditBox_clear(sEditBox *seb, char *str); // clears the entitre stack of the edit-box
void EditBox_set(sEditBox *seb, char* toSet);

#endif