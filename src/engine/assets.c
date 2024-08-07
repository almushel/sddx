#include "SDL_thread.h"
#include "platform.h"
#include "assets.h"

#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "external/stb_rect_pack.h"

#define STBTT_ifloor(x)   ((int) SDL_floor(x))
#define STBTT_iceil(x)    ((int) SDL_ceil(x))
#define STBTT_sqrt(x)      SDL_sqrt(x)
#define STBTT_pow(x,y)     SDL_pow(x,y)
#define STBTT_fmod(x,y)    SDL_fmod(x,y)
#define STBTT_cos(x)       SDL_cos(x)
#define STBTT_acos(x)      SDL_acos(x)
#define STBTT_fabs(x)      SDL_fabs(x)
#define STBTT_malloc(x,u)  ((void)(u),SDL_malloc(x))
#define STBTT_free(x,u)    ((void)(u),SDL_free(x))
#define STBTT_assert(x)    SDL_assert(x)
#define STBTT_strlen(x)    SDL_strlen(x)
#define STBTT_memcpy       SDL_memcpy
#define STBTT_memset       SDL_memset

#define STB_TRUETYPE_IMPLEMENTATION
#include "external/stb_truetype.h"

typedef struct Game_Assets {
	struct {
		Mix_Music_Node table[8];
		SDL_mutex* mutex;
	} music;
	struct {
		Mix_Chunk_Node table[16];
		SDL_mutex* mutex;
	} sfx;
	struct {
		SDL_Texture_Node table[16];
		SDL_mutex* mutex;
	} textures;
} Game_Assets;

#define define_store_asset(type, table_name, func_suffix) void assets_store_##func_suffix(Game_Assets* assets, type* asset, const char* label) {\
	type##_Node* node = &assets->table_name.table[get_hash_index(label, assets->table_name.table)];\
	SDL_LockMutex(assets->table_name.mutex);\
	while (node) {\
		if (!node->data) {\
			node->data = asset;\
			node->name = (char*)label;\
		} else if (*node->name == *label && SDL_strcmp(node->name, label) == 0) {\
			SDL_Log("Asset name %s already in use", label);\
		} else if (!node->next) {\
			type##_Node* new_node = SDL_malloc(sizeof(type##_Node));\
			SDL_memset(new_node, 0, sizeof(type##_Node));\
			node->next = new_node;\
		}\
		node = node->next;\
	}\
	SDL_UnlockMutex(assets->table_name.mutex);\
}

#define define_get_asset(type, table_name, func_suffix) type* assets_get_##func_suffix(Game_Assets* assets, const char* name) { \
	type* result = 0;\
	if (name == 0 || name[0] == '\0') return result;\
	type##_Node* node = &assets->table_name.table[get_hash_index(name, assets->table_name.table)];\
	while (node) { \
		if (node->name && (*node->name == *name) && SDL_strcmp(node->name, name) == 0) {\
			result = node->data;\
			break; \
		}\
		node = node->next;\
	}\
	return result; \
}

double pow(double x, double y) {
	double result = x;

	if (y == 0) {
		result = 1;
	} else {
		int power = SDL_abs(y);
		for (int i = 1; i < power; i++) {
			result *= result;
		}
	}

	if (y < 0) result = 1.0/result;

	return result;
}

