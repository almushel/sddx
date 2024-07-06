#include "../engine/math.h"
#include "../engine/assets.h"
#include "../engine/graphics.h"

#include "score.h"
#include "entities.h"

#define DEFAULT_ENTITY_RADIUS 20.0f
#define ENTITY_WARP_DELAY 26.0f
#define ENTITY_WARP_RADIUS 20
#define ENEMY_EXPLOSION_RADIUS 100.0f
#define ITEM_ACCUMULATE_RATE 1
#define PHYSICS_FRICTION 0.02f
#define WAVE_ESCALATION_RATE 4

#define MAX_ENTITIES 256
struct Entity_System {
	Entity entities[MAX_ENTITIES];
	Uint32 num_entities;

	Uint32 next;
};

static inline Entity* get_enemy_target(Game_State* game) {
	Entity* target = get_entity(game->entities, game->player);
	if (target != NULL && target->state == ENTITY_STATE_ACTIVE) {
		return target;	
	}

	return 0;
}

#include "entities/weapons.c"
#include "entities/player.c"
#include "entities/items.c"
#include "entities/drifter.c"
#include "entities/ufo.c"
#include "entities/tracker.c"
#include "entities/turret.c"
#include "entities/grappler.c"

Entity_System* create_entity_system() {
	Entity_System* result = calloc(1, sizeof(Entity_System));

	return result;
}

void reset_entity_system(Entity_System* es) {
	es->num_entities = 0;
	es->next = 0;
}

void despawn_entities(Entity_System* es) {
	for (Entity* e = es->entities; e != es->entities+es->num_entities; e++) {
		if (e->state > ENTITY_STATE_UNDEFINED && e->state < ENTITY_STATE_COUNT) {
			e->state = ENTITY_STATE_DESPAWNING;
			e->timer = 30.0f;
		}
	}
}

// Get valid, active entity or null pointer
Entity* get_entity(Entity_System* es, Uint32 entity_id) {
	Entity* result = 0;

	if (entity_id > 0 && entity_id <= MAX_ENTITIES) {
		result = es->entities + (entity_id-1);
	}

	return result;
}

Uint32 get_new_entity(Entity_System* es) {
	Uint32 result = 0;

	if (es->next) {
		result = es->next;
		es->next = 0;
		for (int i=result; i < es->num_entities; i++) {
			if (es->entities[i].state == ENTITY_STATE_UNDEFINED) {
				es->next = (i+1);
				break;
			}
		}
	} else if (es->num_entities < MAX_ENTITIES) {
		es->num_entities++;
		result = es->num_entities;
	}

	return result;
}

Uint32 remove_entity(Entity_System* es, Uint32 id) {
	Uint32 result = 0;

	if (id <= es->num_entities) {
		es->entities[id-1].state = ENTITY_STATE_UNDEFINED;
		es->entities[id-1].type = ENTITY_TYPE_UNDEFINED;
		if (!es->next || id < es->next) {
			es->next = id;
		}
	}

	return result;
}

void force_circle(Entity_System* es, float x, float y, float radius, float force) {
	float deltaX = 0;
	float deltaY = 0;
	float deltaAng = 0;

	Game_Shape force_shape = {
		.type = SHAPE_TYPE_CIRCLE,
		.radius = radius,
	};

	Transform2D force_transform = {
		.position = {x,y},
		.scale = {1,1},
	};

	Vector2 overlap;
	for (int i = 1; i <= es->num_entities; i++) {
		Entity* entity = get_entity(es, i);
		if (entity == NULL) { continue; }
		if (check_shape_collision(force_transform, force_shape, entity->transform, entity->shape, &overlap)) {
			Vector2 impulse = subtract_vector2(entity->position, (Vector2){x,y});
					impulse = normalize_vector2(impulse);
					impulse = scale_vector2(impulse, force);
				
			entity->velocity = add_vector2(entity->velocity, impulse);
		}
	}
}

