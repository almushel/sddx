
#include "defs.h"
#include "game_math.h"
#include "assets.h"
#include "graphics.h"
#include "score.h"
#include "entities.h"

#define ENTITY_WARP_DELAY 26.0f
#define ENTITY_WARP_RADIUS 20
#define DEAD_ENTITY_MAX 16 // Maximum number of dead entities garbage collected per frame

#define PHYSICS_FRICTION 0.02f

#define DEFAULT_ENTITY_RADIUS 20.0f

#define PLAYER_FORWARD_THRUST 0.15f
#define PLAYER_LATERAL_THRUST 0.2f
#define PLAYER_TURN_SPEED 3.14f

#define THRUST_POWER 0.15f
#define LATERAL_THRUST 0.2f
#define TURN_RATE 0.025f
#define PLAYER_SHOT_MAX 8
#define PLAYER_SHOT_RADIUS 3.0f
#define PLAYER_SHOT_SPEED 7.0f
#define PLAYER_SHOT_LIFE 80.0f
#define HEAT_MAX 100
#define THRUST_MAX 100
#define THRUST_CONSUMPTION 0.45f
#define SHIP_RADIUS 13
#define PLAYER_STARTING_LIVES 3

#define PLAYER_MG_COOLDOWN 200.0f/16.666f
#define PLAYER_LASER_COOLDOWN 250.0f/16.666f
#define PLAYER_MISSILE_COOLDOWN 800.0f/16.666f
#define PLAYER_MG_HEAT 25.0f
#define PLAYER_LASER_HEAT 35.0f
#define PLAYER_MISSILE_HEAT 45.0f

#define MISSILE_LIFETIME 120.0f
#define MISSILE_ACCEL 0.2f
#define MISSILE_TURN_RATE 3.0f

#define ENEMY_EXPLOSION_RADIUS 100.0f

#define DRIFTER_SPEED 1
#define DRIFTER_RADIUS 40
#define DRIFTER_GREY (RGBA_Color){105, 105, 105, 255}

#define UFO_SPEED 1.9
#define UFO_DIR_CHANGE_DELAY 120.0f
#define UFO_COLLISION_RADIUS 20.0f
#define UFO_TURN_PRECISION 0.05

#define TRACKER_ACCEL 0.13f
#define TRACKER_FRICTION 0.02f
#define TRACKER_TURN_RATE 1.5f
#define TRACKER_PRECISION 0.1f
#define TRACKER_COLLISION_RADIUS 14

#define TURRET_RADIUS 15
#define TURRET_ACCEL 0.06f
#define TURRET_SHOT_MAX 1
#define TURRET_SHOT_RADIUS 4
#define TURRET_SHOT_SPEED 3
#define TURRET_SHOT_LIFE 220
#define TURRET_AIM_TOLERANCE 0.125f
#define TURRET_TURN_SPEED 1.2
#define TURRET_FIRE_ANIM_SPEED 5.0f //83ms
#define TURRET_RECOVERY_ANIM_SPEED 60.0f //500ms

#define GRAPPLER_AIM_TOLERANCE 0.1f
#define GRAPPLER_TURN_SPEED 1.3
#define GRAPPLER_SPACE_FRICTION 0.06
#define GRAPPLER_ACCEL 0.06
#define GRAPPLER_HOOK_SPEED 6.0f
#define GRAPPLER_REELING_ACCELERATION 0.08f

#define ITEM_RADIUS 15

#define WAVE_ESCALATION_RATE 4
#define ITEM_ACCUMULATE_RATE 1

typedef enum Entity_States {
	ENTITY_STATE_UNDEFINED,
	ENTITY_STATE_SPAWNING,
	ENTITY_STATE_ACTIVE,
	ENTITY_STATE_DESPAWNING,
	ENTITY_STATE_DYING,
	ENTITY_STATE_COUNT,
} Entity_States;

typedef enum Entity_Teams {
	ENTITY_TEAM_UNDEFINED = 0,
	ENTITY_TEAM_PLAYER,
	ENTITY_TEAM_ENEMY,
} Entity_Teams;

typedef enum Turret_State {
	TURRET_STATE_AIMING,
	TURRET_STATE_FIRING,
	TURRET_STATE_RECOVERING,
} Turret_State;

typedef enum Grappler_State {
	GRAPPLER_STATE_AIMING,
	GRAPPLER_STATE_EXTENDING,
	GRAPPLER_STATE_RETRACTING,
	GRAPPLER_STATE_REELING,
} Grappler_State;

void force_circle(Entity* entities, Uint32 enitty_count, float x, float y, float radius, float force) {
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
	for (int i = 1; i < enitty_count; i++) {
		Entity* entity = entities + i;
		if (/*entities[i].mass > 0 && */check_shape_collision(force_transform, force_shape, entity->transform, entity->shape, &overlap)) {
			Vector2 impulse = subtract_vector2(entity->position, (Vector2){x,y});
					impulse = normalize_vector2(impulse);
					impulse = scale_vector2(impulse, force);
				
			entity->velocity = add_vector2(entity->velocity, impulse);
		}
	}
}

Uint32 get_new_entity(Game_State* game) {
	Uint32 result = 0;

	if (game->entity_count < game->entities_size) {
		result = game->entity_count++;
	} else {
		void* new_entities = SDL_realloc(game->entities, sizeof(Entity) * game->entities_size * 2);
		if (new_entities)  {
			game->entities = new_entities;
			game->entities_size *= 2;
			result = game->entity_count++;
		} else {
			SDL_Log("get_new_entity(): Failed to realloc game->entities.");
		}
	}
	return result;
}

Entity* get_entity(Game_State* game, Uint32 entity_id) {
	Entity* result = 0;

	if (entity_id && entity_id < game->entity_count) {
		result = game->entities + entity_id;
	}

	return result;
}

