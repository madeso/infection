#include "fxbitmaps.h"
#include "extra_genesis.h"
#include "EffManager.h"
#include "decals.h"
#include "TPool.h"
#include "globalGenesis.h"
#include "log.h"

/*
geBitmap*	stoneSmoke,
			snowSmoke,
			electricSmoke,
			pipeSmoke,
			gravelSmoke,
			mudSmoke;
*/
#define ELECTRIC_SPARK_BITMAPS 10
#define SIMPLE_SPARK_BITMAPS 6

geBitmap* electricSpark[ELECTRIC_SPARK_BITMAPS];
geBitmap* simpleSpark[SIMPLE_SPARK_BITMAPS];

geBitmap* bloodTexture;

void fxbitmaps_init(){
	int i = 0;
	stoneSmoke = 0;
	snowSmoke = 0;
	electricSmoke = 0;
	pipeSmoke = 0;
	gravelSmoke = 0;
	mudSmoke = 0;

	for(i=0; i< ELECTRIC_SPARK_BITMAPS; i++){
		electricSpark[i] = 0;
	}

	for(i=0; i< SIMPLE_SPARK_BITMAPS; i++){
		simpleSpark[i] = 0;
	}

	marbleSprite = 0;
	grassSprite = 0;
	woodSprite = 0;
	glassSprite = 0;
	leafSprite = 0;
	sofaSprite = 0;
	rockSprite = 0;
	sandSprite = 0;
	bloodTexture = 0;
}

#define TEST_BITMAP(bitmap, error_message )		if( !bitmap ){ \
													error(error_message); \
													result = GE_FALSE; \
												}

#define LOAD_SMOKE_BITMAP(texture)		TPool_Bitmap(texture, ".//gfx//fx//a_smoke.bmp", 0, 0, World)
#define LOAD_BITMAP(texture, alpha)		TPool_Bitmap(texture, alpha, 0, 0, World)

