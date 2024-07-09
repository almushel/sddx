#include "../game_types.h"
#include "../entities.h"
#include "../score.h"
#include "../../engine/assets.h"
#include "../../engine/math.h"
#include "../../engine/particles.h"

#define DEMO_DIR_CHANGE 360.0f

#define PLAYER_FORWARD_THRUST 0.15f
#define PLAYER_LATERAL_THRUST 0.2f
#define PLAYER_TURN_SPEED 3.14f

#define PLAYER_SHIP_RADIUS 13

#define PLAYER_THRUST_PARTICLE_SPEED 3.0f
#define PLAYER_THRUST_MAX 100
#define PLAYER_THRUST_CONSUMPTION 0.45f
#define PLAYER_WEAPON_HEAT_MAX 100


#define PLAYER_MG_COOLDOWN 200.0f/16.666f
#define PLAYER_LASER_COOLDOWN 250.0f/16.666f
#define PLAYER_MISSILE_COOLDOWN 800.0f/16.666f
#define PLAYER_MG_HEAT 25.0f
#define PLAYER_LASER_HEAT 35.0f
#define PLAYER_MISSILE_HEAT 45.0f

static inline void init_player(Particle_System* ps, Entity* entity) {
	entity->team = ENTITY_TEAM_PLAYER;
	entity->shape.radius = PLAYER_SHIP_RADIUS;
	entity->angle = 270;
	entity->sprites[0].texture_name = "Player Ship";
	entity->sprites[0].rotation_enabled = 1;
	entity->sprite_count = 1;
	entity->emitter_count = 3;

	for (int i = 0; i < entity->emitter_count; i++) {
		entity->particle_emitters[i] = get_new_particle_emitter(ps);
		Particle_Emitter* thruster = get_particle_emitter(ps, entity->particle_emitters[i]);
		if (thruster) {
			thruster->shape = SHAPE_TYPE_RECT;
			thruster->speed = PLAYER_THRUST_PARTICLE_SPEED;
			thruster->density = 2.0f;
			thruster->colors[0] = SD_BLUE;
			thruster->color_count = 1;
		}
	}
	entity->flags = ENTITY_FLAG_EXPLOSION_ENABLED;
}

static inline void destroy_player(Game_State* game) {
	Mix_PlayChannel(-1, assets_get_sfx(game->assets, "Player Death"), 0);
	
	game->player = 0;
	game->player_state.thrust_energy = PLAYER_THRUST_MAX;
	game->player_state.weapon_heat = 0;
						
	end_score_combo(&game->score);
}

static inline void update_spawning_player(Game_State* game, Entity* entity, float dt) {
	Particle_System* ps = game->particle_system;
	float t = (entity->y - game->world_h) / (game->world_h/2.0f - game->world_h);
	float ts = sin_deg(t * 90.0f);

	Particle_Emitter* thruster;
	thruster = get_particle_emitter(ps, entity->particle_emitters[0]);
	if (thruster) {
		thruster->state = (entity->sx > 0.4f && entity->sx < 0.9f);
		thruster->position = entity->position;
		thruster->angle = normalize_degrees(entity->angle + 180.0f);
	}

	thruster = get_particle_emitter(ps, entity->particle_emitters[1]);
	if (thruster) {
		thruster->state = (entity->sx > 0.5f);
		thruster->position = entity->position;
		thruster->angle = normalize_degrees(entity->angle - 45);
	}

	thruster = get_particle_emitter(ps, entity->particle_emitters[2]);
	if (thruster) {
		thruster->state = (entity->sx > 0.5f);
		thruster->position = entity->position;
		thruster->angle = normalize_degrees(entity->angle + 45);
	}

	force_circle(game->entities, entity->x, entity->y, entity->sx * PLAYER_SHIP_RADIUS * 3.0f, 1.5f);
	entity->y -= (8.0f - lerp(0.0f, 6.0f, t)) * dt;
	entity->sx = entity->sy = t;
	entity->shape.radius = ts * PLAYER_SHIP_RADIUS;

	if (entity->y <= game->world_h/2.0f) {
		entity->state = ENTITY_STATE_ACTIVE;
		entity->scale = (Vector2){1.0f, 1.0f};
		entity->shape.radius = PLAYER_SHIP_RADIUS;// * 2.0f;
		entity->velocity = (Vector2){0};
		entity->timer = 0;
	}
}

