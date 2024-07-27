#include "../../engine/math.h"
#include "../../engine/lerp.h"
#include "../game_types.h"
#include "../entities.h"

#define UFO_SPEED 1.9f
#define UFO_DIR_CHANGE_DELAY 120.0f
#define UFO_COLLISION_RADIUS 20.0f
#define UFO_TURN_PRECISION 0.05f

static inline void init_ufo(Entity* entity) {
	entity->shape.radius = UFO_COLLISION_RADIUS;
	entity->angle = entity->target_angle = randomf() * 360.0f;
	entity->sprites[0].texture_name = "Enemy UFO";
	entity->sprite_count = 1;
	entity->flags = ENTITY_FLAG_EXPLOSION_ENABLED;
}

static inline void update_ufo(Game_State* game, Entity* entity, float dt) {
	float angle_delta = 
		(cos_deg(entity->target_angle) * sin_deg(entity->angle)) - 
		(sin_deg(entity->target_angle) * cos_deg(entity->angle));

	if (SDL_fabs(angle_delta) > UFO_TURN_PRECISION) {
		entity->timer.dir = 0;
		entity->angle += dt * (float)(1 - ((int)(angle_delta < 0) * 2));

		entity->vx += cos_deg(entity->angle) * UFO_SPEED * 0.025;
		entity->vx += sin_deg(entity->angle) * UFO_SPEED * 0.025;
	} else {
		entity->timer.dir = -1;
	}

	if (entity->timer.time <= 0) {
		entity->target_angle = randomf() * 360.0f;
		lerp_timer_start(&entity->timer, 0, UFO_DIR_CHANGE_DELAY, -1);
	}

	float magnitude = SDL_sqrt( (entity->vx * entity->vx) + (entity->vy * entity->vy) );

	if (magnitude > UFO_SPEED) {
		entity->vx *= 1 - 0.028 * dt;
		entity->vy *= 1 - 0.028 * dt;
	} else if (magnitude < UFO_SPEED - 0.05) {
		entity->vx += cos_deg(entity->angle) * UFO_SPEED * 0.025;
		entity->vy += sin_deg(entity->angle) * UFO_SPEED * 0.025;
	}
}
