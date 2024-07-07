#include "SDL_render.h"
#include "SDL_surface.h"

#include "../../engine/platform.h"
#include "../../engine/graphics.h"
#include "../../engine/math.h"
#include "../entities.h"

#define ITEM_RADIUS 15


SDL_Texture* generate_item_texture(SDL_Texture* icon) {
	SDL_Texture* result = 0;
	
	int result_size = ITEM_RADIUS*2 + 4;

	SDL_Texture* target = platform_create_texture(result_size, result_size, SDL_TRUE);
	if (target) {
		platform_set_render_target(target);

		platform_set_render_draw_color((RGBA_Color){0});
		platform_render_clear();
		render_fill_circlef_linear_gradient((float)result_size/2.0f, (float)result_size/2.0f, ITEM_RADIUS, (RGBA_Color){0}, SD_BLUE);
		if (icon) {
			Vector2 dim = platform_get_texture_dimensions(icon);
			float larger_dim = (dim.x > dim.y) ? dim.x : dim.y;
			float ratio = ((float)ITEM_RADIUS * 1.8f)/larger_dim;
			
			Rectangle dest = {0};
			dest.w = dim.x * ratio;
			dest.h = dim.y * ratio;
			dest.x = ((float)result_size-dest.w) / 2.0f;
			dest.y = ((float)result_size-dest.h) / 2.0f;
			
			platform_render_copy(icon, NULL, &dest, 0, 0, SDL_FLIP_NONE);
		}

		// NOTE: Storing as static access texture so it will be recreated
		// on renderer reset (e.g. window resize) and not destroyed.
		Uint32 format;
		int width,height;
		SDL_QueryTexture(target, &format, 0, &width, &height);
		SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, format);
		platform_render_read_pixels(0, format, surface->pixels, surface->pitch);

		result = platform_create_texture_from_surface(surface);
		
		platform_set_render_target(0);
		SDL_DestroyTexture(target);
		SDL_free(surface);
	}

	return result;
}

SDL_Texture* generate_laser_item_texture(void) {
	SDL_Texture* result = 0;
	
	int result_size = ITEM_RADIUS*2;
	Rectangle laser_rect = {
		.x = ITEM_RADIUS/2.0f+1.0f, .y = -2.5f,
		.w = ITEM_RADIUS, .h = 5.0f,
	};

	SDL_Texture* icon = platform_create_texture(result_size, result_size, SDL_TRUE);
	if (icon) {
		platform_set_render_target(icon);
		platform_set_render_draw_color((RGBA_Color){0});
		platform_render_clear();

		platform_set_render_draw_color(SD_BLUE);
		// NOTE: Slightly different offsets to correctly center, likely due to subpixel rounding(?)
		platform_render_fill_rect(
			translate_rect(laser_rect, (Vector2){0,(float)ITEM_RADIUS-4.0f})
		);
		platform_render_fill_rect(
			translate_rect(laser_rect, (Vector2){0,(float)ITEM_RADIUS+5.5f})
		);
	}

	result = generate_item_texture(icon);
	SDL_DestroyTexture(icon);

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
