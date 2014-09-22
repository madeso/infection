#include "linked_list.h"

#include "stdlib.h"
#include "string.h"
#include "stdio.h"

#define STRING_LENGTH 80

typedef struct _LinkedListNode LinkedListNode;

struct _LinkedListNode{
	int data;
	char key[STRING_LENGTH];
	LinkedListNode* next;
};

struct _LinkedList{
	LinkedListNode* first;
};

LinkedList* LL_Create(){
	LinkedList* result=0;
	result = malloc(sizeof(LinkedList) );
	if(! result ) return 0;
	memset(result, 0, sizeof(LinkedList) );
	return result;
}

void LL_Destroy(LinkedList* list){
	if( list ){
		LL_Clear(list);
		free(list);
		list = 0;
	}
}

void LL_Add(LinkedList *list, int data, char* key){
	LinkedListNode* node;
	if( LL_Find(list, key, 0) ){
		printLog("Linked list has already inserted that key\n");
		return;
	}

	if(! list->first ){
		LinkedListNode* n = 0;
		n = malloc( sizeof(LinkedListNode) );
		if(!n) return;
		memset(n, 0, sizeof(LinkedListNode) );
		list->first = n;
		n->data = data;
		strcpy(n->key, key);
		return;
	}

	node = list->first;

	while( node ){
		if( node->next ){
			node = node->next;
		}else{
			LinkedListNode* n = 0;
			n = malloc( sizeof(LinkedListNode) );
			if(!n) return;
			memset(n, 0, sizeof(LinkedListNode) );
			node->next = n;
			n->data = data;
			strcpy(n->key, key);
			return;
		}
	}
}

int LL_Load(LinkedList *list, char* fileName){
	FILE* f=0;
	char key[STRING_LENGTH];
	int data;
	int count=0;

	f = fopen(fileName, "r");
	if(!f) return 0;
	while( !feof(f) ){
		memset(key, 0, sizeof(char)*STRING_LENGTH);
		data = 0;
		fscanf(f, "%s %i", key, &data);
		if( strlen(key) > 0 ){
			LL_Add(list, data, key);
			count++;
		}
	}

	fclose(f);
	{
		char str[400];
		sprintf(str, "\n\n\n\n\n****The count is %i\n\n", count);
		printLog(str);
	}
	return 1;
}

int LL_Find(LinkedList *list, char* key, int* data){
	LinkedListNode* node;

	node = list->first;
	while( node ){
		if( strcmp(key, node->key) == 0){
			*data = node->data;
			return node->data;
		}
		node= node ->next;
	}

	return 0;
}

void LL_Remove(LinkedList *list, char* key){
	LinkedListNode *node= list->first;

	if( list->first ){
		if( strcmp(list->first->key, key) == 0){
			list->first = node->next;
			free(node);
			return;
		}
	}

	while( node ){
		if( node->next ){
			if( strcmp(node->next->key, key) == 0 ){
				// handle the next one
				LinkedListNode *temp=node->next;
				node->next = node->next->next;
				free(temp);
				return;
			}
		}
		node = node->next;
	}
}

void LL_Clear(LinkedList *list){
	LinkedListNode *node= list->first, *temp=0;
	
	while( node ){
		temp = node;
		node = node->next;

		free(temp);
	}

	list->first = 0;
}