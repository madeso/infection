#include "genesis.h"
#include "explosions.h"
#include "damage.h"
#include "enemies.h"
#include "globalGenesis.h"
#include "sound_modder.h"
#include "time_damage.h"

int explosion_getDamage(geVec3d* explosionLocation, geVec3d* pos, float range, int damage){
	geVec3d temp;
	float normalizedDistance;
	float sqrange;

	geVec3d_Subtract(explosionLocation, pos, &temp);
	sqrange = geVec3d_LengthSquared(&temp);
	normalizedDistance = sqrange / range;

	if( normalizedDistance < 1.0f ) {
		return (int)((float)damage * (1.0f-normalizedDistance));
	}

	return 0;
}

void explosion_at(geVec3d* location, float range, int pdamage){
	int killSound;
	killSound = explosion_getDamage(location, &(XForm.Translation),range*3.0f, 100 );
	enemy_explosionDamage(location, range, pdamage);
	damage( explosion_getDamage(location, &(XForm.Translation),range, pdamage ) );
	if( killSound > 0){
		float val = (float)(100-killSound) / 100.0f;
		sm_set( val );
	}
}

void fire_explosion_at(geVec3d* location, float range, int pdamage){
	enemy_fireExplosionDamage(location, range, pdamage);
	timedamage_add(TIMEDAMAGE_FIRE, explosion_getDamage(location, &(XForm.Translation),range, pdamage ) );
}