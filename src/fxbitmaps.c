#include "fxbitmaps.h"
#include "extra_genesis.h"
#include "EffManager.h"
#include "decals.h"
#include "TPool.h"
#include "globalGenesis.h"
#include "log.h"
#include "EffParticle.h"
#include <math.h>
#include "proposeg3d.h"

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
geBitmap* electricBolt;

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

	flameSprite = 0;
	smokeSprite = 0;
	gemSprite = 0;
	fireSprite = 0;

	electricBolt = 0;
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

	flameSprite = LOAD_BITMAP(".//gfx//fx//flame03.bmp", ".//gfx//fx//a_flame.bmp");
	smokeSprite = LOAD_BITMAP(".//gfx//fx//lvsmoke.bmp", ".//gfx//fx//A_lvsmoke.bmp");
	gemSprite = LOAD_BITMAP(".//gfx//fx//Gem.bmp", ".//gfx//fx//a_Gem.bmp");
	fireSprite = LOAD_BITMAP(".//gfx//fx//flame03.bmp", ".//gfx//fx//a_fire.bmp");
	
	electricBolt = LOAD_BITMAP(".//gfx//fx//bolt.bmp",".//gfx//fx//bolt.bmp");

	TEST_BITMAP(electricBolt, "Failed to load bolt");
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

	TEST_BITMAP(flameSprite	,"Failed to load flame texture");
	TEST_BITMAP(smokeSprite	,"Failed to load smoke texture");
	TEST_BITMAP(gemSprite	,"Failed to load gem texture");
	TEST_BITMAP(fireSprite	,"Failed to load gem texture");

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
	ElectricBolt bolt;
	GE_RGBA rgba;
	int res=0;

	rgba.r = 160.0f;
	rgba.g = 160.0f;
	rgba.b = 255.0f;
	rgba.a = 255.0f;

	memset(&bolt, 0, sizeof(ElectricBolt) );

	bolt.imortal = GE_FALSE;
	bolt.life = 0.5f;
	bolt.origin = from;
	bolt.Terminus = to;
	bolt.Width = 8;
	bolt.NumPoints = 64;
	bolt.Intermittent = 0;
	bolt.MinFrequency = 4.0f;
	bolt.MaxFrequency = 1.0f;
	bolt.Wildness = 0.3f;
	bolt.DominantColor = 2;
	bolt.Color = rgba;
	bolt.Texture = electricBolt;
	
	res = EM_Item_Add(EM, EFF_ELECTRICBOLT, &bolt);
	if( res == -1){
		system_message("Bolt bad");
	}
}
#undef SCALE

/*
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
*/

geBoolean fncb_killParticle(Particle* particle, GE_Collision* data, geVec3d* from, geVec3d* to){
	return GE_TRUE;
}
geBoolean fncb_bounceParticle(Particle* particle, GE_Collision* data, geVec3d* from, geVec3d* to){
	geVec3d_Reflect(&(particle->ptclVelocity), &(data->Plane.Normal), &(particle->ptclVelocity), 0.5f);
	return GE_FALSE;
}
geBoolean fncb_fireParticle(Particle* particle, GE_Collision* data, geVec3d* from, geVec3d* to){
	if( data->Actor ) {
		//data->Actor
	}
	else {
		geVec3d impact;
		geVec3d_AddScaled(&(data->Impact), &(data->Plane.Normal), 1.0f, &impact);
		fx_fire(&impact, GE_FALSE);
	}

	// this looked bad
	/*{
		geVec3d wIn;
		geXForm3d_GetIn(&XForm, &wIn);
		if( ! DecalMgr_AddDecal( dMgr, DecalMgr_GetRandomDecal( DECALTYPE_SCORCHSM ),
			-1.0, RGBA_Array, 1.0f, &(data->Impact),
			&(data->Plane.Normal), &wIn)
			)
			system_message("Failed to add decal");
	}*/

	return GE_TRUE;
}

