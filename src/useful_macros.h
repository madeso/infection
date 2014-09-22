/*
useful_macros.h
Useful defines and macros
*/

//macro to help fixing the
//GE_RAM_ALLOCATE_STRUCT / free problem

#ifndef		ALLOCATE_STRUCT
#define		ALLOCATE_STRUCT(s)	(s*)malloc( sizeof(s) )
#endif

#ifndef		FLOAT_ABS
#define		FLOAT_ABS(a)		(a>0.0f) ? a : -a
#endif

#ifndef		RAND_FLOAT
#define		RAND_FLOAT()		((float)rand() / RAND_MAX)
#endif

#ifndef		RAND_INVERT
#define		RAND_INVERT(x)		rand()%2?(x):-(x)
#endif

#ifndef METERS
#define METERS(x)				x*30
#endif

#ifndef DEG
#define DEG						(GE_PI/180)
#endif

#ifndef STRING_LENGTH
#define STRING_LENGTH			100
#endif

#ifndef generate_random
#define generate_random(x)		rand()%x
#endif

#define IN_DEG(x) x = (x)*GE_PI/180.0f

#define IN_DEG_VECTOR(v) { IN_DEG(v.X); IN_DEG(v.Y); IN_DEG(v.Z); }