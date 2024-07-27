#include "../../engine/assets.h"
#include "../../engine/math.h"
#include "../../engine/lerp.h"
#include "../game_types.h"
#include "../entities.h"

#define TURRET_RADIUS 15.0f
#define TURRET_ACCEL 0.06f
#define TURRET_SHOT_RADIUS 4.0f
#define TURRET_SHOT_SPEED 3.0f
#define TURRET_SHOT_LIFE 220.0f
#define TURRET_AIM_TOLERANCE 0.125f
#define TURRET_TURN_SPEED 1.2f
#define TURRET_FIRE_ANIM_SPEED 5.0f //83ms
#define TURRET_RECOVERY_ANIM_SPEED 60.0f //500ms

typedef enum Turret_State {
	TURRET_STATE_AIMING,
	TURRET_STATE_FIRING,
	TURRET_STATE_RECOVERING,
} Turret_State;

static inline void init_turret(Entity* entity) {
	entity->shape.radius = TURRET_RADIUS;
	entity->sprites[0].texture_name = "Enemy Turret Base";
	entity->sprites[1].texture_name = "Enemy Turret Cannon";
	entity->sprites[1].rotation_enabled = 1;
	entity->sprite_count = 2;
	entity->flags = ENTITY_FLAG_EXPLOSION_ENABLED;
}

static inline void update_turret(Game_State* game, Entity* entity, float dt) {
	Entity_System* es = game->entities;
	Particle_System* ps = game->particle_system;

	switch(entity->type_data) {
		case TURRET_STATE_AIMING: {
			Entity* target = get_enemy_target(game);
			if (target) {
				float aim_offset = angle_rotation_to_target(entity->position, target->position, entity->angle, TURRET_AIM_TOLERANCE);
				if (aim_offset == 0 && entity->timer.time <= 0) {
					Vector2 position = {
						entity->position.x + cos_deg(entity->angle) * TURRET_RADIUS,
						entity->position.y + sin_deg(entity->angle) * TURRET_RADIUS
					};
					
					Vector2 velocity = {
						cos_deg(entity->angle) * TURRET_SHOT_SPEED,
						sin_deg(entity->angle) * TURRET_SHOT_SPEED
					};
					
					float shot_offset_angle = normalize_degrees(entity->angle - 90.0f);
					for (int i = 0; i < 2; i++) {
						Uint32 new_shot_id = spawn_entity(es, ps, ENTITY_TYPE_BULLET, position);
						Entity* new_shot = get_entity(es, new_shot_id);
						if (new_shot == NULL) { break; }

						new_shot->x += cos_deg(shot_offset_angle) * TURRET_RADIUS;
						new_shot->y += sin_deg(shot_offset_angle) * TURRET_RADIUS;
						new_shot->velocity = velocity;
						new_shot->shape.type = SHAPE_TYPE_CIRCLE;
						new_shot->shape.radius = TURRET_SHOT_RADIUS;
						lerp_timer_start(&new_shot->timer, 0, TURRET_SHOT_LIFE, -1);

						shot_offset_angle = normalize_degrees(shot_offset_angle + 180.0f);
					}

					Mix_PlayChannel(-1, assets_get_sfx(game->assets, "Turret Fire"), 0);

					lerp_timer_start(&entity->timer, 0, TURRET_FIRE_ANIM_SPEED, -1);
					entity->type_data = TURRET_STATE_FIRING;
				} else {
					entity->angle += aim_offset * TURRET_TURN_SPEED * dt;
				}
			}
		} break;

		case TURRET_STATE_FIRING : {
			float t = lerp_timer_get_t(&entity->timer);
			entity->sprites[1].offset.x = 0.0f;
			entity->sprites[1].offset.x -= (float)TURRET_RADIUS - lerp(0, TURRET_RADIUS, t);
			if (entity->timer.time <= 0) {
				entity->type_data = TURRET_STATE_RECOVERING;
				lerp_timer_start(&entity->timer, 0, TURRET_RECOVERY_ANIM_SPEED, -1);
			}
		} break;

		case TURRET_STATE_RECOVERING: {
			float t = lerp_timer_get_t(&entity->timer);
			entity->sprites[1].offset.x = 0;
			entity->sprites[1].offset.x -= lerp(0, TURRET_RADIUS, t);
			if (entity->timer.time <= 0) {
				entity->sprites[1].offset.x = 0;
				entity->type_data = TURRET_STATE_AIMING;

				entity->timer = (Lerp_Timer){0};
			}
		} break;
	}
}

