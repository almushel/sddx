#include "SDL2/SDL.h"
#include "defs.h"

#define ENEMY_WARP_SPEED 26

#define DRIFT_RATE 1
#define DRIFT_RADIUS 40

#define UFO_SPEED 1.9
#define UFO_DIR_CHANGE_INTERVAL 120
#define UFO_COLLISION_RADIUS 20
#define UFO_TURN_PRECISION 0.05

#define TRACKER_ACCEL 0.13
#define TRACKER_FRICTION 0.02
#define TRACKER_TURN_RATE Math.PI/90
#define TRACKER_PRECISION 0.05
#define TRACKER_COLLISION_RADIUS 14

#define TURRET_RADIUS 15
#define TURRET_ACCEL 0.06
#define TURRET_SHOT_MAX 1
#define TURRET_SHOT_RADIUS 4
#define TURRET_SHOT_SPEED 3
#define TURRET_SHOT_LIFE 220
#define TURRET_AIM_TOLERANCE 15
#define TURRET_TURN_SPEED Math_PI / 300
#define TURRET_FIRE_ANIM_SPEED 5 //83ms
#define TURRET_RECOVERY_ANIM_SPEED 60 //500ms

#define GRAPPLER_AIM_TOLERANCE 0.2
#define GRAPPLER_TURN_SPEED Math_PI / 200
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
		result->type = type;
		result->position = position;

		switch(type) {
			case ENTITY_TYPE_PLAYER: {
				result->angle = 270;
				result->sprites[0].texture_name = "Player Ship";
				result->sprite_count = 1;
			} break;

			case ENTITY_TYPE_ENEMY_DRIFTER: {
				result->angle = random() * 360.0f;
				result->sprites[0].texture_name = "Enemy Drifter";
				result->sprite_count = 1;
			} break;

			case ENTITY_TYPE_ENEMY_UFO : {
				result->sprites[0].texture_name = "Enemy UFO";
				result->sprite_count = 1;
			} break;
			
			case ENTITY_TYPE_ENEMY_TRACKER : {
				result->sprites[0].texture_name = "Enemy Tracker";
				result->sprite_count = 1;
			} break;

			case ENTITY_TYPE_ENEMY_TURRET : {
				result->sprites[0].texture_name = "Enemy Turret Base";
				result->sprites[1].texture_name = "Enemy Turret Cannon";
				result->sprite_count = 2;
			} break;
			
			case ENTITY_TYPE_ENEMY_GRAPPLER : {
				result->sprites[0].texture_name = "Enemy Grappler";
				result->sprites[1].texture_name = "Grappler Hook";
				result->sprite_count = 2;
			} break;
		}
	}

	return result;
}


void update_entities(Game_State* game, float dt) {
	Entity* entity = 0;
	for (int i = 0; i < game->entity_count; i++) {
		entity = game->entities + i;
		
		if (entity->despawning) {
			
		} else {
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
				} break;

				case ENTITY_TYPE_ENEMY_DRIFTER:{ 
					entity->vx = cos_deg(entity->angle) * DRIFT_RATE;
					entity->vy = sin_deg(entity->angle) * DRIFT_RATE;
				} break;
				case ENTITY_TYPE_ENEMY_UFO:{ } break;
				case ENTITY_TYPE_ENEMY_TRACKER:{} break;
				case ENTITY_TYPE_ENEMY_TURRET:{} break;
				case ENTITY_TYPE_ENEMY_GRAPPLER:{} break;
			}
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

void draw_entities(Game_State* game) {
	Entity* entity = 0;
	for (int i = 0; i < game->entity_count; i++) {
		entity = game->entities + i;

		Game_Sprite entity_sprite = {0};

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
				SDL_RenderDrawRectF(game->renderer, &debug_frect);
			}
#endif
			draw_game_sprite(game, &entity->sprites[sprite_index], entity->transform, 1);	
		}
	}
}