geBoolean fxbitmaps_load(){
	geBoolean result = GE_TRUE;

	stoneSmoke=		LOAD_SMOKE_BITMAP(".//gfx//fx//stoneSmoke.bmp");
	snowSmoke=		LOAD_SMOKE_BITMAP(".//gfx//fx//snowSmoke.bmp");
	electricSmoke=	LOAD_SMOKE_BITMAP(".//gfx//fx//electricSmoke.bmp");
	pipeSmoke=		LOAD_SMOKE_BITMAP(".//gfx//fx//pipeSmoke.bmp");
	gravelSmoke=	LOAD_SMOKE_BITMAP(".//gfx//fx//gravelSmoke.bmp");
	mudSmoke=		LOAD_SMOKE_BITMAP(".//gfx//fx//mudSmoke.bmp");

	electricSpark[0] = LOAD_BITMAP(".//gfx//fx//electric//el1.bmp", ".//gfx//fx//electric//ael1.bmp");
	electricSpark[1] = LOAD_BITMAP(".//gfx//fx//electric//el2.bmp", ".//gfx//fx//electric//ael2.bmp");
	electricSpark[2] = LOAD_BITMAP(".//gfx//fx//electric//el3.bmp", ".//gfx//fx//electric//ael3.bmp");
	electricSpark[3] = LOAD_BITMAP(".//gfx//fx//electric//el4.bmp", ".//gfx//fx//electric//ael4.bmp");
	electricSpark[4] = LOAD_BITMAP(".//gfx//fx//electric//el5.bmp", ".//gfx//fx//electric//ael5.bmp");
	electricSpark[5] = LOAD_BITMAP(".//gfx//fx//electric//el6.bmp", ".//gfx//fx//electric//ael6.bmp");
	electricSpark[6] = LOAD_BITMAP(".//gfx//fx//electric//el7.bmp", ".//gfx//fx//electric//ael7.bmp");
	electricSpark[7] = LOAD_BITMAP(".//gfx//fx//electric//el8.bmp", ".//gfx//fx//electric//ael8.bmp");
	electricSpark[8] = LOAD_BITMAP(".//gfx//fx//electric//el9.bmp", ".//gfx//fx//electric//ael9.bmp");
	electricSpark[9] = LOAD_BITMAP(".//gfx//fx//electric//el10.bmp", ".//gfx//fx//electric//ael10.bmp");

	simpleSpark[0] = LOAD_BITMAP(".//gfx//fx//spark//spark1.bmp", ".//gfx//fx//spark//aspark1.bmp");
	simpleSpark[1] = LOAD_BITMAP(".//gfx//fx//spark//spark2.bmp", ".//gfx//fx//spark//aspark2.bmp");
	simpleSpark[2] = LOAD_BITMAP(".//gfx//fx//spark//spark3.bmp", ".//gfx//fx//spark//aspark3.bmp");
	simpleSpark[3] = LOAD_BITMAP(".//gfx//fx//spark//spark4.bmp", ".//gfx//fx//spark//aspark4.bmp");
	simpleSpark[4] = LOAD_BITMAP(".//gfx//fx//spark//spark5.bmp", ".//gfx//fx//spark//aspark5.bmp");
	simpleSpark[5] = LOAD_BITMAP(".//gfx//fx//spark//spark6.bmp", ".//gfx//fx//spark//aspark6.bmp");

	marbleSprite = LOAD_BITMAP(".//gfx//fx//marbleGlassSprite.bmp", ".//gfx//fx//AMarbleSprite.bmp");
	grassSprite = LOAD_BITMAP(".//gfx//fx//grassSprite.bmp", ".//gfx//fx//AGrassSprite.bmp");
	woodSprite = LOAD_BITMAP(".//gfx//fx//woodSprite.bmp", ".//gfx//fx//AWoodSprite.bmp");
	glassSprite = LOAD_BITMAP(".//gfx//fx//marbleGlassSprite.bmp", ".//gfx//fx//AGlassSprite.bmp");
	leafSprite = LOAD_BITMAP(".//gfx//fx//leafSprite.bmp", ".//gfx//fx//ALeafSprite.bmp");

	bloodTexture = LOAD_BITMAP(".//gfx//fx//bloodSprite.bmp", ".//gfx//fx//AGrassSprite.bmp");
	//sofaSprite = LOAD_BITMAP(".//gfx//fx//sofaSprite.bmp", ".//gfx//fx//ASofaSprite.bmp");
	//rockSprite = LOAD_BITMAP(".//gfx//fx//rockSprite.bmp", ".//gfx//fx//ARockSprite.bmp");
	//sandSprite = LOAD_BITMAP(".//gfx//fx//sandSprite.bmp", ".//gfx//fx//ASandSprite.bmp");
	
	TEST_BITMAP(stoneSmoke,		"Failed to load stoneSmoke");
	TEST_BITMAP(snowSmoke,		"Failed to load snowSmoke");
	TEST_BITMAP(electricSmoke,	"Failed to load electricSmoke");
	TEST_BITMAP(pipeSmoke,		"Failed to load pipeSmoke");
	TEST_BITMAP(gravelSmoke,	"Failed to load gravelSmoke");
	TEST_BITMAP(mudSmoke,		"Failed to load mudSmoke");

	TEST_BITMAP(electricSpark[1],		"Failed to load electric spark 1");
	TEST_BITMAP(electricSpark[2],		"Failed to load electric spark 2");
	TEST_BITMAP(electricSpark[3],		"Failed to load electric spark 3");
	TEST_BITMAP(electricSpark[4],		"Failed to load electric spark 4");
	TEST_BITMAP(electricSpark[5],		"Failed to load electric spark 5");
	TEST_BITMAP(electricSpark[6],		"Failed to load electric spark 6");
	TEST_BITMAP(electricSpark[7],		"Failed to load electric spark 7");
	TEST_BITMAP(electricSpark[8],		"Failed to load electric spark 8");
	TEST_BITMAP(electricSpark[9],		"Failed to load electric spark 9");
	TEST_BITMAP(electricSpark[0],		"Failed to load electric spark 0");

	TEST_BITMAP(electricSpark[0],		"Failed to load simple spark 1");
	TEST_BITMAP(electricSpark[1],		"Failed to load simple spark 2");
	TEST_BITMAP(electricSpark[2],		"Failed to load simple spark 3");
	TEST_BITMAP(electricSpark[3],		"Failed to load simple spark 4");
	TEST_BITMAP(electricSpark[4],		"Failed to load simple spark 5");
	TEST_BITMAP(electricSpark[5],		"Failed to load simple spark 6");

	TEST_BITMAP(marbleSprite	,"Failed to load marble sprite");
	TEST_BITMAP(grassSprite		,"Failed to load grass sprite");
	TEST_BITMAP(woodSprite		,"Failed to load wood sprite");
	TEST_BITMAP(glassSprite		,"Failed to load glass sprite");
	TEST_BITMAP(leafSprite		,"Failed to load leaf sprite");

	TEST_BITMAP(bloodTexture	,"Failed to load blood texture");
	//TEST_BITMAP(sofaSprite		,"Failed to load sofa sprite");
	//TEST_BITMAP(rockSprite		,"Failed to load rock sprite");
	//TEST_BITMAP(sandSprite		,"Failed to load sand sprite");

	return result;
}