STBTTF_Font* load_stbtt_font(const char* file_name, float font_size) {
	STBTTF_Font* result = 0;

	SDL_RWops *file = SDL_RWFromFile(file_name, "rb");
	Sint64 file_size = 0;
	if (file) file_size = SDL_RWsize(file);
	
	if (file_size) {
		unsigned char* file_buffer = SDL_malloc(file_size);
		if (SDL_RWread(file, file_buffer, file_size, 1) != 1) return 0;
		SDL_RWclose(file);

		result = SDL_calloc(sizeof(STBTTF_Font), 1);
		result->info = SDL_malloc(sizeof(stbtt_fontinfo));
		result->chars = SDL_malloc(sizeof(stbtt_packedchar) * 96);
		result->size = font_size;
	
		if (stbtt_InitFont(result->info, file_buffer, 0) == 0) {
			SDL_free(file_buffer);
			SDL_free(result->info);
			SDL_free(result->chars);
			SDL_free(result);

			result = 0;
			return result;
		}

		unsigned char* bitmap = 0;
		result->texture_size = 32;

		while(1) {
			bitmap = SDL_malloc(result->texture_size * result->texture_size);
			stbtt_pack_context pack_context;
			stbtt_PackBegin(&pack_context, bitmap, result->texture_size, result->texture_size, 0, 1, 0);
			stbtt_PackSetOversampling(&pack_context, 1, 1);
			if (!stbtt_PackFontRange(&pack_context, file_buffer, 0, font_size, 32, 95, result->chars)) {
				SDL_free(bitmap);
				stbtt_PackEnd(&pack_context);
				result->texture_size *= 2;
			} else {
				stbtt_PackEnd(&pack_context);
				break;
			}
		}

		result->atlas = platform_create_texture(result->texture_size, result->texture_size, false);

		Uint32* pixels = SDL_malloc(result->texture_size * result->texture_size * sizeof(Uint32));
		static SDL_PixelFormat* format = 0;
		if(format == 0) format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
		for (int i = 0; i < result->texture_size * result->texture_size; i++) {
			pixels[i] = SDL_MapRGBA(format, 0xff, 0xff, 0xff, bitmap[i]);
		}
		SDL_UpdateTexture(result->atlas, 0, pixels, result->texture_size * sizeof(Uint32));
		
		SDL_free(pixels);
		SDL_free(bitmap);

		result->scale = stbtt_ScaleForPixelHeight(result->info, font_size);
		stbtt_GetFontVMetrics(result->info, &result->ascent, 0, 0);
		result->baseline = (int) (result->ascent * result->scale);
	
		SDL_free(file_buffer);
	}

	return result;
}

// djb2 hash function
Uint64 str_hash(unsigned char* str) {
	Uint64 result = 5381;
	int c;

	while ( (c=*str++) ) {
		result = ((result << 5) + result) + c;
	}

	return result;
}

#define get_hash_index(name, table) str_hash((unsigned char*)name) % array_length(table)

static SDL_Texture* load_texture(const char* file) {
	size_t file_size;
	int image_width, image_height, image_components;
	SDL_Texture* result = 0;
	
	stbi_uc* buf = (stbi_uc*)SDL_LoadFile(file, &file_size);
	unsigned char* image = stbi_load_from_memory(buf, file_size, &image_width, &image_height, &image_components, 0);

	SDL_Surface* image_surface;
	if (image) {
		image_surface = SDL_CreateRGBSurfaceFrom(image, image_width, image_height, image_components * 8, image_components * image_width, 
		STBI_MASK_R, STBI_MASK_G, STBI_MASK_B, STBI_MASK_A);
	} else {
		SDL_Log("stbi failed to load %s", file);
	}

	if (image_surface) {
		result = platform_create_texture_from_surface(image_surface);
	} else {
		SDL_Log("SDL failed to create surface from loaded image");
	}
	
	if (!result) {
		SDL_Log("SDL failed to create texture from surface");
	}

	if (image_surface) SDL_FreeSurface(image_surface);
	if (image) stbi_image_free(image);
	if (buf) SDL_free(buf);

	return result;
}

SDL_Surface * create_surface_from_image_file(const char* file) {
	size_t file_size;
	int image_width, image_height, image_components;
	
	stbi_uc* buf = (stbi_uc*)SDL_LoadFile(file, &file_size);
	unsigned char* image = stbi_load_from_memory(buf, file_size, &image_width, &image_height, &image_components, 0);

	SDL_Surface* result;
	if (image) {
		result = SDL_CreateRGBSurfaceFrom(image, image_width, image_height, image_components * 8, image_components * image_width, 
		STBI_MASK_R, STBI_MASK_G, STBI_MASK_B, STBI_MASK_A);
	} else {
		SDL_Log("stbi failed to load %s", file);
	}
	
	if (!result) {
		SDL_Log("SDL failed to create Surface from sbti buffer");
	}

	if (image) stbi_image_free(image);
	if (buf) SDL_free(buf);

	return result;
}

Game_Assets* new_game_assets(void) {
	Game_Assets* result = SDL_calloc(1, sizeof(Game_Assets));
	result->music.mutex = SDL_CreateMutex();
	result->textures.mutex = SDL_CreateMutex();
	result->sfx.mutex = SDL_CreateMutex();

	return result;
}

define_store_asset(SDL_Texture, textures, texture)
define_store_asset(Mix_Music, music, music)
define_store_asset(Mix_Chunk, sfx, sfx)