static void generate_drifter_verts(Game_Shape* shape, float radius) {
	int vert_count = SDL_clamp(5 + (int)(random() * 3.0f), 4, MAX_POLY2D_VERTS);
	shape->polygon = generate_poly2D(vert_count, radius / 2.0f, radius);
	shape->type = SHAPE_TYPE_POLY2D;
}

SDL_Texture* generate_item_texture(Game_State* game, SDL_Texture* icon) {
	SDL_Texture* result = 0;
	
	int result_size = ITEM_RADIUS*2 + 4;

	result = platform_create_texture(SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, result_size, result_size);
	if (result) {
		SDL_SetTextureBlendMode(result, SDL_BLENDMODE_BLEND);
		
		platform_set_render_target(result);

		platform_set_render_draw_color((RGBA_Color){0});
		platform_render_clear();
		render_fill_circlef_linear_gradient((float)result_size/2.0f, (float)result_size/2.0f, ITEM_RADIUS, CLEAR_COLOR, SD_BLUE);
		
	if (icon) {
		Vector2 dim = platform_get_texture_dimensions(icon);
		float larger_dim = (dim.x > dim.y) ? dim.x : dim.y;
		float ratio = ((float)ITEM_RADIUS * 1.7f)/larger_dim;
		
		Rectangle dest = {0};
		dest.w = dim.x * ratio;
		dest.h = dim.y * ratio;
		dest.x = ((float)result_size-dest.w) / 2.0f;
		dest.y = ((float)result_size-dest.h) / 2.0f;
		
		platform_render_copy(icon, NULL, &dest, 0, 0, SDL_FLIP_NONE);
	}
		platform_set_render_target(0);
	}

	return result;
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

Vector2 get_clear_spawn(Game_State* game, float radius, SDL_Rect boundary) {
	Vector2 result = {
		.x = (boundary.x + radius) + random() * (boundary.x + boundary.w - radius),
		.y = (boundary.y + radius) + random() * (boundary.y + boundary.h - radius),
	};

	Game_Shape spawn_area = {
		.type = SHAPE_TYPE_CIRCLE,
		.radius = radius,
	};

	Vector2 overlap = {0};
	Entity* entities = game->entities;
	for (int i = 1; i < game->entity_count; i++) {
		Game_Shape entity_shape = scale_game_shape(entities[i].shape, entities[i].scale);
		if (check_shape_collision((Transform2D){.position=result, .scale={1,1}}, spawn_area, entities[i].transform, entity_shape, &overlap)) {
			result.x -= overlap.x;
			result.y -= overlap.y;
		}
	}

	return result;
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

static inline Entity* get_enemy_target(Game_State* game) {
	Entity* result = 0;
	if (game->player && game->player->state == ENTITY_STATE_ACTIVE)
		result = game->player;

	return result;
}

static inline float get_aim_offset(Vector2 origin, Vector2 target, float aim_angle, float tolerance) {
	float result = 0;
	
	float target_angle = normalize_degrees( atan2_deg(target.y - origin.y, target.x - origin.x) );
	float angle_delta = cos_deg(target_angle)*sin_deg(aim_angle) - sin_deg(target_angle)*cos_deg(aim_angle);

	if (angle_delta < -tolerance) result = 1;
	else if (angle_delta > tolerance) result = -1;
	
	return result;
}

Uint32 spawn_entity(Game_State* game, Entity_Types type, Vector2 position) {
	Uint32 result = 0;
	if (type > ENTITY_TYPE_UNDEFINED && type < ENTITY_TYPE_COUNT) {
		result = get_new_entity(game);
	}

	if (result) {
		Entity* entity = game->entities + result;
		*entity = (Entity){0};
		entity->type = type;
		entity->position = position;
		entity->scale.x = entity->scale.y = 1.0f;
		entity->shape.type = SHAPE_TYPE_CIRCLE;
		entity->shape.radius = DEFAULT_ENTITY_RADIUS;
		entity->timer = ENTITY_WARP_DELAY;
		entity->state = ENTITY_STATE_SPAWNING;
		entity->team = ENTITY_TEAM_ENEMY;

		switch(type) {
			case ENTITY_TYPE_PLAYER: {
				entity->team = ENTITY_TEAM_PLAYER;
				entity->shape.radius = SHIP_RADIUS;
				entity->angle = 270;
				entity->sprites[0].texture_name = "Player Ship";
				entity->sprites[0].rotation_enabled = 1;
				entity->sprite_count = 1;
				entity->emitter_count = 3;

				for (int i = 0; i < entity->emitter_count; i++) {
					entity->particle_emitters[i] = get_new_particle_emitter(&game->particle_system);
					*get_particle_emitter(&game->particle_system, entity->particle_emitters[i]) =
						(Particle_Emitter) {
							.shape = SHAPE_TYPE_RECT,
							.density = 2.0f,
							.colors[0] = SD_BLUE,
							.color_count = 1,
						};
				}
			} break;

			case ENTITY_TYPE_BULLET: {
				entity->shape.radius = PLAYER_SHOT_RADIUS;
				entity->state = ENTITY_STATE_ACTIVE;
				entity->color = (RGBA_Color){255, 150, 50, 255};
			} break;

			case ENTITY_TYPE_MISSILE: {
				entity->sprites[0] = (Game_Sprite){
					.rotation_enabled = 1,
					.texture_name = "Projectile Missile",
				};
				entity->sprite_count = 1;
				entity->shape.type = SHAPE_TYPE_POLY2D;
				Rectangle rect = {.x = -15, .y = -5, .w = 30, .h = 10,};
				entity->shape.polygon = rect_to_poly2D(rect);
				entity->color = SD_BLUE;

				entity->particle_emitters[0] = get_new_particle_emitter(&game->particle_system);
				entity->emitter_count = 1;
				Particle_Emitter* thruster = get_particle_emitter(&game->particle_system, entity->particle_emitters[0]);
				thruster->shape= SHAPE_TYPE_RECT,
				thruster->density = 1.0f;

			} break;
			
			case ENTITY_TYPE_LASER: {
				entity->shape.type = SHAPE_TYPE_POLY2D;
				Rectangle rect = {.x = -15, .w = 30, .y = -5, .h = 10,};
				entity->shape.polygon = rect_to_poly2D(rect);
				entity->color = SD_BLUE;
			} break;

			case ENTITY_TYPE_ENEMY_DRIFTER: {
				generate_drifter_verts(&entity->shape, DRIFTER_RADIUS);
				entity->color = DRIFTER_GREY;
				entity->angle = random() * 360.0f;
				entity->velocity.x = cos_deg(entity->angle) * DRIFTER_SPEED;
				entity->velocity.y = sin_deg(entity->angle) * DRIFTER_SPEED;
			} break;

			case ENTITY_TYPE_ENEMY_UFO: {
				entity->shape.radius = UFO_COLLISION_RADIUS;
				entity->angle = entity->target_angle = random() * 360.0f;
				entity->sprites[0].texture_name = "Enemy UFO";
				entity->sprite_count = 1;
			} break;
			
			case ENTITY_TYPE_ENEMY_TRACKER: {
				entity->shape.radius = TRACKER_COLLISION_RADIUS;
				entity->sprites[0].texture_name = "Enemy Tracker";
				entity->sprites[0].rotation_enabled = 1;
				entity->sprite_count = 1;

				entity->particle_emitters[0] = get_new_particle_emitter(&game->particle_system);
				entity->emitter_count = 1;
				Particle_Emitter* thruster = get_particle_emitter(&game->particle_system, entity->particle_emitters[0]);
				thruster->shape= SHAPE_TYPE_RECT,
				thruster->angle = 180;
				thruster->density = 1.5f;
				thruster->colors[0] = (RGBA_Color){255, 0, 0};
				thruster->color_count = 1;
			} break;

			case ENTITY_TYPE_ENEMY_TURRET: {
				entity->shape.radius = TURRET_RADIUS;
				entity->sprites[0].texture_name = "Enemy Turret Base";
				entity->sprites[1].texture_name = "Enemy Turret Cannon";
				entity->sprites[1].rotation_enabled = 1;
				entity->sprite_count = 2;
			} break;
			
			case ENTITY_TYPE_ENEMY_GRAPPLER: {
				entity->shape.radius = TURRET_RADIUS;
				entity->sprites[0].texture_name = "Enemy Grappler";
				entity->sprites[0].rotation_enabled = 1;
				entity->sprites[1].texture_name = "Grappler Hook";
				entity->sprites[1].rotation_enabled = 1;
				entity->sprite_count = 2;
			} break;

			case ENTITY_TYPE_ITEM_MISSILE: {
				entity->team = ENTITY_TEAM_UNDEFINED;
				entity->shape.radius = ITEM_RADIUS;
				entity->sprites[0].texture_name = "Item Missile";
				entity->sprites[0].rotation_enabled = 1;
				entity->sprite_count = 1;
			} break;

			case ENTITY_TYPE_ITEM_LIFEUP: {
				entity->team = ENTITY_TEAM_UNDEFINED;
				entity->shape.radius = ITEM_RADIUS;
				entity->sprites[0].texture_name = "Item LifeUp";
				entity->sprites[0].rotation_enabled = 1;
				entity->sprite_count = 1;
			} break;

			case ENTITY_TYPE_ITEM_LASER: {
				entity->team = ENTITY_TEAM_UNDEFINED;
				entity->shape.radius = ITEM_RADIUS;
				entity->sprites[0].texture_name = "Item Laser";
				entity->sprites[0].rotation_enabled = 1;
				entity->sprite_count = 1;
			}

			case ENTITY_TYPE_SPAWN_WARP: {
				entity->shape.radius = ENTITY_WARP_RADIUS;
				entity->team = ENTITY_TEAM_UNDEFINED;
				entity->color = SD_BLUE;
			} break;
		}
	}

	return result;
}

void random_item_spawn(Game_State* game, Vector2 position, float accumulation) {
	game->score.item_accumulator += ITEM_ACCUMULATE_RATE * accumulation;

	float roll = 5.0f + random() * 95.0f;

	if (roll < game->score.item_accumulator) {
		float roll = random() * 100.0f;
		if (roll > 55) {
			spawn_entity(game, ENTITY_TYPE_ITEM_MISSILE, position);
		} else {
			spawn_entity(game, ENTITY_TYPE_ITEM_LASER, position);
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
	int zone_index = (random() * (array_length(spawn_zones))) - 1;

	//Add new enemy types every 5 waves
	int maxValue = ((float)wave / (float)WAVE_ESCALATION_RATE + 0.5f);
	maxValue = SDL_clamp(maxValue, ENTITY_TYPE_ENEMY_DRIFTER, ENTITY_TYPE_ENEMY_GRAPPLER);

	for (float points_remaining = (float)points_max; points_remaining > 0;) {
		//Generate random type between 0 and current maximum point value
		
		Uint8 spawn_type = ENTITY_TYPE_ENEMY_DRIFTER + (int)(random() * (float)(ENTITY_TYPE_ENEMY_GRAPPLER - ENTITY_TYPE_ENEMY_DRIFTER) + 0.5f);
		float spawn_value = get_entity_score_value(spawn_type);

		if (spawn_value && points_remaining >= spawn_value) {
			points_remaining -= spawn_value;
			float entity_radius = 25;
			Vector2 new_position = get_clear_spawn(game, entity_radius, spawn_zones[zone_index]);
			
			Uint32 warp_id = spawn_entity(game, ENTITY_TYPE_SPAWN_WARP, new_position);
			if (warp_id){
				Entity* warp = get_entity(game, warp_id);
				warp->type_data = spawn_type;
			}

		}
		
		zone_index = (zone_index + 1) % array_length(spawn_zones);
	}
}

void update_entities(Game_State* game, float dt) {
	Uint32 dead_entities[DEAD_ENTITY_MAX];
	Uint32 dead_entity_count = 0;
	
	Entity* entity = 0;
	for (int entity_index = 1; entity_index < game->entity_count; entity_index++) {
		entity = game->entities + entity_index;
		
		if (entity->state == ENTITY_STATE_SPAWNING) {

			if (entity->type == ENTITY_TYPE_PLAYER) {
				float t = (entity->y - game->world_h) / (game->world_h/2.0f - game->world_h);
				float ts = sin_deg(t * 90.0f);
				

				Particle_Emitter* thrusters[] = {
					get_particle_emitter(&game->particle_system, entity->particle_emitters[0]),
					get_particle_emitter(&game->particle_system, entity->particle_emitters[1]),
					get_particle_emitter(&game->particle_system, entity->particle_emitters[2]),
				};

				thrusters[0]->state = (entity->sx > 0.4f && entity->sx < 0.9f);
				thrusters[0]->position = entity->position;
				thrusters[0]->angle = normalize_degrees(entity->angle + 180.0f);

				thrusters[1]->state = (entity->sx > 0.5f);
				thrusters[1]->position = entity->position;
				thrusters[1]->angle = normalize_degrees(entity->angle - 45);

				thrusters[2]->state = (entity->sx > 0.5f);
				thrusters[2]->position = entity->position;
				thrusters[2]->angle = normalize_degrees(entity->angle + 45);

				force_circle(game->entities, game->entity_count, entity->x, entity->y, entity->sx * SHIP_RADIUS * 3.0f, 1.5f);
				entity->y -= (8.0f - lerp(0.0f, 6.0f, t)) * dt;
				entity->sx = entity->sy = t;
				entity->shape.radius = ts * SHIP_RADIUS;

				if (entity->y <= game->world_h/2.0f) {
//					startTransition(-1);
//					transitionHUD(-1);
					entity->state = ENTITY_STATE_ACTIVE;
					entity->scale = (Vector2){1.0f, 1.0f};
					entity->shape.radius = SHIP_RADIUS;// * 2.0f;
					entity->velocity = (Vector2){0};
					entity->timer = 0;
				}
			} else {
				entity->timer -= dt * (float)(int)(entity->timer > 0);

				float t = 1.0f - SDL_clamp((entity->timer/ENTITY_WARP_DELAY), 0.0f, 1.0f);

				entity->transform.scale.x = entity->transform.scale.y = t;
						
				if (entity->timer <= 0) {
					switch(entity->type) {
						case ENTITY_TYPE_ITEM_MISSILE: { entity->timer = MISSILE_LIFETIME; }	
						case ENTITY_TYPE_ITEM_LASER: { entity->timer = PLAYER_SHOT_LIFE; }

						default: {
							entity->timer = 100.0f;
						} break;
					}
					entity->state = ENTITY_STATE_ACTIVE;
				}
			}
		} else if (entity->state == ENTITY_STATE_DESPAWNING) {
			entity->timer -= dt * (float)(int)(entity->timer > 0);
			
			entity->transform.scale.x = 
				entity->transform.scale.y = 
					SDL_clamp(entity->timer/ENTITY_WARP_DELAY, 0.0f, 1.0f);

			if (entity->timer <= 0) entity->state = ENTITY_STATE_DYING;
		} else if (entity->state == ENTITY_STATE_DYING) {
			if (dead_entity_count < DEAD_ENTITY_MAX) {
				dead_entities[dead_entity_count] = entity_index;
				dead_entity_count++;
			}
		} else if (entity->state == ENTITY_STATE_ACTIVE) {
			entity->timer -= dt * (float)(int)(entity->timer > 0);
			switch(entity->type) {
				
				case ENTITY_TYPE_PLAYER: {
					Game_Player_Controller controller = game->player_controller;

					if (is_game_control_held(&game->input, &controller.turn_left))  
						entity->angle -= PLAYER_TURN_SPEED * dt;
					if (is_game_control_held(&game->input, &controller.turn_right)) 
						entity->angle += PLAYER_TURN_SPEED * dt;
					
					SDL_bool thrust_inputs[] = {
						is_game_control_held(&game->input, &controller.thrust),
						is_game_control_held(&game->input, &controller.thrust_left),
						is_game_control_held(&game->input, &controller.thrust_right),
					};

					SDL_bool thrusting = 0;
					float angle_offset = 0.0f;
					for (int i = 0; i < entity->emitter_count; i++) {
						Particle_Emitter* thruster = get_particle_emitter(&game->particle_system, entity->particle_emitters[i]);
						thruster->state = EMITTER_STATE_INACTIVE;
						if (i < array_length(thrust_inputs) && thrust_inputs[i]) {
							thrusting = 1;
							if (game->player_state.thrust_energy > 0) {
								float thrust_speed = (i > 0) ? PLAYER_LATERAL_THRUST : PLAYER_FORWARD_THRUST;

								thruster->state = EMITTER_STATE_ACTIVE;
								entity->vx += cos_deg(entity->angle + angle_offset) * thrust_speed * dt;
								entity->vy += sin_deg(entity->angle + angle_offset) * thrust_speed * dt;
								game->player_state.thrust_energy -= THRUST_CONSUMPTION *dt;
							}
						}

						if (thruster->state == EMITTER_STATE_ACTIVE) {
							thruster->angle = entity->angle + angle_offset + 180;
							thruster->y = entity->y + sin_deg(thruster->angle) * (float)SHIP_RADIUS;
							thruster->x = entity->x + cos_deg(thruster->angle) * (float)SHIP_RADIUS;
						}

						angle_offset -= 90.0f * (float)(i+1);
					}
					game->player_state.thrust_energy = SDL_clamp(game->player_state.thrust_energy + (float)(int)(!thrusting) * dt, 0, THRUST_MAX);

					if (is_game_control_held(&game->input, &controller.fire)) {	
						if (game->player_state.weapon_heat < HEAT_MAX) {
							game->player_state.weapon_heat -= dt;
						
							if (entity->timer <= 0)  {
								switch(entity->type_data) {

									case PLAYER_WEAPON_MG: {
										game->player_state.weapon_heat += PLAYER_MG_HEAT;
										// TO-DO: Figure out weird galloping timing problem for multiple SFX calls in a row
										Mix_PlayChannel(-1, game_get_sfx(game, "Player Shot"), 0);
										Uint32 bullet_id = spawn_entity(game, ENTITY_TYPE_BULLET, entity->position);
										if (bullet_id) {
											Entity* bullet = get_entity(game, bullet_id);

											bullet->timer = PLAYER_SHOT_LIFE;
											Vector2 angle = {
												cos_deg(entity->angle),
												sin_deg(entity->angle)
											};
											
											bullet->x += angle.x * SHIP_RADIUS;
											bullet->y += angle.y * SHIP_RADIUS;
											
											bullet->vx = entity->vx + (angle.x * PLAYER_SHOT_SPEED);
											bullet->vy = entity->vy + (angle.y * PLAYER_SHOT_SPEED);
											bullet->color = SD_BLUE;
											bullet->team = ENTITY_TEAM_PLAYER;
										}
										
										entity->timer = PLAYER_MG_COOLDOWN;
									} break;
									
									case PLAYER_WEAPON_MISSILE: {
										if (game->player_state.ammo <= 0) { 
											game->player->type_data = 0;
											break;	
										}
										
										game->player_state.weapon_heat += PLAYER_MISSILE_HEAT;

										float position_offset = -60.0f;
										Vector2 angle = { cos_deg(entity->angle), sin_deg(entity->angle) };
										
										for (int i = 0; i < 2; i++) {
											Uint32 missile_id = spawn_entity(game, ENTITY_TYPE_MISSILE, entity->position);
											if (missile_id) {
												Entity* missile = get_entity(game, missile_id);
												
												missile->timer /= 2.0f;

												missile->x += cos_deg(entity->angle + position_offset) * SHIP_RADIUS;
												missile->y += sin_deg(entity->angle + position_offset) * SHIP_RADIUS;
												
												missile->angle = entity->angle;
												missile->vx = entity->vx + angle.x;
												missile->vy = entity->vy + angle.y;
												missile->team = ENTITY_TEAM_PLAYER;
											
												position_offset *= -1;
											}
										}

										game->player_state.ammo--;
										entity->timer = PLAYER_MISSILE_COOLDOWN;

									} break;
									
									case PLAYER_WEAPON_LASER: {
										if (game->player_state.ammo <= 0) { 
											game->player->type_data = 0;
											break;	
										}
										
										game->player_state.weapon_heat += PLAYER_LASER_HEAT;

										float position_offset = -60.0f;
										Vector2 angle = { cos_deg(entity->angle), sin_deg(entity->angle) };
										
										for (int i = 0; i < 2; i++) {
											Uint32 laser_id = spawn_entity(game, ENTITY_TYPE_LASER, entity->position);
											if (laser_id) {
												Entity* laser = get_entity(game, laser_id);

												laser->state = ENTITY_STATE_ACTIVE;
												laser->timer = PLAYER_SHOT_LIFE;
												
												laser->x += cos_deg(entity->angle + position_offset) * SHIP_RADIUS;
												laser->y += sin_deg(entity->angle + position_offset) * SHIP_RADIUS;
												
												laser->angle = entity->angle;
												laser->vx = entity->vx + angle.x * PLAYER_SHOT_SPEED * 2.0f;
												laser->vy = entity->vy + angle.y * PLAYER_SHOT_SPEED * 2.0f;
												laser->team = ENTITY_TEAM_PLAYER;
											
												position_offset *= -1;
											}
										}

										game->player_state.ammo--;
										entity->timer = PLAYER_LASER_COOLDOWN;

									} break;
								}
							}
						}

					} else if (game->player_state.weapon_heat > 0) {
						game->player_state.weapon_heat -= dt;
					}

					game->player_state.weapon_heat = SDL_clamp(game->player_state.weapon_heat, 0, HEAT_MAX);
				} break;

				case ENTITY_TYPE_LASER:
				case ENTITY_TYPE_BULLET: {
					if (entity->timer <= 0) {
						entity->state = ENTITY_STATE_DESPAWNING;
						entity->timer = ENTITY_WARP_DELAY;
					}
					entity->vx *= 1.0f + PHYSICS_FRICTION;
					entity->vy *= 1.0f + PHYSICS_FRICTION;
				} break;

				case ENTITY_TYPE_MISSILE: {
					if (entity->timer <= 0) { entity->state = ENTITY_STATE_DYING; }
					
					Particle_Emitter* thruster = get_particle_emitter(&game->particle_system, entity->particle_emitters[0]);
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
						for (int i = 0; i < game->entity_count; i++) {
							potential_target = game->entities + i;
							if (potential_target == entity || 
								potential_target->team == ENTITY_TEAM_UNDEFINED || 
								potential_target->team == entity->team) {
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

						float aim_offset = get_aim_offset(entity->position, target->position, entity->angle, 0.0);
						entity->angle += aim_offset * MISSILE_TURN_RATE * dt;
					}

					Vector2 acceleration = {
						cos_deg(entity->angle) + delta.x,
						sin_deg(entity->angle) + delta.y
					};
					acceleration = scale_vector2(normalize_vector2(acceleration), MISSILE_ACCEL * dt);
					entity->velocity = add_vector2(entity->velocity, acceleration);
				} break;

				case ENTITY_TYPE_ENEMY_DRIFTER: {
					entity->velocity = scale_vector2(
						normalize_vector2(entity->velocity),
						DRIFTER_SPEED
					);
				} break;
				
				case ENTITY_TYPE_ENEMY_UFO: { 
					float angle_delta = cos_deg(entity->target_angle) * sin_deg(entity->angle) - sin_deg(entity->target_angle) * cos_deg(entity->angle);

					if (SDL_fabs(angle_delta) > UFO_TURN_PRECISION) {
						entity->timer += dt;
						entity->angle += dt * (float)(1 - ((int)(angle_delta < 0) * 2));
		
						entity->vx += cos_deg(entity->angle) * UFO_SPEED * 0.025;
						entity->vx += sin_deg(entity->angle) * UFO_SPEED * 0.025;
					}

					if (entity->timer <= 0) {
						entity->target_angle = random() * 360.0f;
						entity->timer = UFO_DIR_CHANGE_DELAY;
					}

					float magnitude = SDL_sqrt( (entity->vx * entity->vx) + (entity->vy * entity->vy) );

					if (magnitude > UFO_SPEED) {
						entity->vx *= 1 - 0.028 * dt;
						entity->vy *= 1 - 0.028 * dt;
					} else if (magnitude < UFO_SPEED - 0.05) {
						entity->vx += cos_deg(entity->angle) * UFO_SPEED * 0.025;
						entity->vy += sin_deg(entity->angle) * UFO_SPEED * 0.025;
					}
				} break;
				
				case ENTITY_TYPE_ENEMY_TRACKER:{
					Particle_Emitter* thruster = get_particle_emitter(&game->particle_system, entity->particle_emitters[0]);
					if (thruster) thruster->state = 0;
				
					Entity* target = get_enemy_target(game);
					if (target) {
						float acceleration_speed = TRACKER_ACCEL/2.0f;
						float aim_offset = get_aim_offset(entity->position, target->position, entity->angle, TRACKER_PRECISION);

						if (aim_offset == 0) {
							acceleration_speed *=2;
							if (thruster) thruster->state = 1;
						} else {
							entity->angle += TRACKER_TURN_RATE * aim_offset * dt;
						}

						if (thruster && thruster->state == 1) {
							thruster->angle = entity->angle + 180;
							thruster->x = entity->x + cos_deg(thruster->angle);// * (entity->shape.radius + PARTICLE_MAX_START_RADIUS) / 2.0f;
							thruster->y = entity->y + sin_deg(thruster->angle);// * (entity->shape.radius + PARTICLE_MAX_START_RADIUS) / 2.0f;
						}

						entity->vx += cos_deg(entity->angle) * acceleration_speed * dt;
						entity->vy += sin_deg(entity->angle) * acceleration_speed * dt;
					}

				} break;
				
				case ENTITY_TYPE_ENEMY_TURRET: {
					switch(entity->type_data) {
						case TURRET_STATE_AIMING: {
							Entity* target = get_enemy_target(game);
							if (target) {
								float aim_offset = get_aim_offset(entity->position, target->position, entity->angle, TURRET_AIM_TOLERANCE);
								if (aim_offset == 0 && entity->timer <= 0) {
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
										Uint32 new_shot_id = spawn_entity(game, ENTITY_TYPE_BULLET, position);

										if (new_shot_id){
											Entity* new_shot = get_entity(game, new_shot_id);
											new_shot->x += cos_deg(shot_offset_angle) * TURRET_RADIUS;
											new_shot->y += sin_deg(shot_offset_angle) * TURRET_RADIUS;
											new_shot->velocity = velocity;
											new_shot->shape.type = SHAPE_TYPE_CIRCLE;
											new_shot->shape.radius = TURRET_SHOT_RADIUS;
											new_shot->timer = TURRET_SHOT_LIFE;

											shot_offset_angle = normalize_degrees(shot_offset_angle + 180.0f);
										}
									}
								
									entity->timer = TURRET_FIRE_ANIM_SPEED;
									entity->type_data = TURRET_STATE_FIRING;
								} else {
									entity->angle += aim_offset * TURRET_TURN_SPEED * dt;
								}
							}
						} break;

						case TURRET_STATE_FIRING : {
							float t = entity->timer / (float)TURRET_FIRE_ANIM_SPEED;
							entity->sprites[1].offset.x = 0.0f;
							entity->sprites[1].offset.x -= (float)TURRET_RADIUS - lerp(0, TURRET_RADIUS, t);
							if (entity->timer <= 0) {
								entity->type_data = TURRET_STATE_RECOVERING;
								entity->timer = TURRET_RECOVERY_ANIM_SPEED;
							}
						} break;

						case TURRET_STATE_RECOVERING: {
							float t = entity->timer / (float)TURRET_RECOVERY_ANIM_SPEED;
							entity->sprites[1].offset.x = 0;
							entity->sprites[1].offset.x -= lerp(0, TURRET_RADIUS, t);
							if (entity->timer <= 0) {
								entity->sprites[1].offset.x = 0;
								entity->type_data = TURRET_STATE_AIMING;
								entity->timer = 0;
							}
						} break;
					}
					
				} break;
				
				case ENTITY_TYPE_ENEMY_GRAPPLER: {
					Entity* target = get_enemy_target(game);

					switch(entity->type_data) {
						case GRAPPLER_STATE_AIMING: {
							if (target) {
								float aim_delta = get_aim_offset(entity->position, target->position, entity->angle, GRAPPLER_AIM_TOLERANCE);
								if (aim_delta == 0)
									entity->type_data = GRAPPLER_STATE_EXTENDING;
								else 
									entity->angle += aim_delta * GRAPPLER_TURN_SPEED * dt;
							}
						} break;

						case GRAPPLER_STATE_EXTENDING: {
							entity->sprites[1].offset.x += GRAPPLER_HOOK_SPEED * dt;

							Vector2 hook_position = rotate_vector2(entity->sprites[1].offset, entity->angle);
							hook_position.x += entity->position.x;
							hook_position.y += entity->position.y;

							Vector2 overlap = {0};
							if (target && sc2d_check_circles(hook_position.x, hook_position.y, (float)TURRET_RADIUS / 1.5f,
												   			 target->x, target->y, target->shape.radius,
												   			 &overlap.x, &overlap.y)) {
									
									entity->type_data = GRAPPLER_STATE_REELING;
							} else if (!sc2d_check_point_rect( 	hook_position.x, hook_position.y,
																0, 0, game->world_w, game->world_h,
																&overlap.x, &overlap.y)) {

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
								
								float magnitude = sqrtf( (delta.x * delta.x) + (delta.y * delta.y));
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


				} break;

				case ENTITY_TYPE_ITEM_LIFEUP: {
					entity->angle += dt;
				} break;

				case ENTITY_TYPE_ITEM_MISSILE: {
					entity->angle += dt;
				} break;

				case ENTITY_TYPE_ITEM_LASER: {
					entity->angle += dt;
				} break;

				case ENTITY_TYPE_SPAWN_WARP: {
					spawn_entity(game, entity->type_data, entity->position);
					entity->timer = ENTITY_WARP_DELAY;
					entity->state = ENTITY_STATE_DESPAWNING;
				} break;
			}

			normalize_degrees(entity->angle);
		
			entity->x += entity->vx * dt;
			entity->y += entity->vy * dt;

			entity->vx *= 1.0 - (PHYSICS_FRICTION*dt);
			entity->vy *= 1.0 - (PHYSICS_FRICTION*dt);

			entity->position = wrap_world_coords(entity->x, entity->y, 0, 0, game->world_w, game->world_h);
		
			if (entity->type  != ENTITY_TYPE_SPAWN_WARP) {
				for (int collision_entity_index = entity_index+1; collision_entity_index < game->entity_count; collision_entity_index++) {
					Entity* collision_entity 	 = game->entities + collision_entity_index;
					if (collision_entity->state != ENTITY_STATE_ACTIVE || collision_entity->type == ENTITY_TYPE_SPAWN_WARP) continue;
					
					Vector2 overlap = {0};
					
					Entity* item_entity = 	(entity_is_item(entity->type)) ? entity :
											(entity_is_item(collision_entity->type)) ? collision_entity :
											0;

					Entity* player_entity = (entity->type == ENTITY_TYPE_PLAYER) ? entity :
											(collision_entity->type == ENTITY_TYPE_PLAYER) ? collision_entity :
											0;

					
					if (check_shape_collision(entity->transform, entity->shape, collision_entity->transform, collision_entity->shape, &overlap)) {
						if (item_entity && player_entity) {
							item_entity->state = ENTITY_STATE_DESPAWNING;
							item_entity->timer = ENTITY_WARP_DELAY/2.0f;
						
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

			Game_Shape entity_shape = scale_game_shape(entity->shape, entity->scale);
			for (int particle_index = 0; particle_index < game->particle_system.particle_count; particle_index++) {
				Particle* particle = game->particle_system.particles + particle_index;
				Vector2 overlap = {0};
				Game_Shape particle_shape = scale_game_shape(particle->shape, particle->scale);

				if ( check_shape_collision(particle->transform, particle_shape, entity->transform, entity_shape, &overlap)) {
					particle->x -= overlap.x;
					particle->y -= overlap.y;

					float magnitude = vector2_length(particle->velocity);

					particle->velocity = normalize_vector2(particle->velocity);

					Vector2 new_v = {-overlap.x, -overlap.y};//subtract_vector2(particle->position, entity->position);
							new_v = normalize_vector2(new_v);
							new_v = add_vector2(new_v, particle->velocity);
							new_v = normalize_vector2(new_v);
					
					particle->velocity = scale_vector2(new_v, magnitude / 2.0f);
				}
			}
		} // end of if (entity->state == ENTITY_STATE_ACTIVE)
	}

	if (dead_entity_count > 0) {
		Entity* dead_entity;
		for (int i = dead_entity_count-1; i >= 0 ; i--) {
			Uint32 dead_entity_index = dead_entities[i]; 
			dead_entity = game->entities + dead_entity_index;
			
			for (int i = 0; i < dead_entity->emitter_count; i++) {
				remove_particle_emitter(&game->particle_system, dead_entity->particle_emitters[i]);
			}
			dead_entity->emitter_count = 0;

			switch(dead_entity->type) {
				case ENTITY_TYPE_PLAYER: {
					game->player = 0;
					game->player_state.thrust_energy = THRUST_MAX;
					game->player_state.weapon_heat = 0;

					end_score_combo(&game->score);
				} break;

				case ENTITY_TYPE_ENEMY_DRIFTER: {
					float v_max = 0;
					for (int i = 0; i < dead_entity->shape.polygon.vert_count; i++) {
						v_max = SDL_max(dead_entity->shape.polygon.vertices[i].x, v_max);
						v_max = SDL_max(dead_entity->shape.polygon.vertices[i].y, v_max);
					}

					if (v_max > DRIFTER_RADIUS/2) {
						float angle = random() * 360.0f;
						for (int i = 0; i < 3; i++) {
							Vector2 position = dead_entity->position;
							position.x += cos_deg(angle) * (float)DRIFTER_RADIUS/2.0f;
							position.y += sin_deg(angle) * (float)DRIFTER_RADIUS/2.0f;

							Uint32 id = spawn_entity(game, ENTITY_TYPE_ENEMY_DRIFTER, position);
							Entity* drifter_child = game->entities + id;
							generate_drifter_verts(&drifter_child->shape, (float)DRIFTER_RADIUS/2.0f);
							drifter_child->angle = angle;
							drifter_child->vx = cos_deg(angle) * (float)DRIFTER_SPEED;
							drifter_child->vy = sin_deg(angle) * (float)DRIFTER_SPEED;
							drifter_child->state = ENTITY_STATE_ACTIVE;

							angle += 360.0f / 3.0f;
						}
					}
				} break;

				case ENTITY_TYPE_ITEM_LIFEUP: {
					game->player_state.lives++;
				} break;
			}

			if (dead_entity->team == ENTITY_TEAM_ENEMY) {
				force_circle(game->entities, game->entity_count, dead_entity->x, dead_entity->y, ENEMY_EXPLOSION_RADIUS, 1.5f);
				float value = get_entity_score_value(dead_entity->type);
				add_score(game, value);
				random_item_spawn(game, dead_entity->position, value);
			}

			if (entity_type_explodes(dead_entity->type)) {
				RGBA_Color entity_color = (dead_entity->color.r || dead_entity->color.g || dead_entity->color.b) ? dead_entity->color : (RGBA_Color){255, 255, 255, 255};
				RGBA_Color colors[] = {entity_color, {255, 255, 255, 255}};

				explode_at_point(&game->particle_system, dead_entity->x, dead_entity->y, colors, array_length(colors), 0, dead_entity->shape.type);
				for (int sprite_index = 0; sprite_index < dead_entity->sprite_count; sprite_index++) {
					explode_sprite(game, dead_entity->sprites+sprite_index, dead_entity->x, dead_entity->y, dead_entity->angle, 6);
				}
			}
			
			if (dead_entity_index < game->entity_count-1) {
				Entity* swap_entity = game->entities + (game->entity_count-1);
				*dead_entity = *swap_entity;
				if (swap_entity == game->player) game->player = dead_entity;
			}
			game->entity_count--;
		}

		//TO-DO: Implement better conditions for this.
		// If there are as many dead entities as entities minus reserved 0 entity and player (if currently alive)
		if (game->entity_count - (int)(game->player > 0) == 1) {
			game->score.current_wave++;
			game->score.spawn_points_max++;
			spawn_wave(game, game->score.current_wave, game->score.spawn_points_max);
		}
	}
}

Rectangle get_entity_bounding_box(Game_State* game, Entity* entity) {
	Rectangle result = {0};

	if (entity->sprite_count > 0) {
		SDL_Rect rect = get_sprite_rect(game, &entity->sprites[0]);
		result = (Rectangle){rect.x, rect.y, rect.w, rect.h};
		result.x -= result.w/2.0f;
		result.y -= result.h/2.0f;
	} else {
		switch(entity->shape.type) {
			case SHAPE_TYPE_CIRCLE: {
				result.x = result.y = -entity->shape.radius;
				result.w = result.h = entity->shape.radius*2.0f;
			} break;

			case SHAPE_TYPE_RECT: {
				// TO-DO: Handle rotation
				result = entity->shape.rectangle;
			} break;
			
			case SHAPE_TYPE_POLY2D: {
				for (int i = 0; i < entity->shape.polygon.vert_count; i++) {
					result.x = SDL_min(result.x, entity->shape.polygon.vertices[i].x);
					result.y = SDL_min(result.y, entity->shape.polygon.vertices[i].y);
					
					result.w = SDL_max(result.w, entity->shape.polygon.vertices[i].x);
					result.h = SDL_max(result.h, entity->shape.polygon.vertices[i].y);
				}
				result.w -= result.x;
				result.h -= result.y;
			} break;
		}
	}

	return result;
}

void draw_entities(Game_State* game) {
	Entity* entity = 0;
	Transform2D transforms[4] = {0};
	int transform_count = 0;

	for (int entity_index = 0; entity_index < game->entity_count; entity_index++) {
		entity = game->entities + entity_index;
		if (entity->state <= 0 || entity->state >= ENTITY_STATE_DYING) continue;

		transforms[0] = transforms[1] = transforms[2]= transforms[3] = entity->transform;
		transform_count = 1;
		Rectangle bounding_box = get_entity_bounding_box(game, entity);

		int wrap_x = (int)(entity->x + bounding_box.x < 0) - (int)(entity->x + bounding_box.w > game->world_w);
		int wrap_y = (int)(entity->y + bounding_box.y < 0) - (int)(entity->y + bounding_box.h > game->world_h);

		if (wrap_x) {
			transforms[transform_count].x += game->world_w * (float)wrap_x;
			transform_count++;
		} 
		
		if (wrap_y) {
			transforms[transform_count].y += game->world_h * (float)wrap_y;
			transform_count++;
		}

		if (wrap_x && wrap_y) {
			transforms[transform_count].x += game->world_w * (float)wrap_x;
			transforms[transform_count].y += game->world_h * (float)wrap_y;
			transform_count++;
		} 

		for (int i = 0; i < transform_count; i++) {
			if (entity->sprite_count > 0) {
				for (int sprite_index = 0; sprite_index < entity->sprite_count; sprite_index++) {
					if (entity->sprites[sprite_index].texture_name != 0) render_draw_game_sprite(game, &entity->sprites[sprite_index], transforms[i], true);	
				}
			}
			else if (entity->shape.type > SHAPE_TYPE_UNDEFINED && entity->shape.type < SHAPE_TYPE_COUNT) {
				Game_Shape  shape = scale_game_shape(entity->shape, transforms[i].scale);
							shape = rotate_game_shape(shape, transforms[i].angle);
				render_fill_game_shape(transforms[i].position, shape, entity->color);
			}

#if DEBUG		
			Rectangle test = bounding_box;
			test.x += transforms[i].x;
			test.y += transforms[i].y;
			
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