void fxbitmaps_free(){
	// make sure everything is zero
	fxbitmaps_init();
}

void createSmokeEmitter(geBitmap* bitmap, geVec3d from, geVec3d direction, SmokeEmitterStyle style){
	Spray sp;
	geVec3d to;
	int index;
	GE_RGBA rgba;
	rgba.r = rgba.g = rgba.b = 255.0f; rgba.a = 255.0f;

	geVec3d_AddScaled(&from, &direction, 5.0f, &to);
	memset(&sp, 0, sizeof(Spray) );


	sp.Texture = bitmap;

	if( style == SES_LONG_AND_LARGE )
		sp.SprayLife = 1.2f;
	else
		sp.SprayLife = 0.2f;
	sp.Rate = 0.05f;
	sp.AnchorPoint = 0;
	sp.ColorMin = rgba;
	sp.ColorMax = rgba;
	//geVec3d_Clear(&(sp.Gravity));
	sp.ShowAlways=GE_TRUE;
	sp.Source = from;
	sp.Dest = to;
	sp.MinScale = 0.3f;
	sp.MaxScale = 1.3f;
	if( style == SES_LARGE || style == SES_LONG_AND_LARGE ){
		sp.MinUnitLife = 1.0f;
		sp.MaxUnitLife = 1.2f;
	} else {
		sp.MinUnitLife = 0.5f;
		sp.MaxUnitLife = 0.7f;
	}
	sp.MinSpeed = 130.0f;
	sp.MaxSpeed = 130.0f;
	sp.ScaleStyle = SSS_SIZE_UP;
	sp.waitTime = 0.0f;

	index = EM_Item_Add(EM, EFF_SPRAY, &sp);
}

void createExplosionSmokeEmitter(geBitmap* bitmap, geVec3d from, float waitTime){
	Spray sp;
	geVec3d to;
	int index;
	GE_RGBA rgba;
	geVec3d direction;
	rgba.r = rgba.g = rgba.b = 255.0f; rgba.a = 255.0f;

	geVec3d_Set(&direction, 0.0f, 1.0f, 0.0f); 
	geVec3d_AddScaled(&from, &direction, 5.0f, &to);
	memset(&sp, 0, sizeof(Spray) );


	sp.Texture = bitmap;
	sp.SprayLife = 1.0f; // change to 0.5f
	sp.Rate = 0.2f;
	sp.AnchorPoint = 0;
	sp.ColorMin = rgba;
	sp.ColorMax = rgba;
	//geVec3d_Clear(&(sp.Gravity));
	sp.ShowAlways=GE_TRUE;
	sp.Source = from;
	sp.Dest = to;
	sp.MinScale = 0.3f;
	sp.MaxScale = 0.3f;
	sp.MinUnitLife = 0.7f;
	sp.MaxUnitLife = 0.9f;
	sp.MinSpeed = 70.0f;
	sp.MaxSpeed = 70.0f;
	sp.ScaleStyle = SSS_RANDOM_SCALE;
	sp.waitTime = waitTime;

	index = EM_Item_Add(EM, EFF_SPRAY, &sp);
}

void createElectricSparkle(geVec3d at){
	Sprite sp;
	int index;
	GE_RGBA rgba;

	rgba.r = rgba.g = rgba.b = 255.0f; rgba.a = 255.0f;
	memset(&sp, 0, sizeof(Sprite) );


	sp.Pos = at;
	sp.Color = rgba;
	sp.Texture = electricSpark;
	sp.TotalTextures = ELECTRIC_SPARK_BITMAPS;
	sp.TextureRate = 0.1f;
	sp.Scale = 0.25f;
	sp.ScaleRate = 0.07f;
	sp.Rotation = 0.0f;
	sp.RotationRate = 0.0f;
	sp.AlphaRate = 0.1f;
	sp.Style = SPRITE_CYCLE_ONCE;

	index = EM_Item_Add(EM, EFF_SPRITE, &sp);
}

