#include "../../engine/platform.h"
#include "../../engine/graphics.h"
#include "../entities.h"

#define ITEM_RADIUS 15

SDL_Texture* generate_item_texture(SDL_Texture* icon) {
	SDL_Texture* result = 0;
	
	int result_size = ITEM_RADIUS*2 + 4;

	result = platform_create_texture(result_size, result_size, SDL_TRUE);
	if (result) {
		platform_set_render_target(result);

		platform_set_render_draw_color((RGBA_Color){0});
		platform_render_clear();
		render_fill_circlef_linear_gradient((float)result_size/2.0f, (float)result_size/2.0f, ITEM_RADIUS, (RGBA_Color){0}, SD_BLUE);
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

static inline void init_item_missile(Entity* entity) {
	entity->team = ENTITY_TEAM_UNDEFINED;
	entity->shape.radius = ITEM_RADIUS;
	entity->sprites[0].texture_name = "Item Missile";
	entity->sprites[0].rotation_enabled = 1;
	entity->sprite_count = 1;
}

static inline void init_item_lifeup(Entity* entity){
	entity->team = ENTITY_TEAM_UNDEFINED;
	entity->shape.radius = ITEM_RADIUS;
	entity->sprites[0].texture_name = "Item LifeUp";
	entity->sprites[0].rotation_enabled = 1;
	entity->sprite_count = 1;
}

static inline void init_item_laser(Entity* entity){
	entity->team = ENTITY_TEAM_UNDEFINED;
	entity->shape.radius = ITEM_RADIUS;
	entity->sprites[0].texture_name = "Item Laser";
	entity->sprites[0].rotation_enabled = 1;
	entity->sprite_count = 1;
}
