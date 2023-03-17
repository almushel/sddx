#include "defs.h"
#include "assets.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

// djb2 hash function
Uint64 str_hash(unsigned char* str) {
	Uint64 result = 5381;
	int c;

	while (c = *str++) {
		result = ((result << 5) + result) + c;
	}

	return result;
}

#define get_hash_index(name, table) str_hash((unsigned char*)name) % array_length(table)

static SDL_Texture* load_texture(SDL_Renderer* renderer, const char* file) {
	int image_width, image_height, image_components;
	SDL_Texture* result = 0;

	unsigned char* image = stbi_load(file, &image_width, &image_height, &image_components, 0);

	SDL_Surface* image_surface;
	if (image) {
		image_surface = SDL_CreateRGBSurfaceFrom(image, image_width, image_height, image_components * 8, image_components * image_width, 
							STBI_MASK_R, STBI_MASK_G, STBI_MASK_B, STBI_MASK_A);
	} else {
		SDL_Log("stbi failed to load %s", file);
	}

	if (image_surface) {
		result = SDL_CreateTextureFromSurface(renderer, image_surface);
	} else {
		SDL_Log("SDL failed to create surface from loaded image");
	}
	
	if (!result) {
		SDL_Log("SDL failed to create texture from surface");
	}

	if (image_surface) SDL_FreeSurface(image_surface);
	if (image) stbi_image_free(image);

	return result;
}

define_game_store_asset(SDL_Texture, textures, texture)
define_game_store_asset(Mix_Music, music, music)
define_game_store_asset(Mix_Chunk, sfx, sfx)

SDL_bool game_load_texture(Game_State* game, const char* file, const char* name) {
	SDL_bool result = 0;
	
	SDL_Texture* texture = load_texture(game->renderer, file);
	if (texture) {
		const char* label = (name && SDL_strlen(name)) ? name : file;
		game_store_texture(game, texture, label);
	}

	return result;
}

SDL_bool game_load_music(Game_State* game, const char* file, const char* name) {
	SDL_bool result = 0;

	Mix_Music* music = Mix_LoadMUS(file);
	if (music) {
		const char* label = (name && SDL_strlen(name)) ? name : file;
		game_store_music(game, music, label);
	}

	return result;
}

SDL_bool game_load_sfx(Game_State* game, const char* file, const char* name) {
	SDL_bool result = 0;

	Mix_Chunk* chunk = Mix_LoadWAV(file);
	if (chunk) {
		const char* label = (name && SDL_strlen(name)) ? name : file;
		game_store_sfx(game, chunk, label);
	}

	return result;
}

define_game_get_asset(Mix_Music, music, music)
define_game_get_asset(Mix_Chunk, sfx, sfx)
define_game_get_asset(SDL_Texture, textures, texture)

SDL_Rect get_sprite_rect(Game_State* game, Game_Sprite* sprite) {
	SDL_Rect result = {0};

	if (sprite->rect.w && sprite->rect.h) {
		result = sprite->rect;
	} else {
		SDL_Texture* texture = 0;
		if (sprite->texture_name) {
			texture = game_get_texture(game, sprite->texture_name);
		}
			
		if (texture) SDL_QueryTexture(texture, NULL, NULL, &result.w, &result.h);
		else SDL_Log("get_sprite_rect(): Invalid texture.");
	}

	return result;
}

// Currently allocates an array of Game_Sprites of length pieces.
// Should be freed after use.
Game_Sprite* divide_sprite(Game_State* game, Game_Sprite* sprite, int pieces) {
	Game_Sprite* result = 0;
	
	if (sprite && pieces > 0) {
		int columns = 2;
		int rows = pieces / columns;

		result = SDL_malloc(sizeof(Game_Sprite) * (pieces));
		
		SDL_Rect sprite_rect = get_sprite_rect(game, sprite);
		
		int chunk_width =  (int)(sprite_rect.w / columns);
		int chunk_height = (int)(sprite_rect.h / rows);

		int next_sprite = 0;
		for (int e = 0; e < rows; e++) {
			for (int i = 0; i < columns; i++) {
				Game_Sprite* chunk = result + next_sprite;

				chunk->texture_name = sprite->texture_name;
				chunk->rect.x  = chunk_width  * i;
				chunk->rect.y  = chunk_height * e;
				chunk->rect.w  = chunk_width;
				chunk->rect.h  = chunk_height;
			
				next_sprite++;
			}
		}
	}

	return result;
}