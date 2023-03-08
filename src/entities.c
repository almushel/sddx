#include "SDL2/SDL.h"
#include "defs.h"

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
			draw_game_sprite(game, &entity->sprites[sprite_index], entity->transform, 1);	
		}
	}
}