/*
multiint.h
Simple data convertions for printing numbers on scrren with bitmaps aa numbers
*/

#ifndef MULTIINT_H
#define MULTIINT_H

typedef struct int3_tag
{
	unsigned char hundred;
	unsigned char ten;
	unsigned char one;
}int3;

typedef struct int2_tag
{
	unsigned char ten;
	unsigned char one;
}int2;

int3 toInt3(int i);
int2 toInt2(int i);

#endif