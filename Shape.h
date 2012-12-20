#ifndef __SHAPE__
#define __SHAPE__
#include "const.h"

typedef struct _Colour
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
} Colour;

typedef struct _Point 
{
	int x;
	int y;
} Point;

typedef struct _Line 
{
	Point start;
	Point end;
} Line;

typedef struct _Region

{
	Point* point;
	int regionsize;
	int pointnum;
} Region;

typedef struct _Object
{
	int top;
	int bottom;
	int left;
	int right;

	BOOL ishuman;
	Colour colour;

} Object;

typedef struct _Target
{
	Object* objects;
	int targetsize;
	int targetnum;
} Target;


#endif