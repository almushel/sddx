
#include "defs.h"
#include "game_math.h"
#include "graphics.h"
#include "score.h"
#include "entity_spawn.h"

#define ENTITY_WARP_DELAY 26.0f
#define ENTITY_WARP_RADIUS 20
#define DEAD_ENTITY_MAX 16 // Maximum number of dead entities garbage collected per frame

#define SPACE_FRICTION 0.02f
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

#define ITEM_RADIUS 15

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

SDL_Texture* generate_drifter_texture(Game_State* game) {
	SDL_Texture* result = 0;

	result = SDL_CreateTexture(game->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, DRIFT_RADIUS*2, DRIFT_RADIUS*2);
	if (result) {
		SDL_SetTextureBlendMode(result, SDL_BLENDMODE_BLEND);

		int vert_count = 5 + SDL_floor(random() * 4);
		SDL_FPoint* vertices = SDL_malloc(sizeof(SDL_FPoint) * vert_count);
		for (int i = 0; i < vert_count; i++) {
			float point_dist = DRIFT_RADIUS / 2.0f + random() * DRIFT_RADIUS / 2.0f;
			float new_angle = 360.0f / (float)vert_count * (float)i;
			
			vertices[i].x = DRIFT_RADIUS + cos_deg(new_angle) * point_dist;
			vertices[i].y = DRIFT_RADIUS + sin_deg(new_angle) * point_dist;
		}

		SDL_Texture* original_render_target = SDL_GetRenderTarget(game->renderer);
		SDL_SetRenderTarget(game->renderer, result);
		
		SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 0);
		SDL_RenderClear(game->renderer);
		
		SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
		render_fill_polygon(game->renderer, vertices, vert_count, (RGB_Color){105, 105, 105});
		
		SDL_SetRenderTarget(game->renderer, original_render_target);
		SDL_free(vertices);
	}

	return result;
}

SDL_Texture* generate_item_texture(Game_State* game, SDL_Texture* icon) {
	SDL_Texture* result = 0;
	
	int result_size = ITEM_RADIUS*2 + 4;

	result = SDL_CreateTexture(game->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, result_size, result_size);
	if (result) {
		SDL_SetTextureBlendMode(result, SDL_BLENDMODE_BLEND);
		
		SDL_SetRenderTarget(game->renderer, result);

		SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 0);
		SDL_RenderClear(game->renderer);
		render_fill_circlef_linear_gradient(game->renderer, (float)result_size/2.0f, (float)result_size/2.0f, ITEM_RADIUS, CLEAR_COLOR, SD_BLUE);
		
	if (icon) {
		int w, h;
		SDL_QueryTexture(icon, NULL, NULL, &w, &h);
		float dim = w > h ? w : h;
		float ratio = ((float)ITEM_RADIUS * 1.7f)/dim;
		
		SDL_FRect dest = {0};
		dest.w = (float)w * ratio;
		dest.h = (float)h * ratio;
		dest.x = ((float)result_size-dest.w) / 2.0f;
		dest.y = ((float)result_size-dest.h) / 2.0f;
		
		SDL_RenderCopyExF(game->renderer, icon, NULL, &dest, 0, 0, SDL_FLIP_NONE);
	}

//		SDL_SetRenderDrawColor(game->renderer, 255, 255, 0, 255);
//		SDL_RenderDrawLineF(game->renderer, (float)result_size/2.0f, 0, result_size/2.0f, result_size);
//		SDL_RenderDrawLineF(game->renderer, 0, (float)result_size/2.0f, result_size, (float)result_size/2.0f);
	
		SDL_SetRenderTarget(game->renderer, 0);
	}

	return result;
}

static inline SDL_bool entity_is_item(Entity_Types type) {
	return (
		type >= ENTITY_TYPE_ITEM_MISSILE &&
		type <= ENTITY_TYPE_ITEM_LASER
	);
}

