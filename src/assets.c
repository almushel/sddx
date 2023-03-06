#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"
#include "defs.h"

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

SDL_bool game_load_texture(Game_State* game, const char* file, const char* name) {
	SDL_bool result = 0;
	
	SDL_Texture* texture = load_texture(game->renderer, file);
	if (texture) {
		const char* label = (name && SDL_strlen(name)) ? name : file;

		SDL_Texture_Node* node = &game->assets.textures[get_hash_index(label, game->assets.textures)];
		while (node) {
			if (!node->data) {
				node->data = texture;
				node->name = (char*)label;
				result = 1;
			} else if (SDL_strcmp(node->name, label) == 0) {
				SDL_Log("game_load_texture(): Texture name %s already in use", label);
			} else if (!node->next) {
				SDL_Texture_Node* new_node = SDL_malloc(sizeof(SDL_Texture_Node));
				SDL_memset(new_node, 0, sizeof(SDL_Texture_Node));
				node->next = new_node;
				node = node->next;
			}

			node = node->next;
		}
	}

	return result;
}

SDL_bool game_load_music(Game_State* game, const char* file, const char* name) {
	SDL_bool result = 0;

	Mix_Music* music = Mix_LoadMUS(file);
	if (music) {
		const char* label = (name && SDL_strlen(name)) ? name : file;
		Mix_Music_Node* node = &game->assets.music[get_hash_index(label, game->assets.music)];
		while (node) {
			if (!node->data) {
				node->data = music;
				node->name = (char*)label;
				result = 1;
			} else if (SDL_strcmp(node->name, label) == 0) {
				SDL_Log("game_load_music(): Music track name %s already in use", label);
			} else if (!node->next) {
				Mix_Music_Node* new_node = SDL_malloc(sizeof(Mix_Music_Node));
				SDL_memset(new_node, 0, sizeof(Mix_Music_Node));
				node->next = new_node;
				node = node->next;
			}

			node = node->next;
		}
	}

	return result;
}

SDL_bool game_load_sfx(Game_State* game, const char* file, const char* name) {
	SDL_bool result = 0;

	Mix_Chunk* chunk = Mix_LoadWAV(file);
	if (chunk) {
		const char* label = (name && SDL_strlen(name)) ? name : file;
		Mix_Chunk_Node* node = &game->assets.sfx[get_hash_index(label, game->assets.sfx)];
		while (node) {
			if (!node->data) {
				node->data = chunk;
				node->name = (char*)label;
				result = 1;
			} else if (SDL_strcmp(node->name, label) == 0) {
				SDL_Log("game_load_sfx(): SFX track name %s already in use", label);
			} else if (!node->next) {
				Mix_Chunk_Node* new_node = SDL_malloc(sizeof(Mix_Chunk_Node));
				SDL_memset(new_node, 0, sizeof(Mix_Chunk_Node));
				node->next = new_node;
				node = node->next;
			}

			node = node->next;
		}
	}

	return result;
}

#define game_get_asset(type, table_name, func_suffix) type* game_get_##func_suffix##(Game_State* game, const char* name) { \
	type* result = 0;\
\
	type##_Node* node = &game->assets.##table_name##[get_hash_index(name, game->assets.##table_name)];\
	while (node) { \
		if (SDL_strcmp(node->name, name) == 0) {\
			result = node->data;\
			break; \
		}\
		node = node->next;\
	}\
	return result; \
}

game_get_asset(Mix_Music, music, music)
game_get_asset(Mix_Chunk, sfx, sfx)
game_get_asset(SDL_Texture, textures, texture)

SDL_Rect get_sprite_rect(Game_Sprite* sprite) {
	SDL_Rect result = {0};

	if (sprite->rect.w && sprite->rect.h) {
		result = sprite->rect;
	} else {
		if (sprite->texture) {
			SDL_QueryTexture(sprite->texture, NULL, NULL, &result.w, &result.h);
		} else {
			SDL_Log("get_sprite_rect(): Invalid texture.");
		}
	}

	return result;
}

// Currently allocates an array of Game_Sprites of length pieces.
// Should probably be freed after use.
Game_Sprite* divide_sprite(Game_Sprite* sprite, int pieces) {
	Game_Sprite* result = 0;
	
	if (sprite && pieces > 0) {
		int columns = 2;
		int rows = pieces / columns;

		result = SDL_malloc(sizeof(Game_Sprite) * (pieces));
		
		SDL_Rect sprite_rect = get_sprite_rect(sprite);
		
		int chunk_width =  (int)(sprite_rect.w / columns);
		int chunk_height = (int)(sprite_rect.h / rows);

		int next_sprite = 0;
		for (int e = 0; e < rows; e++) {
			for (int i = 0; i < columns; i++) {
				Game_Sprite* chunk = result + next_sprite;

				chunk->texture = sprite->texture;
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

void draw_game_sprite(Game_State* game, Game_Sprite* sprite, Transform2D transform, SDL_bool centered) {
	SDL_Texture* texture = sprite->texture;

	if (texture) {
		SDL_Rect sprite_rect = get_sprite_rect(sprite);

		SDL_FRect dest_rect;
		dest_rect.x = transform.x;
		dest_rect.y = transform.y;
		dest_rect.w = (float)sprite_rect.w;
		dest_rect.h = (float)sprite_rect.h;

		if (transform.sx > 0.0f && transform.sy > 0.0f) {
			dest_rect.w *= transform.sx;
			dest_rect.h *= transform.sy;
		}

		if (centered == SDL_TRUE) {
			dest_rect.x -= dest_rect.w/2.0f;
			dest_rect.y -= dest_rect.h/2.0f;
		}

		SDL_RenderCopyExF(game->renderer, texture, &sprite_rect, &dest_rect, transform.angle, 0, SDL_FLIP_NONE);
	}
}