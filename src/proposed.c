#include "proposeg3d.h"

void geVec3d_Reflect(geVec3d* In, geVec3d* Normal, geVec3d* Out, geFloat Scale)
{
    geVec3d_AddScaled (In, Normal, -2*geVec3d_DotProduct (In, Normal), Out);
    geVec3d_Scale (Out, Scale, Out);
}

geBoolean geActor_Collision(geActor* actor, geVec3d *from, geVec3d *to){
	geExtBox eb;

	geActor_GetDynamicExtBox(actor, &eb);

	if( geExtBox_RayCollision( &eb, from, to, 
								NULL, NULL ) // don't save the data
								)
	{
		return GE_TRUE;
	}

	return GE_FALSE;
}

geBoolean geActor_Collision_Ex(geActor* actor, geExtBox* playerEb, geVec3d *from, geVec3d *to){
#define MIN 30.0f
	geExtBox eb;
	geExtBox pathEb;
	geExtBox res;


	geExtBox_LinearSweep(playerEb, from, to, &pathEb);

	geActor_GetDynamicExtBox(actor, &eb);

	if( geExtBox_Intersection(&pathEb, &eb, &res) )

	/*if( geExtBox_RayCollision( &eb, from, to, 
								NULL, NULL ) // don't save the data
								)*/
	{
		return GE_TRUE;
	}

	return GE_FALSE;
#undef MIN
}