void fx_grenadeExplosion(geVec3d at){
#define MIN 0.1f
#define SPHERE_POS_TYPE 1
#define PI 3.1415926535
	geExtBox bb;
	int i;

	int count;
	geVec3d gravity;
	geBitmap* texture;
	float minLife;
	float maxLife;
	float minSpeed;
	float maxSpeed;
	GE_LVertex vertex;
	geVec3d* pos;

	geVec3d velocity;
	float life;
	double theta;
	double beta;
	float speed;

	vertex.u = 0.0f;
	vertex.v = 0.0f;
	vertex.r = 255.0f;
	vertex.g = 255.0f;
	vertex.b = 255.0f;
	vertex.a = 255.0f;

	geVec3d_Clear(&gravity);
	geExtBox_Set(&bb, -1.0f * MIN , -1.0f * MIN , -1.0f * MIN , MIN, MIN, MIN);
	pos = (geVec3d*)(&vertex.X);
	*pos = at;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// Smoke
	///////////////////////////////////////////////////////////////////////////////////////////////////

	texture = smokeSprite;
	count = 10;
	minLife = 6.0f;
	maxLife = 12.0f;
	minSpeed = 70.0f;
	maxSpeed = 140.0f;
	gravity.Y = -50.0f;

	maxLife -= minLife;
	maxSpeed -= minSpeed;
	for(i=0; i<count; i++){
		life = minLife + (RAND_FLOAT() * maxLife);
		speed = minSpeed + (RAND_FLOAT() * maxSpeed);

		// generate sphereical coordinates
#if SPHERE_POS_TYPE != 3
		theta = RAND_FLOAT() * 2 * PI;
#pragma message("theta = RAND_FLOAT() * 2 * PI;")
#endif
#if SPHERE_POS_TYPE != 2
		beta = RAND_FLOAT() * PI;
#pragma message("beta = RAND_FLOAT() * PI;")
#endif

#if SPHERE_POS_TYPE == 3
		theta = (i/count) * 2 * PI;
#pragma message("theta = (i/count) * 2 * PI;")
#endif
#if SPHERE_POS_TYPE == 2
		beta = (i/count) * PI;
#pragma message("beta = (i/count) * PI;")
#endif
		//convert from spherical coordinates to cartesian
		velocity.X = speed * (float)( cos(theta) * sin(beta) );
		velocity.Y = speed * (float)( sin(theta) * sin(beta) + 1.5 );
		velocity.Z = speed * (float)( cos(beta) );
		// set it a bit from the middle
		//geVec3d_AddScaled(&at, &velocity, RAND_FLOAT(), pos);
		Particle_SystemAddParticle(EM->Ps, texture, &vertex, 0, life, &velocity, 4.0f, 2.0f, GE_TRUE, &gravity, fncb_bounceParticle, &bb);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// Fire
	///////////////////////////////////////////////////////////////////////////////////////////////////
	texture = flameSprite;
	count = 13;
	minLife = 6.0f;
	maxLife = 10.0f;
	minSpeed = 10.0f;
	maxSpeed = 25.0f;
	gravity.Y = -10.0f;

	maxLife -= minLife;
	maxSpeed -= minSpeed;
	for(i=0; i<count; i++){
		life = minLife + (RAND_FLOAT() * maxLife);
		speed = minSpeed + (RAND_FLOAT() * maxSpeed);

		// generate sphereical coordinates
#if SPHERE_POS_TYPE != 3
		theta = RAND_FLOAT() * 2 * PI;
#endif
#if SPHERE_POS_TYPE != 2
		beta = RAND_FLOAT() * PI;
#endif

#if SPHERE_POS_TYPE == 3
		theta = (i/count) * 2 * PI;
#endif
#if SPHERE_POS_TYPE == 2
		beta = (i/count) * PI;
#endif
		//convert from spherical coordinates to cartesian
		velocity.X = speed * (float)( cos(theta) * sin(beta) );
		velocity.Y = speed * (float)( sin(theta) * sin(beta) + 1.5 );
		velocity.Z = speed * (float)( cos(beta) );
		// set it a bit from the middle
		//geVec3d_AddScaled(&at, &velocity, RAND_FLOAT(), pos);
		// fncb_bounceParticle
		// fncb_killParticle
		Particle_SystemAddParticle(EM->Ps, texture, &vertex, 0, life, &velocity, 5.0f, 7.0f, GE_TRUE, &gravity, fncb_killParticle, &bb);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// Gem
	///////////////////////////////////////////////////////////////////////////////////////////////////

	texture = gemSprite;
	count = 2;
	minLife = 0.5f;
	maxLife = 3.0f;
	minSpeed = 100.0f;
	maxSpeed = 150.0f;
	gravity.Y = -100.0f;

	maxLife -= minLife;
	maxSpeed -= minSpeed;
	for(i=0; i<count; i++){
		life = minLife + (RAND_FLOAT() * maxLife);
		speed = minSpeed + (RAND_FLOAT() * maxSpeed);

		// generate sphereical coordinates
#if SPHERE_POS_TYPE != 3
		theta = RAND_FLOAT() * 2 * PI;
#endif
#if SPHERE_POS_TYPE != 2
		beta = RAND_FLOAT() * PI;
#endif

#if SPHERE_POS_TYPE == 3
		theta = (i/count) * 2 * PI;
#endif
#if SPHERE_POS_TYPE == 2
		beta = (i/count) * PI;
#endif
		//convert from spherical coordinates to cartesian
		velocity.X = speed * (float)( cos(theta) * sin(beta) );
		velocity.Y = speed * (float)( sin(theta) * sin(beta) + 1.5 );
		velocity.Z = speed * (float)( cos(beta) );
		// set it a bit from the middle
		//geVec3d_AddScaled(&at, &velocity, RAND_FLOAT(), pos);
		Particle_SystemAddParticle(EM->Ps, texture, &vertex, 0, life, &velocity, 1.0f, 1.0f, GE_FALSE, &gravity, fncb_bounceParticle, &bb);
	}

#undef MIN
}
void fx_fire(geVec3d* at, geBoolean anchor){
	Spray sp;
	geVec3d to;
	int index;
	GE_RGBA rgba;
	geVec3d direction;
	rgba.r = rgba.g = rgba.b = 255.0f; rgba.a = 255.0f;

	geVec3d_Set(&direction, 0.0f, 1.0f, 0.0f); 
	//geVec3d_AddScaled(&from, &direction, 5.0f, &to);
	memset(&sp, 0, sizeof(Spray) );

	if( anchor ){
		to = direction;
		sp.AnchorPoint = at;
		geVec3d_Clear(& ( sp.Source) );
		sp.Dest = direction;
	} else {
		sp.AnchorPoint = 0;
		sp.Source = *at;
		 geVec3d_Add(at, &direction, &(sp.Dest) );
	}

	sp.Texture = flameSprite;
	sp.SprayLife = 2.0f; // change to 0.5f
	sp.Rate = 0.3f;
	sp.ColorMin = rgba;
	sp.ColorMax = rgba;
	//geVec3d_Clear(&(sp.Gravity));
	sp.ShowAlways=GE_TRUE;
	sp.MinScale = 2.0f;
	sp.MaxScale = 5.0f;
	sp.MinUnitLife = 0.7f;
	sp.MaxUnitLife = 1.0f;
	sp.MinSpeed = 70.0f;
	sp.MaxSpeed = 80.0f;
	sp.ScaleStyle = SSS_RANDOM_SCALE;
	sp.waitTime = 0.0f;

	index = EM_Item_Add(EM, EFF_SPRAY, &sp);
}
void fx_shootFire(geVec3d at, geVec3d direction){
#define MIN 5.0f
	geExtBox bb;
	GE_LVertex vertex;
	geVec3d* pos;

	vertex.u = 0.0f;
	vertex.v = 0.0f;
	vertex.r = 255.0f;
	vertex.g = 255.0f;
	vertex.b = 255.0f;
	vertex.a = 255.0f;

	geExtBox_Set(&bb, -1.0f * MIN , -1.0f * MIN , -1.0f * MIN , MIN, MIN, MIN);
	pos = (geVec3d*)(&vertex.X);
	*pos = at;
	Particle_SystemAddParticle(EM->Ps, fireSprite, &vertex, 0, 0.25f, &direction, 1.0f, 17.0f, GE_TRUE, 0, fncb_fireParticle, &bb);
#undef MIN
}