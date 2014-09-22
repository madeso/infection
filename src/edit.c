#include "edit.h"
#include "string.h"
#include "globalGenesis.h"


void EditBox_set(sEditBox *seb, char* toSet){
	if( !seb ) return;
	seb->cPos = strlen(toSet);
	sprintf(seb->data, "%s", toSet);
}

void EditBox_push_char(sEditBox *seb, char c)
{
	if( !seb ) return;
	if( seb->cPos >= MAX_CHAR) return;
	seb->data[seb->cPos]=c;
	seb->cPos++;
	seb->data[seb->cPos]=0;
}

void EditBox_pop(sEditBox *seb)
{
	if( !seb ) return;
	if( seb->cPos == 0) return;
	seb->cPos--;
	seb->data[seb->cPos]=0;
}

geBoolean EditBox_xpop(sEditBox *seb, char* c)
{
	if( !seb ) return GE_FALSE;
	if( seb->cPos == 0) return GE_FALSE;
	seb->cPos--;
	*c = seb->data[seb->cPos];
	seb->data[seb->cPos]=0;

	return GE_TRUE;
}

void EditBox_init(sEditBox *seb, char* str, int x, int y)
{
	if( !seb ) return;
	seb->cPos = strlen(str);
	sprintf(seb->data, "%s", str);
	seb->x = x;
	seb->y = y;
}

void EditBox_render(sEditBox *seb)
{
	if( !seb ) return;

	geEngine_Printf(Engine, seb->x , seb->y , seb->data);
}

void EditBox_clear(sEditBox *seb, char *str)
{
	if( !seb ) return;
	
	if( str )
		sprintf(str, "%s", seb->data);
	sprintf(seb->data, "");
	seb->cPos = 0;
}