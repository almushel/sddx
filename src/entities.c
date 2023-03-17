#include "SDL2/SDL.h"
#include "defs.h"
#include "graphics.h"

#define ENTITY_WARP_DELAY 26.0f
#define DEAD_ENTITY_MAX 16 // Maximum number of dead entities garbage collected per frame

#define SPACE_FRICTION 0.02f
#define THRUST_POWER 0.15f
#define LATERAL_THRUST 0.2f
#define TURN_RATE 0.025f
#define PLAYER_SHOT_MAX 8
#define PLAYER_SHOT_RADIUS 2.5f
#define PLAYER_SHOT_SPEED 7.0f
#define PLAYER_SHOT_LIFE 80.0f
#define HEAT_MAX 100
#define THRUST_MAX 100
#define THRUST_CONSUMPTION 0.3f
#define SHIP_RADIUS 13
#define PLAYER_STARTING_LIVES 3

#define PLAYER_MG_COOLDOWN 200.0f/16.666f
#define PLAYER_LASER_COOLDOWN 250.0f/16.666f
#define PLAYER_MISSILE_COOLDOWN 800.0f/16.666f

#define DRIFT_RATE 1
#define DRIFT_RADIUS 40

#define UFO_SPEED 1.9
#define UFO_DIR_CHANGE_DELAY 120.0f
#define UFO_COLLISION_RADIUS 20
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
#define TURRET_AIM_TOLERANCE 15
#define TURRET_TURN_SPEED 1.2
#define TURRET_FIRE_ANIM_SPEED 5 //83ms
#define TURRET_RECOVERY_ANIM_SPEED 60 //500ms

#define GRAPPLER_AIM_TOLERANCE 0.2
#define GRAPPLER_TURN_SPEED 1.3
#define GRAPPLER_SPACE_FRICTION 0.06
#define GRAPPLER_ACCEL 0.06

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

