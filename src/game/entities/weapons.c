#include "../game_types.h"
#include "../entities.h"
#include "../../engine/math.h"
#include "../../engine/lerp.h"
#include "../../engine/particles.h"

#define PLAYER_SHOT_RADIUS 3.0f
#define PLAYER_SHOT_SPEED 7.0f

#define BULLET_LIFETIME 26.0f

#define MISSILE_LIFETIME 120.0f
#define MISSILE_ACCEL 0.2f
#define MISSILE_TURN_RATE 3.0f

static inline void init_bullet(Entity* entity) {
	entity->shape.radius = PLAYER_SHOT_RADIUS;
	entity->state = ENTITY_STATE_ACTIVE;
	entity->color = (RGBA_Color){255, 150, 50, 255};
	entity->flags = 
		ENTITY_FLAG_FRICTION_DISABLED
		| ENTITY_FLAG_COLLISION_TRIGGER;
}

static inline void init_laser(Entity* entity) {
	entity->shape.type = SHAPE_TYPE_POLY2D;
	Rectangle rect = {.x = -15, .w = 30, .y = -5, .h = 10,};
	entity->shape.polygon = rect_to_poly2D(rect);
	entity->color = SD_BLUE;
	entity->flags = 
		ENTITY_FLAG_FRICTION_DISABLED
		| ENTITY_FLAG_COLLISION_TRIGGER;
}

static inline void init_missile(Particle_System* ps, Entity* entity) {
	entity->sprites[0] = (Game_Sprite){
		.rotation_enabled = 1,
		.texture_name = "Projectile Missile",
	};
	entity->sprite_count = 1;
	entity->shape.type = SHAPE_TYPE_POLY2D;
	entity->shape.polygon = 
		rect_to_poly2D(
			(Rectangle){.x = -15, .y = -5, .w = 30, .h = 10,}
		);
	entity->color = SD_BLUE;

	entity->particle_emitters[0] = get_new_particle_emitter(ps);
	entity->emitter_count = 1;
	Particle_Emitter* thruster = get_particle_emitter(ps, entity->particle_emitters[0]);
	if (thruster) {
		thruster->shape= SHAPE_TYPE_RECT,
		thruster->speed = 1.0f;
		thruster->colors[0] = WHITE;
		thruster->color_count = 1;
		thruster->density = 1.0f;
	}

	entity->flags =
		ENTITY_FLAG_EXPLOSION_ENABLED
		| ENTITY_FLAG_COLLISION_TRIGGER;
}

static inline void update_bullet(Game_State* game, Entity* entity, float dt) {
	if (entity->timer.time <= 0) {
		entity->state = ENTITY_STATE_DESPAWNING;
		lerp_timer_start(&entity->timer, 0, BULLET_LIFETIME, -1);
	}
}

static inline void update_missile(Game_State* game, Entity* entity, float dt) {
	if (entity->timer.time <= 0) { 
		entity->state = ENTITY_STATE_DYING;
		return;
	}
	
	Particle_Emitter* thruster = get_particle_emitter(game->particle_system, entity->particle_emitters[0]);
	thruster->state = 1;
	thruster->position = entity->position;
	thruster->angle = entity->angle + 180.0f;

	Vector2 missile_direction = {
		cos_deg(entity->angle), sin_deg(entity->angle)
	};

	Entity* target = 0;
	{ // Acquire target
		int32_t nearest = 1.0;
		Entity* potential_target = 0;
		for (int i = 1; i <= game->entities->num_entities; i++) {
			potential_target = get_entity(game->entities, i);
			if (	potential_target == NULL ||
				potential_target == entity ||
				potential_target->team == ENTITY_TEAM_UNDEFINED || 
				potential_target->team == entity->team
			) {
				continue;
			}

			Vector2 delta = {
				potential_target->x - entity->x,
				potential_target->y - entity->y,
			};
			delta = normalize_vector2(delta);
			
			float dot = dot_product_vector2(missile_direction, delta);

			// Ignore targets behind the missile
			if (dot > 0.0f) {
				if (dot < nearest) {
					nearest = dot;
					target = potential_target;
				}
			}
		}
	}

	Vector2 delta = {0};

	if (target) {
		delta.x = (target->x + target->vx * dt) - entity->x;
		delta.y = (target->y + target->vy * dt) - entity->y;
		delta = normalize_vector2(delta);

		float aim_offset = angle_rotation_to_target(entity->position, target->position, entity->angle, 0.0);
		entity->angle += aim_offset * MISSILE_TURN_RATE * dt;
	}

	Vector2 acceleration = {
		cos_deg(entity->angle) + delta.x,
		sin_deg(entity->angle) + delta.y
	};
	acceleration = scale_vector2(normalize_vector2(acceleration), MISSILE_ACCEL * dt);
	entity->velocity = add_vector2(entity->velocity, acceleration);

}