static inline Entity* get_enemy_target(Game_State* game) {
	Entity* result = 0;
	if (game->player && game->player->state == ENTITY_STATE_ACTIVE)
		result = game->player;

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
		entity->timer = ENTITY_WARP_DELAY;
		entity->state = ENTITY_STATE_SPAWNING;
		entity->team = ENTITY_TEAM_ENEMY;

		switch(type) {
			case ENTITY_TYPE_PLAYER: {
				entity->team = ENTITY_TEAM_PLAYER;
				entity->collision_radius = SHIP_RADIUS;
				entity->angle = 270;
				entity->sprites[0].texture_name = "Player Ship";
				entity->sprites[0].rotation = 1;
				entity->sprite_count = 1;

				entity->data.player.main_thruster  = 	get_new_particle_emitter(&game->particle_system);
				entity->data.player.left_thruster  = 	get_new_particle_emitter(&game->particle_system);
				entity->data.player.right_thruster = 	get_new_particle_emitter(&game->particle_system);
				
				Particle_Emitter* main_thruster = get_particle_emitter(&game->particle_system, entity->data.player.main_thruster);
				*main_thruster = (Particle_Emitter){0};
				main_thruster->angle = 180;
				main_thruster->density = 2.0f;
				main_thruster->colors[0] = SD_BLUE;
				main_thruster->color_count = 1;

				Particle_Emitter* left_thruster = get_particle_emitter(&game->particle_system, entity->data.player.left_thruster);
				*left_thruster = *main_thruster;
				left_thruster->angle = 90.0f;

				Particle_Emitter* right_thruster = get_particle_emitter(&game->particle_system, entity->data.player.right_thruster);
				*right_thruster = *main_thruster;
				right_thruster->angle = -90.0f;
			} break;

			case ENTITY_TYPE_BULLET: {
				entity->collision_radius = PLAYER_SHOT_RADIUS;
				entity->state = ENTITY_STATE_ACTIVE;
				entity->shape = PRIMITIVE_SHAPE_CIRCLE;
				entity->color = (RGB_Color){255, 150, 50};
			} break;

			case ENTITY_TYPE_ENEMY_DRIFTER: {
				entity->collision_radius = DRIFT_RADIUS;
				entity->angle = random() * 360.0f;
				entity->sprites[0].texture_name = "Enemy Drifter";
				entity->sprites[0].rotation = 1;
				entity->sprite_count = 1;
			} break;

			case ENTITY_TYPE_ENEMY_UFO: {
				entity->collision_radius = UFO_COLLISION_RADIUS;
				entity->angle = entity->target_angle = random() * 360.0f;
				entity->sprites[0].texture_name = "Enemy UFO";
				entity->sprite_count = 1;
			} break;
			
			case ENTITY_TYPE_ENEMY_TRACKER: {
				entity->collision_radius = TRACKER_COLLISION_RADIUS;
				entity->sprites[0].texture_name = "Enemy Tracker";
				entity->sprites[0].rotation = 1;
				entity->sprite_count = 1;

				entity->data.tracker.thruster = get_new_particle_emitter(&game->particle_system);
				Particle_Emitter* thruster = get_particle_emitter(&game->particle_system, entity->data.tracker.thruster);
				thruster->angle = 180;
				thruster->density = 1.5f;
				thruster->colors[0] = (RGB_Color){255, 0, 0};
				thruster->color_count = 1;
			} break;

			case ENTITY_TYPE_ENEMY_TURRET: {
				entity->collision_radius = TURRET_RADIUS;
				entity->sprites[0].texture_name = "Enemy Turret Base";
				entity->sprites[1].texture_name = "Enemy Turret Cannon";
				entity->sprites[1].rotation = 1;
				entity->sprite_count = 2;
			} break;
			
			case ENTITY_TYPE_ENEMY_GRAPPLER: {
				entity->collision_radius = TURRET_RADIUS;
				entity->sprites[0].texture_name = "Enemy Grappler";
				entity->sprites[0].rotation = 1;
				entity->sprites[1].texture_name = "Grappler Hook";
				entity->sprites[1].rotation = 1;
				entity->sprite_count = 2;
			} break;

			case ENTITY_TYPE_ITEM_MISSILE: {
				entity->team = ENTITY_TEAM_UNDEFINED;
				entity->collision_radius = ITEM_RADIUS;
				entity->sprites[0].texture_name = "Item Missile";
				entity->sprites[0].rotation = 1;
				entity->sprite_count = 1;
			} break;

			case ENTITY_TYPE_ITEM_LIFEUP: {
				entity->team = ENTITY_TEAM_UNDEFINED;
				entity->collision_radius = ITEM_RADIUS;
				entity->sprites[0].texture_name = "Item LifeUp";
				entity->sprites[0].rotation = 1;
				entity->sprite_count = 1;
			} break;

			case ENTITY_TYPE_ITEM_LASER: {
				entity->team = ENTITY_TEAM_UNDEFINED;
				entity->collision_radius = ITEM_RADIUS;
				entity->sprites[0].texture_name = "Item Laser";
				entity->sprites[0].rotation = 1;
				entity->sprite_count = 1;
			}

			case ENTITY_TYPE_SPAWN_WARP: {
				entity->collision_radius = ENTITY_WARP_RADIUS;
				entity->team = ENTITY_TEAM_UNDEFINED;
				entity->shape = PRIMITIVE_SHAPE_CIRCLE;
				entity->color = SD_BLUE;
			} break;
		}
	}

	return result;
}