static inline float get_entity_score_value(Entity_Types type) {
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

Vector2 get_clear_spawn(Entity_System* es, float radius, SDL_Rect boundary) {
	Vector2 result = {
		.x = (boundary.x + radius) + randomf() * (boundary.x + boundary.w - radius),
		.y = (boundary.y + radius) + randomf() * (boundary.y + boundary.h - radius),
	};

	Game_Shape spawn_area = {
		.type = SHAPE_TYPE_CIRCLE,
		.radius = radius,
	};

	Vector2 overlap = {0};
	Entity* entity;
	for (int i = 1; i <= es->num_entities; i++) {
		entity = get_entity(es, i);
		if (entity == NULL) continue;

		Game_Shape entity_shape = scale_game_shape(entity->shape, entity->scale);
		if (check_shape_collision((Transform2D){.position=result, .scale={1,1}}, spawn_area, entity->transform, entity_shape, &overlap)) {
			result.x -= overlap.x;
			result.y -= overlap.y;
		}
	}

	return result;
}

static inline bool entity_type_is_enemy(Entity_Types type) {
	return (
		type >= ENTITY_TYPE_ENEMY_DRIFTER && 
		type <= ENTITY_TYPE_ENEMY_GRAPPLER
	);
}

static inline bool entity_is_item(Entity_Types type) {
	return (
		type >= ENTITY_TYPE_ITEM_MISSILE &&
		type <= ENTITY_TYPE_ITEM_LASER
	);
}

static inline bool entity_type_explodes(Entity_Types type) {
	bool result = (
		type == ENTITY_TYPE_PLAYER || type == ENTITY_TYPE_MISSILE ||
		(type >= ENTITY_TYPE_ENEMY_DRIFTER && type <= ENTITY_TYPE_ENEMY_GRAPPLER)
	);

	return result;
}

Uint32 spawn_entity(Entity_System* es, Particle_System* ps, Entity_Types type, Vector2 position) {
	Uint32 result = 0;
	Entity* entity;

	if (type > ENTITY_TYPE_UNDEFINED && type < ENTITY_TYPE_COUNT) {
		result = get_new_entity(es);
		entity  = get_entity(es, result);
	}
	if (entity == NULL) { return 0; }

	*entity = (Entity) {
		.type = type,
		.position = position,
		.scale = { 1.0f, 1.0f },
		.shape.type = SHAPE_TYPE_CIRCLE,
		.shape.radius = DEFAULT_ENTITY_RADIUS,
		.timer = ENTITY_WARP_DELAY,
		.state = ENTITY_STATE_SPAWNING,
		.team = ENTITY_TEAM_ENEMY,
	};

	switch(type) {
		case ENTITY_TYPE_DEMOSHIP: {
			entity->timer = 0;
		}	
		case ENTITY_TYPE_PLAYER:	{ init_player(ps, entity); } break;
		case ENTITY_TYPE_BULLET:	{ init_bullet(entity); } break;
		case ENTITY_TYPE_MISSILE:	{ init_missile(ps, entity); } break;
		case ENTITY_TYPE_LASER:		{ init_laser(entity); } break;
		case ENTITY_TYPE_ENEMY_DRIFTER: { init_drifter(entity); } break;
		case ENTITY_TYPE_ENEMY_UFO:	{ init_ufo(entity); } break;
		case ENTITY_TYPE_ENEMY_TRACKER: { init_tracker(ps, entity); } break;
		case ENTITY_TYPE_ENEMY_TURRET:	{ init_turret(entity); } break;
		case ENTITY_TYPE_ENEMY_GRAPPLER:{ init_grappler(entity); } break;
		case ENTITY_TYPE_ITEM_MISSILE:	{ init_item_missile(entity); } break;
		case ENTITY_TYPE_ITEM_LIFEUP:	{ init_item_lifeup(entity); } break;
		case ENTITY_TYPE_ITEM_LASER:	{ init_item_laser(entity); } break;

		case ENTITY_TYPE_SPAWN_WARP: {
			entity->scale.x = entity->scale.y = 0;
			entity->shape.radius = ENTITY_WARP_RADIUS;
			entity->team = ENTITY_TEAM_UNDEFINED;
			entity->color = SD_BLUE;
		} break;

		default : {} break;
	}

	return result;
}

void random_item_spawn(Game_State* game, Vector2 position, float accumulation) {
	game->score.item_accumulator += ITEM_ACCUMULATE_RATE * accumulation;

	float roll = 5.0f + randomf() * 95.0f;

	if (roll < game->score.item_accumulator) {
		float roll = randomf() * 100.0f;
		if (roll > 55) {
			spawn_entity(game->entities, game->particle_system, ENTITY_TYPE_ITEM_MISSILE, position);
		} else {
			spawn_entity(game->entities, game->particle_system, ENTITY_TYPE_ITEM_LASER, position);
		} 

		game->score.item_accumulator = 0;
	}
}

void spawn_wave(Game_State* game, int wave, int points_max) {
	const SDL_Rect spawn_zones[] = {
		{0  , 0  , 390, 290}, 
		{410, 0  , 390, 290}, 
		{0  , 310, 390, 290},
		{410, 330, 390, 290}
	};
	int zone_index = (randomf() * (array_length(spawn_zones))) - 1;

	//Add new enemy types every 5 waves
	int type_value_max = ((float)wave / (float)WAVE_ESCALATION_RATE + 0.5f);
	type_value_max = SDL_clamp(type_value_max, 0, ENTITY_TYPE_ENEMY_GRAPPLER - ENTITY_TYPE_ENEMY_DRIFTER);
	
	for (float points_remaining = (float)points_max; points_remaining > 0;) {
		//Generate random type between 0 and current maximum point value
		
		Uint8 spawn_type = ENTITY_TYPE_ENEMY_DRIFTER + (int)(randomf() * (float)(type_value_max));// + 0.5f);
		float spawn_value = 1.0f + (float)(spawn_type - ENTITY_TYPE_ENEMY_DRIFTER);

		if (spawn_value && points_remaining >= spawn_value) {
			points_remaining -= spawn_value;
			float entity_radius = 25;
			Vector2 new_position = get_clear_spawn(game->entities, entity_radius, spawn_zones[zone_index]);
			
			Uint32 warp_id = spawn_entity(game->entities, game->particle_system, ENTITY_TYPE_SPAWN_WARP, new_position);
			if (warp_id){
				Entity* warp = get_entity(game->entities, warp_id);
				if (warp == NULL) { break; }

				warp->type_data = spawn_type;
				game->enemy_count++;
			}
		}
		
		zone_index = (zone_index + 1) % array_length(spawn_zones);
	}
}

void remove_dead_entity(Game_State* game, Uint32 entity_id) {
	Entity_System* es = game->entities;
	Particle_System* ps = game->particle_system;

	Entity* dead_entity = get_entity(es, entity_id);
	if (dead_entity == NULL) { return; }
	
	for (int i = 0; i < dead_entity->emitter_count; i++) {
		remove_particle_emitter(ps, dead_entity->particle_emitters[i]);
	}
	dead_entity->emitter_count = 0;

	if (dead_entity->state == ENTITY_STATE_DYING) { // Skip for despawning entities
		switch(dead_entity->type) {
			case ENTITY_TYPE_PLAYER:	{ destroy_player(game); } break;
			case ENTITY_TYPE_ENEMY_DRIFTER: { destroy_drifter(game, dead_entity); } break;
			case ENTITY_TYPE_ITEM_LIFEUP:	{ game->player_state.lives++; } break;
		}

		if (entity_type_is_enemy(dead_entity->type)) {
			float value = get_entity_score_value(dead_entity->type);
			
			add_score(game, value);
			random_item_spawn(game, dead_entity->position, value);
			game->enemy_count--;

			force_circle(game->entities, dead_entity->x, dead_entity->y, ENEMY_EXPLOSION_RADIUS, 1.5f);
			Mix_PlayChannel(-1, assets_get_sfx(game->assets, "Enemy Death"), 0);
		}

		if (entity_type_explodes(dead_entity->type)) {
			RGBA_Color entity_color = (dead_entity->color.r || dead_entity->color.g || dead_entity->color.b) ? dead_entity->color : (RGBA_Color){255, 255, 255, 255};
			RGBA_Color colors[] = {entity_color, {255, 255, 255, 255}};

			explode_at_point(ps, dead_entity->x, dead_entity->y, colors, array_length(colors), 0, dead_entity->shape.type);
			for (int sprite_index = 0; sprite_index < dead_entity->sprite_count; sprite_index++) {
				explode_sprite(game->assets, ps, dead_entity->sprites+sprite_index, dead_entity->x, dead_entity->y, dead_entity->angle, 6);
			}
		}
	}

	remove_entity(es, entity_id);
}

void update_entities(Game_State* game, float dt) {
	Entity_System* es = game->entities;
	Particle_System* ps = game->particle_system;
	Entity* entity = 0;
	for (int entity_index = 1; entity_index <= es->num_entities; entity_index++) {
		entity = get_entity(game->entities, entity_index);
		if (entity == NULL) { continue; }
		
		if (entity->state == ENTITY_STATE_SPAWNING) {
			if (entity->type == ENTITY_TYPE_PLAYER) {
				update_spawning_player(game, entity, dt);
			} else {
				entity->timer -= dt * (float)(int)(entity->timer > 0);

				entity->transform.scale.x = entity->transform.scale.y = 
					1.0f - SDL_clamp((entity->timer/ENTITY_WARP_DELAY), 0.0f, 1.0f);
						
				if (entity->timer <= 0) {
					switch(entity->type) {
						case ENTITY_TYPE_MISSILE: { entity->timer = MISSILE_LIFETIME; }	
						case ENTITY_TYPE_LASER: { entity->timer = PLAYER_SHOT_LIFE; }

						default: {
							entity->timer = 100.0f;
						} break;
					}
					entity->state = ENTITY_STATE_ACTIVE;
				}
			}
		// TO-DO: Movement update for despawning enemies
		} else if (entity->state == ENTITY_STATE_DESPAWNING) {
			entity->timer -= dt * (float)(int)(entity->timer > 0);
			
			entity->transform.scale.x = 
				entity->transform.scale.y = 
					SDL_clamp(entity->timer/ENTITY_WARP_DELAY, 0.0f, 1.0f);

			if (entity->timer <= 0) {
				remove_dead_entity(game, entity_index);
			}
		} else if (entity->state == ENTITY_STATE_DYING) {
			remove_dead_entity(game, entity_index);
		} else if (entity->state == ENTITY_STATE_ACTIVE) {
			entity->timer -= dt * (float)(int)(entity->timer > 0);
			switch(entity->type) {
				case ENTITY_TYPE_DEMOSHIP:	{ update_demo_ship(game, entity, dt); } break;
				case ENTITY_TYPE_PLAYER:	{ update_player_entity(game, entity, dt); } break;
				case ENTITY_TYPE_LASER:
				case ENTITY_TYPE_BULLET:	{ update_bullet(game, entity, dt); } break;
				case ENTITY_TYPE_MISSILE:	{ update_missile(game, entity, dt); } break;
				case ENTITY_TYPE_ENEMY_DRIFTER: { update_drifter(game, entity, dt); } break;
				case ENTITY_TYPE_ENEMY_UFO:	{ update_ufo(game, entity, dt); } break;
				case ENTITY_TYPE_ENEMY_TRACKER:	{ update_tracker(game, entity, dt); } break;
				case ENTITY_TYPE_ENEMY_TURRET:	{ update_turret(game, entity, dt); } break;
				case ENTITY_TYPE_ENEMY_GRAPPLER:{ update_grappler(game, entity, dt); } break;

				case ENTITY_TYPE_ITEM_LIFEUP:
				case ENTITY_TYPE_ITEM_MISSILE:
				case ENTITY_TYPE_ITEM_LASER:	{ entity->angle += dt; } break;

				case ENTITY_TYPE_SPAWN_WARP: {
					entity->timer = ENTITY_WARP_DELAY;
					entity->state = ENTITY_STATE_DESPAWNING;
					
					Entity* spawn = get_entity(es, spawn_entity(es, ps, entity->type_data, entity->position));
					if (spawn) {
						spawn->transform.scale = (Vector2){0}; // Prevent drawing full size this frame
					}
				} break;
			}

			entity->angle = normalize_degrees(entity->angle);
		
			entity->x += entity->vx * dt;
			entity->y += entity->vy * dt;

			entity->vx *= 1.0 - (PHYSICS_FRICTION*dt);
			entity->vy *= 1.0 - (PHYSICS_FRICTION*dt);

			entity->position = wrap_world_coords(entity->x, entity->y, 0, 0, game->world_w, game->world_h);
		
			// Entity-to-entity collision
			if (entity->type  != ENTITY_TYPE_SPAWN_WARP) {
				for (int collision_entity_index = entity_index+1; collision_entity_index <= es->num_entities; collision_entity_index++) {
					Entity* collision_entity = get_entity(es, collision_entity_index);
					if (collision_entity == NULL) continue;
					if (collision_entity->state != ENTITY_STATE_ACTIVE || collision_entity->type == ENTITY_TYPE_SPAWN_WARP) continue;
					
					Vector2 overlap = {0};
					Entity* item_entity = 	(entity_is_item(entity->type)) ? entity	 :
								(entity_is_item(collision_entity->type)) ? collision_entity :
								0;

					Entity* player_entity = (entity->type == ENTITY_TYPE_PLAYER) ? entity :
								(collision_entity->type == ENTITY_TYPE_PLAYER) ? collision_entity :
								0;
					
					if (check_shape_collision(entity->transform, entity->shape, collision_entity->transform, collision_entity->shape, &overlap)) {
						if (item_entity && player_entity) {
							item_entity->state = ENTITY_STATE_DESPAWNING;
							item_entity->timer = ENTITY_WARP_DELAY/2.0f;

							Mix_PlayChannel(-1, assets_get_sfx(game->assets, "Weapon Pickup"), 0);

						
							switch (item_entity->type) {
								case ENTITY_TYPE_ITEM_MISSILE: {
									game->player_state.ammo *= (int)(player_entity->type_data == PLAYER_WEAPON_MISSILE);
									player_entity->type_data = PLAYER_WEAPON_MISSILE;
									game->player_state.ammo += 10;	
								} break;
								
								case ENTITY_TYPE_ITEM_LASER: {
									game->player_state.ammo *= (int)(player_entity->type_data == PLAYER_WEAPON_LASER);
									player_entity->type_data = PLAYER_WEAPON_LASER;
									game->player_state.ammo += 10;
								} break;
							}
						} else if (entity->team && collision_entity->team && entity->team != collision_entity->team) {
							entity->state  = ENTITY_STATE_DYING;
							collision_entity->state = ENTITY_STATE_DYING;
						} else {
							overlap = normalize_vector2(overlap);

							entity 			->vx -= overlap.x/2.0f;
							entity 			->vy -= overlap.y/2.0f;
							collision_entity->vx += overlap.x/2.0f;
							collision_entity->vy += overlap.y/2.0f;
						}
					}
				}
			}

			displace_particles(ps, entity->transform, scale_game_shape(entity->shape, entity->scale));
		} // end of if (entity->state == ENTITY_STATE_ACTIVE)
	}
}

Rectangle get_entity_bounding_box(Game_Assets* assets, Entity* entity) {
	Rectangle result = {0};
	Game_Shape shape;

	if (entity->sprite_count > 0) {
		shape.type = SHAPE_TYPE_RECT;
		shape.rectangle = get_sprite_rect(assets, &entity->sprites[0]);
		shape.rectangle.x -= shape.rectangle.w/2.0f;
		shape.rectangle.y -= shape.rectangle.h/2.0f;

		shape = rotate_game_shape(shape, entity->angle * (float)(int)(entity->sprites[0].rotation_enabled));
	} else {
		shape = entity->shape;
		shape = rotate_game_shape(shape, entity->angle);
	}

	shape = scale_game_shape (shape, entity->scale);

	switch (shape.type) {
		case SHAPE_TYPE_CIRCLE: {
			result.x = result.y = -shape.radius;
			result.w = result.h =  shape.radius*2.0f;
		} break;

		case SHAPE_TYPE_RECT: {
			result = shape.rectangle;
		} break;
		
		case SHAPE_TYPE_POLY2D: {
			for (int i = 0; i < shape.polygon.vert_count; i++) {
				result.x = SDL_min(result.x, shape.polygon.vertices[i].x);
				result.y = SDL_min(result.y, shape.polygon.vertices[i].y);
				
				result.w = SDL_max(result.w, shape.polygon.vertices[i].x);
				result.h = SDL_max(result.h, shape.polygon.vertices[i].y);
			}
			result.w -= result.x;
			result.h -= result.y;
		} break;

		default: {} break;
	}

	return result;
}

void draw_entities(Entity_System* es, Game_Assets* assets, int world_w, int world_h) {
	Entity* entity = 0;
	Rectangle world_rect = {0,0,world_w,world_h};
	Vector2 wrap_positions[4] = {0};
	int wrap_count = 0;

	for (int entity_index = 1; entity_index <= es->num_entities; entity_index++) {
		entity = get_entity(es, entity_index);
		if (entity == NULL
		||  entity->state <= 0
		||  entity->state >= ENTITY_STATE_DYING
		||  entity->transform.scale.x+entity->transform.scale.y == 0
		) { continue; }

		if (entity->type == ENTITY_TYPE_ENEMY_GRAPPLER) {
			draw_grappler(entity);
		}

		Transform2D transform = entity->transform;
		Rectangle bounding_box = get_entity_bounding_box(assets, entity);
		wrap_aab(entity->transform.position, bounding_box, world_rect, wrap_positions, &wrap_count);

		for (int i = 0; i < wrap_count; i++) {
			transform.position = wrap_positions[i];

			if (entity->sprite_count > 0) {
				for (int sprite_index = 0; sprite_index < entity->sprite_count; sprite_index++) {
					if (entity->sprites[sprite_index].texture_name != 0) {
						render_draw_game_sprite(assets, &entity->sprites[sprite_index], transform, true);
					}
				}
			}
			else if (entity->shape.type > SHAPE_TYPE_UNDEFINED && entity->shape.type < SHAPE_TYPE_COUNT) {
				Game_Shape  shape = scale_game_shape(entity->shape, transform.scale);
							shape = rotate_game_shape(shape, transform.angle);
				render_fill_game_shape(transform.position, shape, entity->color);
			}
#ifdef DEBUG
			Rectangle test = bounding_box;
			test.x += transform.x;
			test.y += transform.y;
			
			platform_set_render_draw_color((RGBA_Color){255, 255, 0, 255});
			platform_render_draw_rect(test);
#endif
		}
#ifdef DEBUG
		Game_Shape  shape = scale_game_shape(entity->shape, entity->scale);
					shape = rotate_game_shape(shape, entity->angle);

		platform_set_render_draw_color((RGBA_Color){255, 0, 0, 255});
		render_draw_game_shape(entity->position, shape, (RGBA_Color){255, 0, 0, 255});
#endif
	}
}
