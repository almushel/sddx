#include "../../engine/assets.h"
#include "../../engine/math.h"
#include "../../engine/platform.h"
#include "../entities.h"
#include "../game_types.h"

#define GRAPPLER_COLOR (RGBA_Color){0,255,0,255}
#define GRAPPLER_RADIUS 15.0f
#define GRAPPLER_AIM_TOLERANCE 0.09f
#define GRAPPLER_TURN_SPEED 1.3f
#define GRAPPLER_SPACE_FRICTION 0.06f
#define GRAPPLER_ACCEL 0.06f
#define GRAPPLER_HOOK_SPEED 6.0f
#define GRAPPLER_REELING_ACCELERATION 0.08f

#define GRAPPLER_HOOK_RADIUS 15.0f

typedef enum Grappler_State {
	GRAPPLER_STATE_AIMING,
	GRAPPLER_STATE_EXTENDING,
	GRAPPLER_STATE_RETRACTING,
	GRAPPLER_STATE_REELING,
} Grappler_State;

static inline void init_grappler(Entity* entity) {
	entity->shape.radius = GRAPPLER_RADIUS;
	entity->sprites[0].texture_name = "Enemy Grappler";
	entity->sprites[0].rotation_enabled = 1;
	entity->sprites[1].texture_name = "Grappler Hook";
	entity->sprites[1].rotation_enabled = 1;
	entity->sprite_count = 2;
	entity->flags = ENTITY_FLAG_EXPLOSION_ENABLED;
}

static inline void update_grappler(Game_State* game, Entity* entity, float dt) {
	Entity* target = get_enemy_target(game);

	switch(entity->type_data) {
		case GRAPPLER_STATE_AIMING: {
			if (target) {
				// TODO: Currently fires when target is directly behind as well
				float aim_delta = angle_rotation_to_target(entity->position, target->position, entity->angle, GRAPPLER_AIM_TOLERANCE);
				if (aim_delta == 0) {
					entity->type_data = GRAPPLER_STATE_EXTENDING;
					Mix_PlayChannel(-1, assets_get_sfx(game->assets, "Grappler Fire"), 0);
				} else {
					entity->angle += aim_delta * GRAPPLER_TURN_SPEED * dt;
				}
			}
		} break;

		case GRAPPLER_STATE_EXTENDING: {
			entity->sprites[1].offset.x += GRAPPLER_HOOK_SPEED * dt;

			Vector2 hook_position = rotate_vector2(entity->sprites[1].offset, entity->angle);
			hook_position.x += entity->position.x;
			hook_position.y += entity->position.y;

			Vector2 overlap = {0};
			if (	target && 
				sc2d_check_circles(
					hook_position.x, hook_position.y, (float)GRAPPLER_HOOK_RADIUS,
					target->x, target->y, target->shape.radius,
					&overlap.x, &overlap.y
				)
			) {
				entity->type_data = GRAPPLER_STATE_REELING;
				Mix_PlayChannel(-1, assets_get_sfx(game->assets, "Hook Impact"), 0);

			} else if (!sc2d_check_point_rect(
					hook_position.x, hook_position.y,
					0, 0, game->world_w, game->world_h,
					&overlap.x, &overlap.y
				)
			) {
				entity->type_data = GRAPPLER_STATE_RETRACTING;
			}
		} break;

		case GRAPPLER_STATE_RETRACTING: {
			entity->sprites[1].offset.x -= GRAPPLER_HOOK_SPEED * dt;
			if (entity->sprites[1].offset.x <= 0) {
				entity->sprites[1].offset.x = 0;
				entity->type_data = GRAPPLER_STATE_AIMING;
			}
		} break;

		case GRAPPLER_STATE_REELING: {
			if (target) {
				Vector2 delta = {
					target->x - entity->x,
					target->y - entity->y
				};
				
				float magnitude = SDL_sqrtf( (delta.x * delta.x) + (delta.y * delta.y));
				delta = scale_vector2(delta, 1.0f/magnitude);
				
				float dot = dot_product_vector2(
					(Vector2){cos_deg(entity->angle), sin_deg(entity->angle)},
					delta
				);

				// Check for player screen wrap
				if (dot > 0 && dot > 0.9f) {
					float angle_to_target = normalize_degrees( atan2_deg(delta.y, delta.x) );
					entity->angle = angle_to_target;

					target->vx -= delta.x * GRAPPLER_REELING_ACCELERATION * dt;
					target->vy -= delta.y * GRAPPLER_REELING_ACCELERATION * dt;

					entity->sprites[1].offset.x = magnitude;
				} else {
					entity->type_data = GRAPPLER_STATE_RETRACTING;
				}
			} else {
				entity->type_data = GRAPPLER_STATE_RETRACTING;

			}
		} break;
	}
}

static inline void draw_grappler(Entity* entity) {
	Vector2 offset = {
		.x = cos_deg(entity->angle+90.0f)*1.5f,
		.y = sin_deg(entity->angle+90.0f)*1.5f
	};
	Vector2 end = add_vector2(
		entity->position,
		rotate_vector2(
			(Vector2){entity->sprites[1].offset.x-3, 0},
			entity->angle
		)
	);

	platform_set_render_draw_color(GRAPPLER_COLOR);
	Vector2 line[2] = {add_vector2(entity->position, offset), add_vector2(end, offset)};
	platform_render_draw_lines(line, 2);
	line[0] = subtract_vector2(entity->position, offset);
	line[1] = subtract_vector2(end, offset);
	platform_render_draw_lines(line, 2);
}