void update_entities(Game_State* game, float dt) {
	Uint32 dead_entities[DEAD_ENTITY_MAX];
	Uint32 dead_entity_count = 0;
	
	Entity* entity = 0;
	for (int entity_index = 1; entity_index < game->entity_count; entity_index++) {
		entity = game->entities + entity_index;
		
		if (entity->state == ENTITY_STATE_SPAWNING) {
			entity->timer -= dt * (float)(int)(entity->timer > 0);

			float t = 1.0f - SDL_clamp((entity->timer/ENTITY_WARP_DELAY), 0.0f, 1.0f);

			entity->transform.scale.x = entity->transform.scale.y = t;
					
			if (entity->timer <= 0) {
				//spawn entity
				switch(entity->type) {
					case ENTITY_TYPE_PLAYER: {
						entity->timer = 0;
					} break;

					default: {
						entity->timer = 100.0f;
					} break;
				}
				entity->state = ENTITY_STATE_ACTIVE;
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
					if (game->player_controller.turn_left.held)  entity->angle -= PLAYER_TURN_SPEED * dt;
					if (game->player_controller.turn_right.held) entity->angle += PLAYER_TURN_SPEED * dt;
					
					SDL_bool thrust_inputs[] = {
						game->player_controller.thrust.held,
						game->player_controller.thrust_right.held,
						game->player_controller.thrust_left.held,
					};
					
					Particle_Emitter* thrusters[] = {
						get_particle_emitter(&game->particle_system, entity->data.player.main_thruster),
						get_particle_emitter(&game->particle_system, entity->data.player.right_thruster),
						get_particle_emitter(&game->particle_system, entity->data.player.left_thruster),
					};

					SDL_bool thrusting = 0;
					float angle_offset = 0.0f;
					for (int i = 0; i < array_length(thrust_inputs); i++) {
						thrusters[i]->state = EMITTER_STATE_INACTIVE;
						if (thrust_inputs[i]) {
							thrusting = 1;
							if (game->player_state.thrust_energy > 0) {
								float thrust_speed = (i > 0) ? PLAYER_LATERAL_THRUST : PLAYER_FORWARD_THRUST;

								thrusters[i]->state = EMITTER_STATE_ACTIVE;
								entity->vx += cos_deg(entity->angle + angle_offset) * thrust_speed * dt;
								entity->vy += sin_deg(entity->angle + angle_offset) * thrust_speed * dt;
								game->player_state.thrust_energy -= THRUST_CONSUMPTION *dt;
							}
						}

						if (thrusters[i]->state == EMITTER_STATE_ACTIVE) {
							thrusters[i]->angle = entity->angle + angle_offset + 180;
							thrusters[i]->y = entity->y + sin_deg(thrusters[i]->angle);// * (SHIP_RADIUS + PARTICLE_MAX_START_RADIUS);
							thrusters[i]->x = entity->x + cos_deg(thrusters[i]->angle);// * (SHIP_RADIUS + PARTICLE_MAX_START_RADIUS);
						}

						angle_offset -= 90.0f * (float)(i+1);
					}
					game->player_state.thrust_energy = SDL_clamp(game->player_state.thrust_energy + (float)(int)(!thrusting) * dt, 0, THRUST_MAX);

					if (game->player_controller.fire.held) {
						if (game->player_state.weapon_heat < HEAT_MAX) {
							game->player_state.weapon_heat -= dt;
						
							if (entity->timer <= 0)  {
							// TO-DO: Figure out weird galloping timing problem for multiple SFX calls in a row
							game->player_state.weapon_heat += PLAYER_MG_HEAT;
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
							}
						}

					// NOTE: Would prefer to cool down as long as !fire.held && weapon_heat < HEAT_MAX
					} else if (game->player_state.weapon_heat > 0) {
						game->player_state.weapon_heat -= dt;
					}

					game->player_state.weapon_heat = SDL_clamp(game->player_state.weapon_heat, 0, HEAT_MAX);
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
					Particle_Emitter* thruster = get_particle_emitter(&game->particle_system, entity->data.tracker.thruster);
					if (thruster) thruster->state = 0;
				
					Entity* target = get_enemy_target(game);
					if (target) {
						entity->target_angle = atan2_deg(target->y - entity->y, target->x - entity->x); //Angle to player

						entity->target_angle = normalize_degrees(entity->target_angle);
						float angle_delta = cos_deg(entity->target_angle)*sin_deg(entity->angle) - sin_deg(entity->target_angle)*cos_deg(entity->angle);
						float acceleration_speed = TRACKER_ACCEL/2.0f;

						if (fabs(angle_delta) > TRACKER_PRECISION) {
							float sign = (float)(1 - ((int)(angle_delta < 0) * 2));
							entity->angle -= TRACKER_TURN_RATE * sign * dt;
						} else {
							acceleration_speed *=2;
							if (thruster) thruster->state = 1;
						}

						if (thruster && thruster->state == 1) {
							thruster->angle = entity->angle + 180;
							thruster->x = entity->x + cos_deg(thruster->angle);// * (entity->collision_radius + PARTICLE_MAX_START_RADIUS) / 2.0f;
							thruster->y = entity->y + sin_deg(thruster->angle);// * (entity->collision_radius + PARTICLE_MAX_START_RADIUS) / 2.0f;
						}

						entity->vx += cos_deg(entity->angle) * acceleration_speed * dt;
						entity->vy += sin_deg(entity->angle) * acceleration_speed * dt;
					}

				} break;
				
				case ENTITY_TYPE_ENEMY_TURRET:{
					Entity* target = get_enemy_target(game);
					if (target) {
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
							
							float shot_offset_angle = normalize_degrees(entity->angle - 90.0f);
							for (int i = 0; i < 2; i++) {
								Uint32 new_shot_id = spawn_entity(game, ENTITY_TYPE_BULLET, position);

								if (new_shot_id){
									Entity* new_shot = get_entity(game, new_shot_id);
									new_shot->x += cos_deg(shot_offset_angle) * TURRET_RADIUS;
									new_shot->y += sin_deg(shot_offset_angle) * TURRET_RADIUS;
									new_shot->velocity = velocity;
									new_shot->collision_radius = TURRET_SHOT_RADIUS;
									new_shot->timer = TURRET_SHOT_LIFE;

									shot_offset_angle = normalize_degrees(shot_offset_angle + 180.0f);
								}
							}

							
		
						
							entity->timer = TURRET_FIRE_ANIM_SPEED + TURRET_RECOVERY_ANIM_SPEED;
						}
					}

					// update fire animation
				} break;
				
				case ENTITY_TYPE_ENEMY_GRAPPLER:{
					Entity* target = get_enemy_target(game);
					if (target) {
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
					}

				} break;

				case ENTITY_TYPE_ITEM_LIFEUP: {
					entity->angle += dt;
				} break;

				case ENTITY_TYPE_ITEM_MISSILE: {
					entity->angle += dt;
				} break;

				case ENTITY_TYPE_SPAWN_WARP: {
					spawn_entity(game, entity->data.spawn_warp.spawn_type, entity->position);
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
					if (sc2d_check_circles(entity->position.x, entity->position.y, entity->collision_radius, 
											collision_entity->position.x, collision_entity->position.y, collision_entity->collision_radius,
											&overlap.x, &overlap.y)) {

						if (entity_is_item(entity->type) && collision_entity->type == ENTITY_TYPE_PLAYER) {
							entity->state = ENTITY_STATE_DESPAWNING;
							entity->timer = ENTITY_WARP_DELAY/2.0f;
						} else if (entity->type == ENTITY_TYPE_PLAYER && entity_is_item(collision_entity->type)) {
							collision_entity->state = ENTITY_STATE_DESPAWNING;
							collision_entity->timer = ENTITY_WARP_DELAY/2.0f;
						} if (entity->team && collision_entity->team && entity->team != collision_entity->team) {
							entity->state  = ENTITY_STATE_DYING;
							collision_entity->state = ENTITY_STATE_DYING;
						}

						// NOTE: Do we need any kind of physics response here?
						// entity ->vx -= overlap.x/2.0f;
						// entity ->vy -= overlap.y/2.0f;
						// collision_entity->vx += overlap.x/2.0f;
						// collision_entity->vy += overlap.y/2.0f;
					}
				}
			}

			for (int particle_index = 0; particle_index < game->particle_system.particle_count; particle_index++) {
				Particle* particle = game->particle_system.particles + particle_index;
				Vector2 overlap = {0};

				if ((particle->parent != entity_index) && 
					sc2d_check_circles(	entity->position.x, entity->position.y, entity->collision_radius, 
										particle->position.x, particle->position.y, particle->collision_radius,
										&overlap.x, &overlap.y)) {
					particle->x += overlap.x;
					particle->y += overlap.y;

					float magnitude = sqrtf( (particle->vx*particle->vx) + (particle->vy*particle->vy) );

					Vector2 new_v = {
						particle->vx + overlap.x,
						particle->vy + overlap.y
					};

					new_v = normalize_vector2(new_v);
					new_v.x *= magnitude;
					new_v.y *= magnitude;

					particle->vx = new_v.x;
					particle->vy = new_v.y;
				}
			}
		} // end of if (entity->state == ENTITY_STATE_ACTIVE)
	}

	if (dead_entity_count > 0) {
		Entity* dead_entity;
		for (int i = dead_entity_count-1; i >= 0 ; i--) {
			Uint32 dead_entity_index = dead_entities[i]; 
			dead_entity = game->entities + dead_entity_index;
			switch(dead_entity->type) {
				case ENTITY_TYPE_PLAYER: {
					remove_particle_emitter(&game->particle_system, dead_entity->data.player.main_thruster);
					remove_particle_emitter(&game->particle_system, dead_entity->data.player.left_thruster);
					remove_particle_emitter(&game->particle_system, dead_entity->data.player.right_thruster);
					game->player = 0;
					
					game->player_state.thrust_energy = THRUST_MAX;
					game->player_state.weapon_heat = 0;

					end_score_combo(&game->score);
				} break;

				case ENTITY_TYPE_ENEMY_TRACKER: {
					remove_particle_emitter(&game->particle_system, dead_entity->data.tracker.thruster);
				} break;

				case ENTITY_TYPE_ITEM_LIFEUP: {
					game->player_state.lives++;
				} break;
			}

			if (dead_entity->team == ENTITY_TEAM_ENEMY) {
				float value = get_entity_score_value(dead_entity->type);
				add_score(game, value);
				//random_item_spawn(game, dead_entity->position, value);
			}

			for (int sprite_index = 0; sprite_index < dead_entity->sprite_count; sprite_index++) {
				explode_sprite(game, dead_entity->sprites+sprite_index, dead_entity->x, dead_entity->y, dead_entity->angle, 6);
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

void draw_entities(Game_State* game) {
	Entity* entity = 0;
	for (int entity_index = 0; entity_index < game->entity_count; entity_index++) {
		entity = game->entities + entity_index;
		if (entity->state <= 0 || entity->state >= ENTITY_STATE_DYING) continue;

		if (entity->shape > PRIMITIVE_SHAPE_UNDEFINED && entity->shape < PRIMITIVE_SHAPE_COUNT) {
			switch (entity->shape) {

				case PRIMITIVE_SHAPE_CIRCLE: {
					float scaled_radius = entity->collision_radius * (entity->transform.scale.x + entity->transform.scale.y) / 2.0f;
					SDL_SetRenderDrawColor(game->renderer, entity->color.r, entity->color.g, entity->color.b, 255);
					render_fill_circlef(game->renderer, entity->x, entity->y, scaled_radius);
					//render_fill_circlef_linear_gradient(game->renderer, entity->x, entity->y, scaled_radius, entity->color, CLEAR_COLOR);	
				} break;
//				case PRIMITIVE_SHAPE_RECT: {
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
		
		if (entity->sprite_count > 0) {
			for (int sprite_index = 0; sprite_index < entity->sprite_count; sprite_index++) {
				render_draw_game_sprite(game, &entity->sprites[sprite_index], entity->transform, 1);	
			}
		}

#ifdef DEBUG
		SDL_SetRenderDrawColor(game->renderer, 255, 0, 0, 255);
		// NOTE: This function really likes to create a seg fault
		render_draw_circlef(game->renderer, entity->x, entity->y, entity->collision_radius);
#endif

	}
}