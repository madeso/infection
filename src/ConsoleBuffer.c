#include "ConsoleBuffer.h"
#include "stdlib.h"
#include "memory.h"
#include "string.h"

typedef struct _CBNode CBNode;

struct _CBNode{
	char input[200];
	CBNode* next;
	CBNode* previous;
};

struct _ConsoleBuffer{
	CBNode* first;
	CBNode* last;
	CBNode* current;
	unsigned int count;
	unsigned int maxcount;
};

char* consoleBuffer_previous(ConsoleBuffer *buffer){
	CBNode* node = 0;
	if(! buffer->current ){
		buffer->current = buffer->last;
		if(! buffer->current ){
			return 0;
		}
		return buffer->current->input;
	}
	if( buffer->current ){
		node = buffer->current->previous;
		if( !node ) return buffer->current->input;
		buffer->current = node;
		return node->input;
	}
	return 0;
}

char* consoleBuffer_next(ConsoleBuffer *buffer){
	CBNode* node=0;
	if( buffer->current ){
		buffer->current = buffer->current->next;
		if( buffer->current ) return buffer->current->input;
		else return 0;
	}
	return 0;
}

char* consoleBuffer_resetCursor(ConsoleBuffer *buffer){
	buffer->current = 0;
	return 0;
}

void consoleBuffer_add(ConsoleBuffer* buffer, char* strToAdd){
	CBNode* toAdd = malloc(sizeof(CBNode) );
	if( !toAdd ) return;
	memset(toAdd, 0, sizeof(CBNode) );
	strcpy(toAdd->input, strToAdd);
	buffer->count++;

	if( buffer->first == 0 || buffer->last == 0 ){
		buffer->first = buffer->last = toAdd;
		toAdd->next = toAdd->previous = 0;
	} else {
		CBNode* last = 0;
		if( buffer->count > buffer->maxcount ){
			CBNode* first = buffer->first;
			buffer->first = first->next;
			buffer->first->previous = 0;
			buffer->count--;
			free(first);
		}
		last = buffer->last;
		last->next = toAdd;
		toAdd->previous = last;
		toAdd->next = 0;
		buffer->last = toAdd;
	}

	buffer->current = 0;
}

void consoleBuffer_delete(ConsoleBuffer* buffer){
	CBNode* node = buffer->first;
	CBNode* temp;
	while( node ){
		temp = node;
		node = node->next;
		free(temp);
	}
	free(buffer);
}

ConsoleBuffer* consoleBuffer_init(unsigned int maximumCount){
	ConsoleBuffer* buffer = malloc(sizeof(ConsoleBuffer) );
	if(! buffer ) return 0;
	memset(buffer, 0, sizeof(ConsoleBuffer) );
	buffer->maxcount = maximumCount;
	return buffer;
}