static inline void update_player_entity(Game_State* game, Entity* entity, float dt) {
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
		Particle_Emitter* thruster = get_particle_emitter(game->particle_system, entity->particle_emitters[i]);
		if (thruster == 0) { continue; }

		thruster->state = EMITTER_STATE_INACTIVE;
		if (i < array_length(thrust_inputs) && thrust_inputs[i]) {
			thrusting = 1;
			if (game->player_state.thrust_energy > 0) {
				float thrust_speed = (i > 0) ? PLAYER_LATERAL_THRUST : PLAYER_FORWARD_THRUST;

				thruster->state = EMITTER_STATE_ACTIVE;
				entity->vx += cos_deg(entity->angle + angle_offset) * thrust_speed * dt;
				entity->vy += sin_deg(entity->angle + angle_offset) * thrust_speed * dt;
				game->player_state.thrust_energy -= PLAYER_THRUST_CONSUMPTION *dt;
			}
		}

		if (thruster->state == EMITTER_STATE_ACTIVE) {
			thruster->angle = entity->angle + angle_offset + 180;
			thruster->y = entity->y + sin_deg(thruster->angle) * (float)PLAYER_SHIP_RADIUS;
			thruster->x = entity->x + cos_deg(thruster->angle) * (float)PLAYER_SHIP_RADIUS;
		}

		angle_offset -= 90.0f * (float)(i+1);
	}
	game->player_state.thrust_energy = SDL_clamp(game->player_state.thrust_energy + (float)(int)(!thrusting) * dt, 0, PLAYER_THRUST_MAX);

	if (is_game_control_held(&game->input, &controller.fire)) {	
		if (game->player_state.weapon_heat < PLAYER_WEAPON_HEAT_MAX) {
			game->player_state.weapon_heat -= dt;
		
			if (entity->timer <= 0)  {
				switch(entity->type_data) {

					case PLAYER_WEAPON_MG: {
						game->player_state.weapon_heat += PLAYER_MG_HEAT;
						Mix_PlayChannel(-1, assets_get_sfx(game->assets, "Player Shot"), 0);
						Uint32 bullet_id = spawn_entity(game->entities, game->particle_system, ENTITY_TYPE_BULLET, entity->position);
						Entity* bullet = get_entity(game->entities, bullet_id);
						if (bullet == NULL) { break; }

						bullet->timer = PLAYER_SHOT_LIFE;
						Vector2 angle = {
							cos_deg(entity->angle),
							sin_deg(entity->angle)
						};
						
						bullet->x += angle.x * PLAYER_SHIP_RADIUS;
						bullet->y += angle.y * PLAYER_SHIP_RADIUS;
						
						bullet->vx = entity->vx + (angle.x * PLAYER_SHOT_SPEED);
						bullet->vy = entity->vy + (angle.y * PLAYER_SHOT_SPEED);
						bullet->color = SD_BLUE;
						bullet->team = ENTITY_TEAM_PLAYER;
						
						entity->timer = PLAYER_MG_COOLDOWN;
					} break;
					
					case PLAYER_WEAPON_MISSILE: {
						if (game->player_state.ammo <= 0) { 
							entity->type_data = PLAYER_WEAPON_MG;
							break;	
						}
						Mix_PlayChannel(-1, assets_get_sfx(game->assets, "Player Missile"), 0);

						game->player_state.weapon_heat += PLAYER_MISSILE_HEAT;

						float position_offset = -60.0f;
						Vector2 angle = { cos_deg(entity->angle), sin_deg(entity->angle) };
						
						for (int i = 0; i < 2; i++) {
							Uint32 missile_id = spawn_entity(game->entities, game->particle_system, ENTITY_TYPE_MISSILE, entity->position);
							Entity* missile = get_entity(game->entities, missile_id);
							if (missile == NULL) { break; }
							
							missile->timer /= 2.0f;

							missile->x += cos_deg(entity->angle + position_offset) * PLAYER_SHIP_RADIUS;
							missile->y += sin_deg(entity->angle + position_offset) * PLAYER_SHIP_RADIUS;
							
							missile->angle = entity->angle;
							missile->vx = entity->vx + angle.x;
							missile->vy = entity->vy + angle.y;
							missile->team = ENTITY_TEAM_PLAYER;
						
							position_offset *= -1;
						}

						game->player_state.ammo--;
						entity->timer = PLAYER_MISSILE_COOLDOWN;

					} break;
					
					case PLAYER_WEAPON_LASER: {
						if (game->player_state.ammo <= 0) { 
							entity->type_data = PLAYER_WEAPON_MG;
							break;	
						}
						Mix_PlayChannel(-1, assets_get_sfx(game->assets, "Player Laser"), 0);

						game->player_state.weapon_heat += PLAYER_LASER_HEAT;

						float position_offset = -60.0f;
						Vector2 angle = { cos_deg(entity->angle), sin_deg(entity->angle) };
						
						for (int i = 0; i < 2; i++) {
							Uint32 laser_id = spawn_entity(game->entities, game->particle_system, ENTITY_TYPE_LASER, entity->position);
							Entity* laser = get_entity(game->entities, laser_id);
							if (laser == NULL) { break; }

							laser->state = ENTITY_STATE_ACTIVE;
							laser->timer = PLAYER_SHOT_LIFE;
							
							laser->x += cos_deg(entity->angle + position_offset) * PLAYER_SHIP_RADIUS;
							laser->y += sin_deg(entity->angle + position_offset) * PLAYER_SHIP_RADIUS;
							
							laser->angle = entity->angle;
							laser->vx = entity->vx + angle.x * PLAYER_SHOT_SPEED * 2.0f;
							laser->vy = entity->vy + angle.y * PLAYER_SHOT_SPEED * 2.0f;
							laser->team = ENTITY_TEAM_PLAYER;
						
							position_offset *= -1;
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

	game->player_state.weapon_heat = SDL_clamp(game->player_state.weapon_heat, 0, PLAYER_WEAPON_HEAT_MAX);
}

static inline void update_demo_ship(Game_State* game, Entity* entity, float dt) {
	Particle_System* ps = game->particle_system;
	float w = game->world_w / 2.0f;
	float h = game->world_h / 2.0f;
	Vector2 delta = {
		entity->x - w,
		entity->y - h,
	};
	float dist = SDL_sqrtf(delta.x*delta.x + delta.y*delta.y);
	float vert = 1.2f - dist / SDL_sqrtf(w*w+h*h);

	entity->transform.scale.x = entity->transform.scale.y = vert;
	entity->shape.radius = 0;//vert * PLAYER_SHIP_RADIUS * 2;

	if (entity->timer <= 0.0f) {
		entity->type_data = (uint8_t)(!(SDL_bool)entity->type_data);
		entity->timer = DEMO_DIR_CHANGE;
	}

	entity->angle += (1 + ((float)entity->type_data * -2.0f)) * dt;

	float v2 = vert*vert;
	entity->vx += cos_deg(entity->angle) * v2 * dt;
	entity->vy += sin_deg(entity->angle) * v2 * dt;

	entity->vx *= 1.0f - 0.15f * dt;
	entity->vy *= 1.0f - 0.15f * dt;

	Particle_Emitter* thruster = get_particle_emitter(ps, entity->particle_emitters[0]);	
	if (thruster) {
		thruster->state = EMITTER_STATE_ACTIVE * (vert > 0.2);
		thruster->position = entity->position;
		thruster->speed = PLAYER_THRUST_PARTICLE_SPEED * vert;
		thruster->scale.x = thruster->scale.y = vert;
		thruster->angle = entity->angle - 180.0f;
		thruster->position = (Vector2) {
			entity->position.x + cos_deg(thruster->angle) * 16.0f * vert,
			entity->position.y + sin_deg(thruster->angle) * 16.0f * vert,
		};
	}
}
