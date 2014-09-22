#include "genesis.h"

#include "extra_genesis.h"

#include "Assert.h"
#include "globalGenesis.h"
#include "bitmap.h"
#include "math.h"
#include "log.h"



// NOTE: begin Jeffs Code

// By Jeff
// URL: http://www.genesis3d.com/forum/viewtopic.php?topic=999858&forum=3
//

void killBitmap(geBitmap* bitmap){
	if( bitmap ){
		geEngine_RemoveBitmap(Engine, bitmap);
		geBitmap_Destroy(&bitmap);
		bitmap = 0;
	}
}

/*
One of the more common questions on this forum is "How do I scale a bitmap".
Well I wrote a function that will copy and scale the source bitmap so that it
fits on the destination bitmap. So if the user loads a 800x600 bitmap but the screen
resolution is 640x480 then all the user need to do is create a bitmap that is 640x480
and use this function to copy and scale the 800x600 bitmap to the 640x480 bitmap. 

This function has not been thoroughly tested so there could be problems, I tested it using
the Direct3D driver in windows mode. Here is the code: 

Note: Assert.h and bitmap.h header files must be included. 
*/

void ScaleBitmap(geBitmap *srcBitmap,geBitmap *destBitmap)
{

    geBitmap *srcLock,*destLock;
    geBoolean success;
    geBitmap_Info srcInfo,destInfo;
    uint16 *srcBits,*destBits,*srcPtr,*destPtr;
    double incX,incY;
    uint16 x,y;
    
      
	success = geBitmap_LockForWriteFormat(srcBitmap,&srcLock,0,0,GE_PIXELFORMAT_16BIT_565_RGB);
	if ( ! success )
	{

		success = geBitmap_SetFormat(srcBitmap,GE_PIXELFORMAT_16BIT_565_RGB,GE_TRUE,0,NULL);
		assert(success);


		success = geBitmap_LockForWriteFormat(srcBitmap,&srcLock,0,0,GE_PIXELFORMAT_16BIT_565_RGB);
		assert(success);
	}


	success = geBitmap_LockForWriteFormat(destBitmap,&destLock,0,0,GE_PIXELFORMAT_16BIT_565_RGB);
	if ( ! success )
	{

		success = geBitmap_SetFormat(destBitmap,GE_PIXELFORMAT_16BIT_565_RGB,GE_TRUE,0,NULL);
		assert(success);


		success = geBitmap_LockForWriteFormat(destBitmap,&destLock,0,0,GE_PIXELFORMAT_16BIT_565_RGB);
		assert(success);
	}


	success = geBitmap_GetInfo(srcLock,&srcInfo,NULL);
	assert(success);
		 
	success = geBitmap_GetInfo(destLock,&destInfo,NULL);
	assert(success);


	srcBits = geBitmap_GetBits(srcLock);
	assert( srcBits );

	destBits = geBitmap_GetBits(destLock);
	assert( destBits );


	srcPtr = srcBits;
	destPtr = destBits; 
	

	incX = ((float)srcInfo.Width/(float)destInfo.Width);
	incY = ((float)srcInfo.Height/(float)destInfo.Height);

	
    for(y=0; y < destInfo.Height; y++)
	{
		for(x=0; x < destInfo.Width; x++)
		{
			
		    destPtr[x+(y*destInfo.Stride)] = srcPtr[((int)(x*incX)+((int)(y*incY)*srcInfo.Stride))];
								
		}
    		
	}
	
	destBits = srcBits = destPtr = srcPtr = NULL;

	success = geBitmap_UnLock(destLock);
	assert(success);

	success = geBitmap_UnLock(srcLock);
	assert(success);

	success = geBitmap_RefreshMips(destBitmap);
	assert(success);	
} 

/*
Usage:
To use this function in your program you can do something like this: 

geBitmap *srcBitmap; 
geBitmap *destBitmap; 
geBitmap_Info Info; 

srcBitmap = geBitmap_CreateFromFileName(NULL,"HUD.bmp"); 

geBitmap_GetInfo(srcBitmap,&Info,NULL); 
destBitmap = geBitmap_Create(640, 480, Info.MaximumMip+1, Info.Format); 

ScaleBitmap(srcBitmap,destBitmap);
*/

// NOTE: end Jeffs Code

geBitmap* loadBitmapEx(char* fileName, int width, int height, geEngine* Engine)
{
	geBitmap *srcBitmap; 
	geBitmap *destBitmap; 
	geBitmap_Info Info; 

	srcBitmap = geBitmap_CreateFromFileName(NULL, fileName); 

	if( ! srcBitmap )
	{
		char str[300];
		sprintf(str, "Failed to load %s in loadBitmapEx()\n", fileName);
		error(str);
		printLog(str);
		return 0;
	}

	geBitmap_GetInfo(srcBitmap,&Info,NULL); 
	destBitmap = geBitmap_Create(width, height, Info.MaximumMip+1, Info.Format); 
	ScaleBitmap(srcBitmap,destBitmap);

	geBitmap_Destroy(&srcBitmap ); // destroy the old one
	geEngine_AddBitmap(Engine, destBitmap); // add it to the engine

	return destBitmap;
}

