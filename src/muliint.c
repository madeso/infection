#include "multiint.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		toInt3
//			converts a integer to a int3
//			used in the hud
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int3 toInt3(int i)
{
	int3 result;
	int2 r;
	int temp;

	temp = i % 100;
	if( temp == i )
	{
		// less than 100
		result.hundred = 0;
	}
	else
	{
		//larger than 99
		result.hundred = (i-temp)/100;
	}
	r = toInt2(temp);

	result.ten = r.ten;
	result.one = r.one;
	return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		toInt2
//			converts a integer to a int2
//			used in the hud
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int2 toInt2(int i)
{
	int2 result;
	result.one = i% 10;
	result.ten = (i - result.one )/10;
	return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////