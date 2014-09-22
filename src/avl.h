#ifndef __AVL_H
#define __AVL_H

typedef struct _AVLTree AVLTree;

AVLTree* Avl_Create();
void Avl_Destroy(AVLTree* tree);

#define AVL_DESTROY(a)	{ Avl_Destroy(a); a=0; }

void Avl_Add(AVLTree *tree, int data, char* key);
int Avl_Load(AVLTree *tree, char* fileName);

int Avl_Find(AVLTree *tree, char* search, int* data);

#endif