#include "defs.h"
#include "game_math.h"
#include "entities.h"

#define WAVE_ESCALATION_RATE 4
#define ITEM_ACCUMULATE_RATE 1

float pickUpAccumulator = 0;

float get_entity_score_value(Entity_Types type) {
	float result = 0.0f;
	switch(type) {
		case ENTITY_TYPE_ENEMY_DRIFTER: {result = 0.25; } break;
		case ENTITY_TYPE_ENEMY_UFO: { result = 1.0f; } break;
		case ENTITY_TYPE_ENEMY_TRACKER: { result = 3.0f; } break;
		case ENTITY_TYPE_ENEMY_TURRET: { result = 4.0f; } break;
		case ENTITY_TYPE_ENEMY_GRAPPLER: { result = 5.0f; } break;
		default: break;
	}

	return result;
}

Vector2 get_clear_spawn(Game_State* game, float radius, SDL_Rect boundary) {
	Vector2 result = {
		.x = (boundary.x + radius) + random() * (boundary.x + boundary.w - radius),
		.y = (boundary.y + radius) + random() * (boundary.y + boundary.h - radius),
	};

	Vector2 overlap = {0};
	Entity* entities = game->entities;
	for (int i = 0; i < game->entity_count; i++) {
		
		if (sc2d_check_circles(result.x, result.y, radius,
			entities[i].x, entities[i].y, entities[i].collision_radius, &overlap.x, &overlap.y)) {

			result.x -= overlap.x;
			result.y -= overlap.y;
		}
	}

	if (game->player) {
		if (sc2d_check_circles(result.x, result.y, radius, game->player->x, game->player->y, game->player->collision_radius, &overlap.x, &overlap.y)) {
			result.x -= overlap.x;
			result.y -= overlap.y;
		}
	}

	return result;
}

void spawn_wave(Game_State* game, int wave, int points_max) {
	const SDL_Rect spawn_zones[] = {
		{0  , 0  , 390, 290}, 
		{410, 0  , 390, 290}, 
		{0  , 310, 390, 290},
		{410, 330, 390, 290}
	};
	int zone_index = (random() * (array_length(spawn_zones))) - 1;

	//Add new enemy types every 5 waves
	int maxValue = ((float)wave / (float)WAVE_ESCALATION_RATE + 0.5f);
	maxValue = SDL_clamp(maxValue, ENTITY_TYPE_ENEMY_DRIFTER, ENTITY_TYPE_ENEMY_GRAPPLER);

	for (float points_remaining = (float)points_max; points_remaining > 0;) {
		//Generate random type between 0 and current maximum point value
		
		int spawn_type = ENTITY_TYPE_ENEMY_DRIFTER + (int)(random() * (float)(ENTITY_TYPE_ENEMY_GRAPPLER - ENTITY_TYPE_ENEMY_DRIFTER) + 0.5f);
		int spawn_value = get_entity_score_value(spawn_type);

		if (spawn_value && points_remaining >= spawn_value) {
			points_remaining -= spawn_value;
			float entity_radius = 25;
			Vector2 new_position = get_clear_spawn(game, entity_radius, spawn_zones[zone_index]);
			
			Entity* warp = spawn_entity(game, ENTITY_TYPE_SPAWN_WARP, new_position);
			warp->data.spawn_warp.spawn_type = spawn_type;
		}
		
		zone_index = (zone_index + 1) % array_length(spawn_zones);
	}
}

/*
void forceCircle(x, y, radius, force) {
	let deltaX = 0;
		deltaY = 0;
		deltaAng = 0;
	for (let i = 0; i < allEntities.length; i++) {
		if (allEntities[i].mass > 0 && circleIntersect(x, y, radius, allEntities[i].x, allEntities[i].y, allEntities[i].collision_radius)) {
			deltaX = x - allEntities[i].x;
			deltaY = y - allEntities[i].y;
			deltaAng = Math.atan2(deltaY, deltaX);

			allEntities[i].xv -= Math.cos(deltaAng) * force;
			allEntities[i].yv -= Math.sin(deltaAng) * force;
		}
	}

	if (circleIntersect(x, y, radius, p1.x, p1.y, p1.collision_radius)) {
		deltaX = x - p1.x;
		deltaY = y - p1.y;
		if (deltaX == 0 && deltaY == 0) return;//Prevent p1 froming pushing itself
		deltaAng = Math.atan2(deltaY, deltaX);

		p1.xv -= Math.cos(deltaAng) * force;
		p1.yv -= Math.sin(deltaAng) * force;
	}
}
*/

void spawnItems(Game_State* game, Vector2 position, float accumulation) {
	pickUpAccumulator += ITEM_ACCUMULATE_RATE * accumulation;

	float roll = 5.0f + random() * 95.0f;

	if (roll < pickUpAccumulator) {
		float roll = random() * 100.0f;
		Entity* pickup;
		if (roll > 55) {
			pickup = spawn_entity(game, ENTITY_TYPE_ITEM_MISSILE, position);
		} else {
			pickup = spawn_entity(game, ENTITY_TYPE_ITEM_LASER, position);
		} 

		pickUpAccumulator = 0;
	}
}