void createSimpleSparkle(geVec3d at){
	Sprite sp;
	int index;
	GE_RGBA rgba;

	rgba.r = rgba.g = rgba.b = 255.0f; rgba.a = 255.0f;
	memset(&sp, 0, sizeof(Sprite) );


	sp.Pos = at;
	sp.Color = rgba;
	sp.Texture = simpleSpark;
	sp.TotalTextures = SIMPLE_SPARK_BITMAPS;
	sp.TextureRate = 0.08f;
	sp.Scale = 2.0f;
	sp.ScaleRate = 0.0f;
	sp.Rotation = 0.0f;
	sp.RotationRate = 0.0f;
	sp.AlphaRate = 0.0f;
	sp.Style = SPRITE_CYCLE_ONCE;

	index = EM_Item_Add(EM, EFF_SPRITE, &sp);
}

void ParticleExplosion(geBitmap* sprite, geVec3d location, geVec3d direction){
	geVec3d gravity;
	if( !sprite ) return;

	geVec3d_Scale(&direction, 100.0f, &direction);

	geVec3d_Set(&gravity, 0.0f, -60.0f, 0.0f);
	/*if(! EM_ParticleExplosion(EM, location, direction, sprite, 0.0f, gravity, 0.0f,
		0.0f, 6, 20.0f, 2.0f) ) system_message("Failed to explode particle");*/

	if(! EM_ParticleExplosion(EM, location, direction, sprite, 1.0f, gravity, 5.0f,
		5.0f, 2, 2.0f, 0.25f) ) system_message("Failed to explode particle");
}

#define MIN 0.1f
void BloodExplosion(geVec3d position){
	Blood sp;
	int i;
	int count;
	GE_RGBA rgba;
	geVec3d gravity;
	/*geVec3d direction;

	geVec3d_Subtract(&XForm.Translation, &position, &direction); 
	geVec3d_Normalize(&direction); */

	count = 3;

	rgba.r = 255.0f;
	rgba.g = 0.0f;
	rgba.b = 0.0f;
	rgba.a = 255.0f;
	
	memset(&sp, 0, sizeof(Blood) );
	geVec3d_Set(&gravity, 0.0f, -150.0f, 0.0f);

	sp.Color = rgba;
	sp.decalColor = rgba;
	sp.Gravity = gravity;
	sp.Position = position;
	sp.Texture = bloodTexture;
	sp.TextureScale = 0.5f;
	sp.mgr = dMgr;
	sp.decalType = DECALTYPE_SCORCHSM;
	geExtBox_Set(&sp.ExtBox, -1.0f * MIN , -1.0f * MIN , -1.0f * MIN , MIN, MIN, MIN); 

#define SCALE_SIZE	80.0f
	for( i=0; i<count; i++){
		//geVec3d tempVec;
		geVec3d_Set(&sp.Velocity, RAND_INVERT(SCALE_SIZE*RAND_FLOAT()), SCALE_SIZE*RAND_FLOAT(), RAND_INVERT(SCALE_SIZE*RAND_FLOAT()) );
		sp.life = 1.0f + 0.5f * RAND_FLOAT();
		/*geVec3d_Scale(&direction, 40.0f + 40.0f*RAND_FLOAT(), &sp.Velocity);
		geVec3d_Add(&sp.Velocity, &tempVec, &sp.Velocity);*/
		EM_Item_Add(EM, EFF_BLOOD, &sp);
	}
#undef SCALE_SIZE
}
#undef MIN

#define SCALE 20.0f
void fx_blast(geVec3d from, geVec3d to){
	Bolt bolt;
	GE_RGBA rgba;
	int res=0;

	rgba.r = 0.0f;
	rgba.g = 0.0f;
	rgba.b = 255.0f;
	rgba.a = 255.0f;

	memset(&bolt, 0, sizeof(Bolt) );

	bolt.Texture = electricSpark[rand()%ELECTRIC_SPARK_BITMAPS];
	bolt.CompleteLife = 1.0f;
	bolt.Start = from;
	bolt.Start.X += RAND_INVERT(RAND_FLOAT()) * SCALE;
	bolt.Start.Y += RAND_INVERT(RAND_FLOAT()) * SCALE;
	bolt.Start.Z += RAND_INVERT(RAND_FLOAT()) * SCALE;

	bolt.End = to;
	bolt.SegmentLength = 1400;
	bolt.SegmentWidth = 30;
	bolt.Offset = 0.0f;
	bolt.BoltLimit = 1;
	bolt.BoltLife = 1.0f;
	bolt.BoltCreate = 1.0f;
	bolt.Loop = GE_FALSE;
	bolt.Color = rgba;
	res = EM_Item_Add(EM, EFF_BOLT, &bolt);
	if( res == -1){
		system_message("Bolt bad");
	}
}
#undef SCALE