geBitmap* loadBitmapExColorKey(char* fileName, int width, int height, geEngine* Engine, int index)
{
	geBitmap *srcBitmap; 
	geBitmap *destBitmap; 
	geBitmap_Info Info; 

	srcBitmap = geBitmap_CreateFromFileName(NULL, fileName); 

	if( ! srcBitmap )
	{
		char str[300];
		sprintf(str, "Failed to load %s in loadBitmapEx()\n", fileName);
		error(str);
		printLog(str);
		return 0;
	}

	if( geBitmap_SetColorKey(srcBitmap, GE_TRUE, index, GE_FALSE) == GE_FALSE )
	{
		printLog("Failed to set color key.\n");
		geBitmap_Destroy(&srcBitmap);
		return 0;
	}

	geBitmap_GetInfo(srcBitmap,&Info,NULL); 
	destBitmap = geBitmap_Create(width, height, Info.MaximumMip+1, Info.Format); 
	ScaleBitmap(srcBitmap,destBitmap);

	geBitmap_Destroy(&srcBitmap ); // destroy the old one

	/*{
		geBoolean success;
		geBitmap_Info Info;
		int index;
	
		geBitmap_GetInfo(destBitmap,&Info,NULL);

		index = gePixelFormat_ComposePixel(Info.Format,0,0,255,0);

		geBitmap_SetColorKey(destBitmap, GE_TRUE, index, GE_FALSE );
	}*/

	geEngine_AddBitmap(Engine, destBitmap); // add it to the engine

	return destBitmap;
}

// NOTE: begin Sevens Code
float AnglesToPoint( geVec3d *s, geVec3d *d, geVec3d *a )
{
	float x,l;
	geVec3d_Subtract( d,s,a );
	l = geVec3d_Length( a );
	
	// protect from Div by Zero
	if(l>0.0f) 
	{
		x = a->X;
		a->X = (geFloat)( GE_PI*0.5 ) - (geFloat)acos(a->Y / l);
		a->Y = (geFloat)atan2( x , a->Z ) + GE_PI;
		// roll is zero - always!!?
		a->Z = 0.0;
	}
	
	return l;
}

void LookAt(geVec3d from, geVec3d point, geVec3d *Result)
{
	AnglesToPoint(&from,&point,Result);
}

// NOTE: end Sevens Code

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//			renderBitmap
//				wrapper for the Genesis3d function
//				renders a bitmap
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
geBoolean renderBitmap(geBitmap *bmp, int x, int y)
{
	if( !geEngine_DrawBitmap(Engine, bmp, NULL, x, y ) )
	{
		printLog("Failed in render Image.\n");
		return GE_FALSE;
	}

	return GE_TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		LoadBmp
//			Function for easy the loading of bitmaps with transperency
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
geBitmap* LoadBmp(char *filename)
{
	//local variables
	geVFile *File;
	geBitmap *Bitmap = 0;

	printLog("Loading bitmap: ");
	printLog(filename);
	printLog("\n");

	//open the file:
	File = geVFile_OpenNewSystem(NULL, GE_VFILE_TYPE_DOS, filename, NULL, GE_VFILE_OPEN_READONLY);
	if( !File )
	{
		printLog("Couldn't open file.\n");
		return 0;
	}
	Bitmap = geBitmap_CreateFromFile(File);
	geVFile_Close(File);


	if( geBitmap_SetColorKey(Bitmap, GE_TRUE, 0, GE_FALSE) == GE_FALSE )
	{
		printLog("Failed to set color key.\n");
		geBitmap_Destroy(&Bitmap);
		return 0;
	}

	if( geEngine_AddBitmap(Engine, Bitmap) == GE_FALSE)
	{
		printLog("Failed to add bitmap to engine.\n");
		geBitmap_Destroy(&Bitmap);
		return 0;
	}
	printLog("Loading ok.\n");

	return Bitmap;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

geBitmap* LoadBmpNoColorKey(char *filename)
{
	//local variables
	geVFile *File;
	geBitmap *Bitmap = 0;

	printLog("Loading bitmap: ");
	printLog(filename);
	printLog("\n");

	//open the file:
	File = geVFile_OpenNewSystem(NULL, GE_VFILE_TYPE_DOS, filename, NULL, GE_VFILE_OPEN_READONLY);
	if( !File )
	{
		printLog("Couldn't open file.\n");
		return 0;
	}
	Bitmap = geBitmap_CreateFromFile(File);
	geVFile_Close(File);

	if( geEngine_AddBitmap(Engine, Bitmap) == GE_FALSE)
	{
		printLog("Failed to add bitmap to engine.\n");
		geBitmap_Destroy(&Bitmap);
		return 0;
	}
	printLog("Loading ok.\n");

	return Bitmap;
}


// returns GE_TRUE if the square of the distance betwen pos and pos2 is less than squaredRange, returns GE_FALSE otherwise
geBoolean isWithinSquaredRange( geVec3d* pos, geVec3d* pos2, float squaredRange )
{
	geVec3d result;
	float f;

	if( !pos || !pos2 ) return GE_TRUE;

	geVec3d_Subtract(pos, pos2, &result);
	f = geVec3d_LengthSquared(&result);

	if( f <= squaredRange )
		return GE_TRUE;

	return GE_FALSE;
}