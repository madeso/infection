#include "genesis.h"

#ifndef _PROPOSE_G3_H
#define _PROPOSE_G3_H

//Purpose: To reflect an In vector off of a plane (defined by Normal), scaled by Scale
//               (This could be used to bounce a ball off a wall).
//Input:      In: Input vector. Does not need to be normalized
//              Normal: A normalized vector from surface to be reflected from.
//              Out: The resulting vector
//              Scale: A scale factor for resulting output vector. 
//                         An example of use would be to set at 0.75, so that reflected velocity loses 25% in reflection.
//Note: The general formula is: Out = In - 2*Normal*(In dot Normal)
void geVec3d_Reflect(geVec3d* In, geVec3d* Normal, geVec3d* Out, geFloat Scale);
geBoolean geActor_Collision(geActor* actor, geVec3d *from, geVec3d *to);

#endif