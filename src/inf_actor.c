#include "inf_actor.h"
#include "log.h"


geActor_Def* LoadActorDef(char* fileName)
{
	//local variables
	geVFile *File=0;
	geActor_Def* def=0;

	//open the file:
	File = geVFile_OpenNewSystem(NULL, GE_VFILE_TYPE_DOS, fileName, NULL, GE_VFILE_OPEN_READONLY);
	if( !File )
	{
		printLog("Couldn't open file.\n");
		return 0;
	}
	
	def = geActor_DefCreateFromFile(File);
	geVFile_Close(File);

	if( !def )
	{
		printLog("Failed to get Actor def.\n");
		return 0;
	}


	return def;
}

geActor* LoadActor(geActor_Def* def, geWorld* World, float scale, uint32 flags, geXForm3d* xf)
{
	geActor* r=0;
	
	r = geActor_Create(def);
	if( !r ) return 0;

	geActor_SetScale(r, scale, scale, scale);
	if( !geWorld_AddActor(World, r, flags, 0x11111111 ) )
	{
		geActor_Destroy( &r ); r=0;
		return 0;
	}
	geActor_ClearPose(r, xf);

	return r;
}

void KillActor(geWorld* World, geActor** act)
{
	if( *act )
	{
		geActor_Destroy(act);
		geWorld_RemoveActor(World, *act);
		*act = 0;
	}
}

void setup_box(geExtBox* eb, geActor* act, geVec3d pos)
{
	geExtBox_Set(eb, -10,0,-10,10,10,10);

	// set the pose to a default 
	if (act) 
	{
		geExtBox Box;

		geActor_GetDynamicExtBox(act, &Box);
		geVec3d_Subtract(&Box.Min, &pos, &Box.Min);
		geVec3d_Subtract(&Box.Max, &pos, &Box.Max);
		geExtBox_Set(eb, Box.Min.X,Box.Min.Y,Box.Min.Z,Box.Max.X,Box.Max.Y,Box.Max.Z);
		geActor_SetExtBox(act, &Box, NULL);
		geActor_SetRenderHintExtBox(act, &Box, NULL);
	}
}

void setup_box_ex(geActor* act)
{
	geXForm3d xform;
	geXForm3d_SetIdentity(&xform);

	// set the pose to a default 
	if (act) 
	{
		geExtBox Box;

		geActor_ClearPose(act, &xform); 
		geActor_GetDynamicExtBox(act, &Box);
		geActor_SetExtBox(act, &Box, NULL);
	}
}