void generate_drifter_texture(Game_State* game) {
	SDL_Surface* surface = SDL_CreateRGBSurface(0, DRIFT_RADIUS*2, DRIFT_RADIUS*2, 32, STBI_MASK_R, STBI_MASK_G, STBI_MASK_B, STBI_MASK_A);
	SDL_Renderer* software_renderer = SDL_CreateSoftwareRenderer(surface);

	int vert_count = 5 + SDL_floor(random() * 4);
	Vector2* vertices = SDL_malloc(sizeof(Vector2) * vert_count);
	for (int i = 0; i < vert_count; i++) {
		float point_dist = DRIFT_RADIUS / 2.0f + random() * DRIFT_RADIUS / 2.0f;
		float new_angle = 360.0f / (float)vert_count * (float)i;
		
		vertices[i].x = cos_deg(new_angle) * point_dist;
		vertices[i].y = sin_deg(new_angle) * point_dist;
	}

	SDL_SetRenderDrawColor(software_renderer, 0, 0, 0, 0);
	SDL_RenderClear(software_renderer);
	SDL_SetRenderDrawColor(software_renderer, 255, 255, 255, 255);
	
	for (int vert_index = 0; vert_index < vert_count; vert_index++) {
		SDL_RenderDrawLineF(software_renderer,
			(float)DRIFT_RADIUS + vertices[ vert_index ].x, 				(float)DRIFT_RADIUS + vertices[ vert_index].y,
			(float)DRIFT_RADIUS + vertices[(vert_index+1) % vert_count].x, 	(float)DRIFT_RADIUS + vertices[(vert_index+1) % vert_count].y
		);
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(game->renderer, surface);
	if (texture) {
		game_store_texture(game, texture, "Enemy Drifter");
	} else {
		SDL_Log(SDL_GetError());
	}

	SDL_free(vertices);
	SDL_FreeSurface(surface);
	SDL_DestroyRenderer(software_renderer);
}

Entity* spawn_entity(Game_State* game, Entity_Types type, Vector2 position) {
	Entity* result = 0;
	if (type > ENTITY_TYPE_UNDEFINED && type < ENTITY_TYPE_COUNT) {
		result = game->entities + get_new_entity(game);
	}

	if (result) {
		*result = (Entity){0};
		result->type = type;
		result->position = position;
		result->scale.x = result->scale.y = 1.0f;
		result->collision_radius = 25.0f;
		result->timer = ENTITY_WARP_DELAY;
		result->state = ENTITY_STATE_SPAWNING;

		switch(type) {
			case ENTITY_TYPE_PLAYER: {
				result->angle = 270;
				result->sprites[0].texture_name = "Player Ship";
				result->sprites[0].rotation = 1;
				result->sprite_count = 1;

				result->data.player.main_thruster  = 	new_particle_emitter(&game->particle_system);
				result->data.player.left_thruster  = 	new_particle_emitter(&game->particle_system);
				result->data.player.right_thruster = 	new_particle_emitter(&game->particle_system);
				
				Particle_Emitter* main_thruster = get_particle_emitter(&game->particle_system, result->data.player.main_thruster);
				*main_thruster = (Particle_Emitter){0};
				main_thruster->angle = 180;
				main_thruster->density = 2.0f;
				main_thruster->colors[0] = SD_BLUE;
				main_thruster->color_count = 1;

				Particle_Emitter* left_thruster = get_particle_emitter(&game->particle_system, result->data.player.left_thruster);
				*left_thruster = *main_thruster;
				left_thruster->angle = 90.0f;

				Particle_Emitter* right_thruster = get_particle_emitter(&game->particle_system, result->data.player.right_thruster);
				*right_thruster = *main_thruster;
				right_thruster->angle = -90.0f;
			} break;

			case ENTITY_TYPE_BULLET: {
				result->collision_radius = PLAYER_SHOT_RADIUS;
				result->state = ENTITY_STATE_ACTIVE;
				result->shape = PRIMITIVE_SHAPE_CIRCLE;
				result->color = (RGB_Color){255, 150, 50};
			} break;

			case ENTITY_TYPE_ENEMY_DRIFTER: {
				result->angle = random() * 360.0f;
				result->sprites[0].texture_name = "Enemy Drifter";
				result->sprites[0].rotation = 1;
				result->sprite_count = 1;
			} break;

			case ENTITY_TYPE_ENEMY_UFO: {
				result->angle = result->target_angle = random() * 360.0f;
				result->sprites[0].texture_name = "Enemy UFO";
				result->sprite_count = 1;
			} break;
			
			case ENTITY_TYPE_ENEMY_TRACKER: {
				result->sprites[0].texture_name = "Enemy Tracker";
				result->sprites[0].rotation = 1;
				result->sprite_count = 1;

				result->data.tracker.thruster = new_particle_emitter(&game->particle_system);
				Particle_Emitter* thruster = get_particle_emitter(&game->particle_system, result->data.tracker.thruster);
				thruster->angle = 180;
				thruster->density = 1.5f;
				thruster->colors[0] = (RGB_Color){255, 0, 0};
				thruster->color_count = 1;
			} break;

			case ENTITY_TYPE_ENEMY_TURRET: {
				result->sprites[0].texture_name = "Enemy Turret Base";
				result->sprites[1].texture_name = "Enemy Turret Cannon";
				result->sprites[1].rotation = 1;
				result->sprite_count = 2;
			} break;
			
			case ENTITY_TYPE_ENEMY_GRAPPLER: {
				result->sprites[0].texture_name = "Enemy Grappler";
				result->sprites[0].rotation = 1;
				result->sprites[1].texture_name = "Grappler Hook";
				result->sprites[1].rotation = 1;
				result->sprite_count = 2;
			} break;

			case ENTITY_TYPE_SPAWN_WARP: {
				result->shape = PRIMITIVE_SHAPE_CIRCLE;
				result->color = SD_BLUE;
			} break;
		}
	}

	return result;
}


void update_entities(Game_State* game, float dt) {
	Uint32 dead_entities[DEAD_ENTITY_MAX];
	Uint32 dead_entity_count = 0;
	
	Entity* entity = 0;
	for (int entity_index = 0; entity_index < game->entity_count; entity_index++) {
		entity = game->entities + entity_index;
		
		if (entity->state == ENTITY_STATE_SPAWNING) {
			if (entity->timer > 0) entity->timer -= dt;

			entity->transform.scale.x = 
				entity->transform.scale.y = 
					SDL_clamp(1.0f - (entity->timer/ENTITY_WARP_DELAY), 0.0f, 1.0f);
					
			if (entity->timer <= 0) {
				//spawn enemy
				entity->timer = 100.0f;
				entity->state = ENTITY_STATE_ACTIVE;
			}
		} else if (entity->state == ENTITY_STATE_DESPAWNING) {
			if (entity->timer > 0) entity->timer -= dt;
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
			if (entity->timer > 0) entity->timer -= dt;
			switch(entity->type) {
				case ENTITY_TYPE_PLAYER: {
					if (game->player_controller.turn_left.held) entity->angle -= PLAYER_TURN_SPEED * dt;
					if (game->player_controller.turn_right.held) entity->angle += PLAYER_TURN_SPEED * dt;
					
					if (game->player_controller.thrust.held) {
						entity->vx += cos_deg(entity->angle) * PLAYER_FORWARD_THRUST * dt;
						entity->vy += sin_deg(entity->angle) * PLAYER_FORWARD_THRUST * dt;
					}

					if (game->player_controller.thrust_left.held) {
						entity->vx += cos_deg(entity->angle + 90) * PLAYER_LATERAL_THRUST * dt;
						entity->vy += sin_deg(entity->angle + 90) * PLAYER_LATERAL_THRUST * dt;
					}
					
					if (game->player_controller.thrust_right.held) {
						entity->vx += cos_deg(entity->angle - 90) * PLAYER_LATERAL_THRUST * dt;
						entity->vy += sin_deg(entity->angle - 90) * PLAYER_LATERAL_THRUST * dt;
					}

					if (game->player_controller.fire.held && entity->timer <= 0)  {
						// TO-DO: Figure out weird galloping timing problem for multiple SFX calls in a row
						Mix_PlayChannel(-1, game_get_sfx(game, "Player Shot"), 0);
						Entity* bullet = spawn_entity(game, ENTITY_TYPE_BULLET, entity->position);
						if (bullet) {
							bullet->timer = PLAYER_SHOT_LIFE;
							Vector2 angle = {
								cos_deg(entity->angle),
								sin_deg(entity->angle)
							};
							
							bullet->x += angle.x * SHIP_RADIUS;
							bullet->y += angle.y * SHIP_RADIUS;
							
							bullet->vx = entity->vx + (angle.x * PLAYER_SHOT_SPEED);
							bullet->vy = entity->vy + (angle.y * PLAYER_SHOT_SPEED);
							bullet->data.projectile.player_shot = 1;
							bullet->color = SD_BLUE;
						}
						
						entity->timer = PLAYER_MG_COOLDOWN;
					}

					Particle_Emitter* main_thruster = get_particle_emitter(&game->particle_system, entity->data.player.main_thruster);
					Particle_Emitter* left_thruster = get_particle_emitter(&game->particle_system, entity->data.player.left_thruster);
					Particle_Emitter* right_thruster = get_particle_emitter(&game->particle_system, entity->data.player.right_thruster);
					
					if (main_thruster) {
						main_thruster->active = game->player_controller.thrust.held;
						main_thruster->angle = entity->angle + 180;
						main_thruster->x = entity->x + cos_deg(main_thruster->angle) * SHIP_RADIUS;
						main_thruster->y = entity->y + sin_deg(main_thruster->angle) * SHIP_RADIUS;
					}
					if (left_thruster) {
						left_thruster->active = game->player_controller.thrust_right.held;
						left_thruster->angle = entity->angle + 90;
						left_thruster->x = entity->x;
						left_thruster->y = entity->y;
					}
					if (right_thruster) {
						right_thruster->active  = game->player_controller.thrust_left.held;
						right_thruster->angle = entity->angle - 90;
						right_thruster->x = entity->x;
						right_thruster->y = entity->y;
					}
				} break;

				case ENTITY_TYPE_BULLET: {
					if (entity->timer <= 0) {
						entity->state = ENTITY_STATE_DESPAWNING;
						entity->timer = ENTITY_WARP_DELAY;
					}
					entity->vx *= 1.0f + PHYSICS_FRICTION;
					entity->vy *= 1.0f + PHYSICS_FRICTION;
				} break;

				case ENTITY_TYPE_ENEMY_DRIFTER:{ 
					entity->vx = cos_deg(entity->angle) * DRIFT_RATE;
					entity->vy = sin_deg(entity->angle) * DRIFT_RATE;
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
					Entity* target = game->player;
					entity->target_angle = atan2_deg(target->y - entity->y, target->x - entity->x); //Angle to player
					
					Particle_Emitter* thruster = get_particle_emitter(&game->particle_system, entity->data.tracker.thruster);
					if (thruster) {
						thruster->angle = entity->angle + 180;
						thruster->x = entity->x;
						thruster->y = entity->y;
					}

					entity->target_angle = normalize_degrees(entity->target_angle);
					float angle_delta = cos_deg(entity->target_angle)*sin_deg(entity->angle) - sin_deg(entity->target_angle)*cos_deg(entity->angle);
					float acceleration_speed = TRACKER_ACCEL/2.0f;

					if (fabs(angle_delta) > TRACKER_PRECISION) {
						float sign = (float)(1 - ((int)(angle_delta < 0) * 2));
						entity->angle -= TRACKER_TURN_RATE * sign * dt;
						if (thruster) thruster->active = 0;
					} else {
						acceleration_speed *=2;
						if (thruster) thruster->active = 1;
					}

					entity->vx += cos_deg(entity->angle) * acceleration_speed * dt;
					entity->vy += sin_deg(entity->angle) * acceleration_speed * dt;
				} break;
				
				case ENTITY_TYPE_ENEMY_TURRET:{
					Entity* target = game->player;
					float delta_x = target->x - entity->x;
					float delta_y = target->y - entity->y;

					float angle_delta = delta_x * sin_deg(entity->angle) - delta_y * cos_deg(entity->angle);

					if (angle_delta < -TURRET_AIM_TOLERANCE) {
						entity->angle += TURRET_TURN_SPEED * dt;
					} else if (angle_delta > TURRET_AIM_TOLERANCE) {
						entity->angle -= TURRET_TURN_SPEED * dt;
					} else if (entity->timer <= 0) {
						Vector2 position = {
							entity->position.x + cos_deg(entity->angle) * TURRET_RADIUS,
							entity->position.y + sin_deg(entity->angle) * TURRET_RADIUS
						};
						
						Vector2 velocity = {
							cos_deg(entity->angle) * TURRET_SHOT_SPEED,
							sin_deg(entity->angle) * TURRET_SHOT_SPEED
						};

						Entity* new_shot = spawn_entity(game, ENTITY_TYPE_BULLET, position);
						new_shot->x += cos_deg(entity->angle + 90.0f) * TURRET_RADIUS;
						new_shot->y += sin_deg(entity->angle + 90.0f) * TURRET_RADIUS;
						new_shot->velocity = velocity;
						new_shot->collision_radius = TURRET_SHOT_RADIUS;
						new_shot->timer = TURRET_SHOT_LIFE;

						new_shot = spawn_entity(game, ENTITY_TYPE_BULLET, position);
						new_shot->x += cos_deg(entity->angle - 90.0f) * TURRET_RADIUS;
						new_shot->y += sin_deg(entity->angle - 90.0f) * TURRET_RADIUS;
						new_shot->velocity = velocity;
						new_shot->collision_radius = TURRET_SHOT_RADIUS;
						new_shot->timer = TURRET_SHOT_LIFE;
					
						entity->timer = TURRET_FIRE_ANIM_SPEED + TURRET_RECOVERY_ANIM_SPEED;
					}

					// update fire animation
				} break;
				
				case ENTITY_TYPE_ENEMY_GRAPPLER:{
					Entity* target = game->player;
					float delta_x, delta_y;

					SDL_bool hook_active = SDL_FALSE; //TO-DO: actual checks for this
					if (hook_active) {
						Entity* hook = 0;
						delta_x = hook->x - entity->x,
						delta_y = hook->y - entity->y;
						entity->angle = atan2_deg(delta_y, delta_x);
						return;
					}

					delta_x = target->x + target->vx - entity->x,
					delta_y = target->y + target->vy - entity->y;

					float angle_delta = delta_x * sin_deg(entity->angle) - delta_y * cos_deg(entity->angle);

					if (angle_delta < -GRAPPLER_AIM_TOLERANCE) {
						entity->angle += GRAPPLER_TURN_SPEED * dt;
					} else if (angle_delta > GRAPPLER_AIM_TOLERANCE) {
						entity->angle -= GRAPPLER_TURN_SPEED * dt;
					} else {
						// Extend grappling hook
					}
				} break;

				case ENTITY_TYPE_SPAWN_WARP: {
					spawn_entity(game, entity->data.spawn_warp.spawn_type, entity->position);
					entity->state = ENTITY_STATE_DESPAWNING;
				} break;
			}
		
			entity->x += entity->vx * dt;
			entity->y += entity->vy * dt;

			entity->vx *= 1.0 - (PHYSICS_FRICTION*dt);
			entity->vy *= 1.0 - (PHYSICS_FRICTION*dt);

			if (entity->x < 0) entity->x = SCREEN_WIDTH + entity->x;
			else if (entity->x > SCREEN_WIDTH) entity->x -= SCREEN_WIDTH;

			if (entity->y < 0) entity->y = SCREEN_HEIGHT + entity->y;
			else if (entity->y > SCREEN_HEIGHT) entity->y -= SCREEN_HEIGHT;
		}
	}

	if (dead_entity_count > 0) {
		Entity* entity;
		for (int dead_entity_index = 0; dead_entity_index < dead_entity_count; dead_entity_index++) {
			entity = game->entities + dead_entities[dead_entity_index];
			for (int sprite_index = 0; sprite_index < entity->sprite_count; sprite_index++) {
				explode_sprite(game, entity->sprites+sprite_index, entity->x, entity->y, entity->angle, 6);
			}

			if (dead_entity_index != (game->entity_count-1)) {
				*(Entity*)(entity) = *(Entity*)(game->entities + (game->entity_count-1) );
			}
			game->entity_count--;
		}
	}
}

void draw_entities(Game_State* game) {
	Entity* entity = 0;
	for (int entity_index = 0; entity_index < game->entity_count; entity_index++) {
		entity = game->entities + entity_index;

		if (entity->sprite_count > 0) {
			for (int sprite_index = 0; sprite_index < entity->sprite_count; sprite_index++) {
#ifdef DEBUG			
				if (sprite_index == 0) {
					SDL_Rect debug_rect = get_sprite_rect(game, &entity->sprites[0]);
					SDL_FRect debug_frect = {
						.x = entity->x + (float)debug_rect.x,
						.y = entity->y + (float)debug_rect.y,
						.h = (float)debug_rect.h,
						.w = (float)debug_rect.w,
					};
					debug_frect.x -= debug_frect.w/2.0f;
					debug_frect.y -= debug_frect.h/2.0f;
					
					SDL_SetRenderDrawColor(game->renderer, 255, 0, 0, 255);
					//SDL_RenderDrawRectF(game->renderer, &debug_frect);
					render_circle(game->renderer, entity->x, entity->y, entity->collision_radius);
				}
#endif
				draw_game_sprite(game, &entity->sprites[sprite_index], entity->transform, 1);	
			}
		} else {
			switch (entity->shape) {

				case PRIMITIVE_SHAPE_CIRCLE: {
					SDL_SetRenderDrawColor(game->renderer, entity->color.r, entity->color.g, entity->color.b, 255);
					float scaled_radius = entity->collision_radius * (entity->transform.scale.x + entity->transform.scale.y) / 2.0f;
					render_fill_circlef(game->renderer, entity->x, entity->y, scaled_radius);				
				} break;
//					case PRIMITIVE_SHAPE_RECT:
				default: {
					SDL_SetRenderDrawColor(game->renderer, entity->color.r, entity->color.g, entity->color.b, 255);

					float scaled_radius = entity->collision_radius * (entity->transform.scale.x + entity->transform.scale.y) / 2.0f;
					SDL_FRect p_rect;
					p_rect.x = entity->x - scaled_radius;
					p_rect.y = entity->y - scaled_radius;
					p_rect.w = scaled_radius*2.0f;
					p_rect.h = p_rect.w;

					SDL_RenderFillRectF(game->renderer, &p_rect);
				} break;
			}
		}
	}
}