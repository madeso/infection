#include "damage.h"
#include "player.h"
#include "sound_system.h"
#include "playerSounds.h"
#include "useful_macros.h"
#include "infection.h"
#include "enemies.h"

void kill() {
	soundsys_play_sound( &die[generate_random(5)], GE_FALSE );
	hero_hit_points = 0;
	player_is_alive = GE_TRUE;
	add_redFlash( 3.0f );
}

void damagex(int dmg, int type){
	if( type == DAMAGE_ARMORPIERCING )
		armor_piercing_damage(dmg);
	else
		damage(dmg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		armor_piercing_damage
//			does armor piercing damage to the player
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void armor_piercing_damage(int damage)
{
	{
		float val = damage / 100.0f;
		val *= 3.0f;
		//if( val < 0.2f ) val = 0.2f;
		add_redFlash( val );
	}

	if( !cheats.god )
	{
		if( damage >= hero_hit_points )
		{
			kill();
		}
		else
		{
			//play sound damage
			soundsys_play_sound( &hurt[generate_random(7)], GE_FALSE );
			//knockback
			hero_hit_points -= damage;
		}
	}
	else
	{
		soundsys_play_sound( &hurt[generate_random(7)], GE_FALSE );
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		damage
//			does "normal" damage to the player (armor can block)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void damage(int damage)
{
	if( damage <= 0 ) return;

	if(hero_hit_points > 0)
	{
		if(hero_armor_points > 0)
		{
			if( damage > hero_armor_points )
			{
				armor_piercing_damage( damage - hero_armor_points);
				hero_armor_points = 0;
			}
			else
			{
				hero_armor_points -= damage;
				soundsys_play_sound( &armor[generate_random(5)], GE_FALSE );
			}
		}
		else
		{
			armor_piercing_damage(damage);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

















