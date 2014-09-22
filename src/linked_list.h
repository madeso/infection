#ifndef __LINKED_LIST_H
#define __LINKED_LIST_H

typedef struct _LinkedList LinkedList;

LinkedList* LL_Create();
void LL_Destroy(LinkedList* list);

#define LL_DESTROY(a)	{ LL_Destroy(a); a=0; }

void LL_Add(LinkedList *list, int data, char* key);
int LL_Load(LinkedList *list, char* fileName);

int LL_Find(LinkedList *list, char* key, int* data);

void LL_Remove(LinkedList *list, char* key);
void LL_Clear(LinkedList *list);

#endif