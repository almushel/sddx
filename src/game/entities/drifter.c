#include "../../engine/assets.h"
#include "../../engine/math.h"
#include "../game_types.h"
#include "../entities.h"

#define DRIFTER_SPEED 1.0f
#define DRIFTER_RADIUS 40.0f
#define DRIFTER_GREY (RGBA_Color){105, 105, 105, 255}

static void generate_drifter_verts(Game_Shape* shape, float radius) {
	int vert_count = SDL_clamp(5 + (int)(randomf() * 3.0f), 4, MAX_POLY2D_VERTS);
	shape->polygon = generate_poly2D(vert_count, radius / 2.0f, radius);
	shape->type = SHAPE_TYPE_POLY2D;
}

static inline void init_drifter(Entity* entity) {
	generate_drifter_verts(&entity->shape, DRIFTER_RADIUS);
	entity->color = DRIFTER_GREY;
	entity->angle = randomf() * 360.0f;
	entity->velocity.x = cos_deg(entity->angle) * DRIFTER_SPEED;
	entity->velocity.y = sin_deg(entity->angle) * DRIFTER_SPEED;
}

static inline void update_drifter(Game_State* game, Entity* entity, float dt) {
	entity->velocity = scale_vector2(
		normalize_vector2(entity->velocity),
		DRIFTER_SPEED
	);
}

static inline void destroy_drifter(Game_State* game, Entity* entity) {
	Entity_System* es = game->entities;
	Particle_System* ps = game->particle_system;

	float v_max = 0;
	for (int i = 0; i < entity->shape.polygon.vert_count; i++) {
		float v_dist = hypotf(
			entity->shape.polygon.vertices[i].y,
			entity->shape.polygon.vertices[i].x
		);
		v_max = SDL_max(v_dist, v_max);
	}

	if (v_max > DRIFTER_RADIUS/2) {
		float angle = randomf() * 360.0f;
		for (int i = 0; i < 3; i++) {
			Vector2 position = entity->position;
			position.x += cos_deg(angle) * (float)DRIFTER_RADIUS/2.0f;
			position.y += sin_deg(angle) * (float)DRIFTER_RADIUS/2.0f;

			Entity* drifter_child = 
				get_entity(
					es, 
					spawn_entity(es, ps, ENTITY_TYPE_ENEMY_DRIFTER, position)
				);
			if (drifter_child == NULL) { continue; }

			generate_drifter_verts(&drifter_child->shape, (float)DRIFTER_RADIUS/2.0f);
			drifter_child->angle = angle;
			drifter_child->vx = cos_deg(angle) * (float)DRIFTER_SPEED;
			drifter_child->vy = sin_deg(angle) * (float)DRIFTER_SPEED;
			drifter_child->state = ENTITY_STATE_ACTIVE;

			angle += 360.0f / 3.0f;
		}

		game->enemy_count += 3;
	}
}