typedef struct asset_load_data {
	Game_Assets* assets;
	char* file;
	char* name;
} asset_load_data;

// NOTE: load_texture does not play nice with concurrency.
// This appears to be because the SDL renderer API is intended
// to be called only from the main thread.
// Possible solution could be to create surfaces in
// concurrent threads then finish loading textures in main.
SDL_bool assets_load_texture(Game_Assets* assets, const char* file, const char* name) {
	SDL_Texture* texture = load_texture(file);
	if (!texture) {
		return 0;
	}

	const char* label = (name && name[0] != '\0') ? name : file;
	assets_store_texture(assets, texture, label);

	return 1;
}

static int SDLCALL thread_load_music(void* _data) {
	asset_load_data* data = (asset_load_data*)_data;

	Mix_Music* music = Mix_LoadMUS(data->file);
	if (music) {
		const char* label = (data->name && data->name[0] != '\0') ? data->name : data->file;
		assets_store_music(data->assets, music, label);
	}

	free(_data);

	return 1;
}

SDL_bool assets_load_music(Game_Assets* assets, const char* file, const char* name) {
	SDL_bool result = 0;

	asset_load_data* _data = malloc(sizeof(asset_load_data));
	*_data = (asset_load_data){assets, (char*)file, (char*)name};

	SDL_Thread* thread = SDL_CreateThread(thread_load_music, file, _data);
	SDL_DetachThread(thread);

	return result;
}

static int SDLCALL thread_load_sfx(void* _data) {
	asset_load_data* data = (asset_load_data*)_data;

	Mix_Chunk* chunk = Mix_LoadWAV(data->file);
	if (chunk) {
		const char* label = (data->name && data->name[0] != '\0') ? data->name : data->file;
		assets_store_sfx(data->assets, chunk, label);
	}

	free(_data);

	return 1;
}

SDL_bool assets_load_sfx(Game_Assets* assets, const char* file, const char* name) {
	SDL_bool result = 0;

	asset_load_data* _data = malloc(sizeof(asset_load_data));
	*_data = (asset_load_data){assets, (char*)file, (char*)name};

	SDL_Thread* thread = SDL_CreateThread(thread_load_sfx, file, _data);
	SDL_DetachThread(thread);

	return result;
}

define_get_asset(Mix_Music, music, music)
define_get_asset(Mix_Chunk, sfx, sfx)
define_get_asset(SDL_Texture, textures, texture)

Rectangle get_sprite_rect(Game_Assets* assets, Game_Sprite* sprite) {
	Rectangle result = {0};

	if (sprite->src_rect.w && sprite->src_rect.h) {
		result = sprite->src_rect;
	} else {
		SDL_Texture* texture = 0;
		if (sprite->texture_name) {
			texture = assets_get_texture(assets, sprite->texture_name);
		}
		
		if (texture) {
			int w,h;
			SDL_QueryTexture(texture, NULL, NULL, &w, &h);
			result.w = (float)w;
			result.h = (float)h;
		}
		else SDL_Log("get_sprite_rect(): Invalid texture.");
	}

	return result;
}

// Currently allocates an array of Game_Sprites of length pieces.
// Should be freed after use.
Game_Sprite* divide_sprite(Game_Assets* assets, Game_Sprite* sprite, int pieces) {
	Game_Sprite* result = 0;
	
	if (sprite && pieces > 0) {
		int columns = 2;
		int rows = pieces / columns;

		result = SDL_malloc(sizeof(Game_Sprite) * (pieces));
		
		Rectangle sprite_rect = get_sprite_rect(assets, sprite);
		
		int chunk_width =  (int)(sprite_rect.w / columns);
		int chunk_height = (int)(sprite_rect.h / rows);

		int next_sprite = 0;
		for (int e = 0; e < rows; e++) {
			for (int i = 0; i < columns; i++) {
				Game_Sprite* chunk = result + next_sprite;

				chunk->texture_name = sprite->texture_name;
				chunk->src_rect.x  = chunk_width  * i;
				chunk->src_rect.y  = chunk_height * e;
				chunk->src_rect.w  = chunk_width;
				chunk->src_rect.h  = chunk_height;
				chunk->offset.x = chunk->offset.y = 0;
			
				next_sprite++;
			}
		}
	}

	return result;
}
