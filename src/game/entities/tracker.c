#include "../../engine/particles.h"
#include "../../engine/math.h"
#include "../entities.h"
#include "../game_types.h"

#define TRACKER_ACCEL 0.13f
#define TRACKER_FRICTION 0.02f
#define TRACKER_TURN_RATE 1.5f
#define TRACKER_PRECISION 0.1f
#define TRACKER_COLLISION_RADIUS 14.0f

static inline void init_tracker(Particle_System* ps, Entity* entity) {
	entity->shape.radius = TRACKER_COLLISION_RADIUS;
	entity->sprites[0].texture_name = "Enemy Tracker";
	entity->sprites[0].rotation_enabled = 1;
	entity->sprite_count = 1;

	entity->particle_emitters[0] = get_new_particle_emitter(ps);
	entity->emitter_count = 1;
	Particle_Emitter* thruster = get_particle_emitter(ps, entity->particle_emitters[0]);
	if (thruster) {
		thruster->shape= SHAPE_TYPE_RECT,
		thruster->speed = 6.0f/2.0f;
		thruster->density = 1.0f;
		thruster->colors[0] = RED;
		thruster->color_count = 1;
	}
	entity->flags = ENTITY_FLAG_EXPLOSION_ENABLED;
}

static inline void update_tracker(Game_State* game, Entity* entity, float dt) {
	Particle_Emitter* thruster = get_particle_emitter(game->particle_system, entity->particle_emitters[0]);
	if (thruster) { thruster->state = 0; }
	Entity* target = get_enemy_target(game);
	if (target) {
		float acceleration_speed = TRACKER_ACCEL/2.0f;
		float aim_offset = angle_rotation_to_target(entity->position, target->position, entity->angle, TRACKER_PRECISION);

		if (aim_offset == 0) {
			acceleration_speed *= 2;
		
			if (thruster) {
				thruster->state = 1;
				thruster->angle = entity->angle + 180;
				thruster->x = entity->x + cos_deg(thruster->angle);// * (entity->shape.radius + PARTICLE_MAX_START_RADIUS) / 2.0f;
				thruster->y = entity->y + sin_deg(thruster->angle);// * (entity->shape.radius + PARTICLE_MAX_START_RADIUS) / 2.0f;
			}
		} else {
			entity->angle += TRACKER_TURN_RATE * aim_offset * dt;
		}

		entity->vx += cos_deg(entity->angle) * acceleration_speed * dt;
		entity->vy += sin_deg(entity->angle) * acceleration_speed * dt;
	}

}
