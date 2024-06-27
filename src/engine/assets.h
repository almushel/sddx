#ifndef GAME_ASSETS_H
#define GAME_ASSETS_H

#include "types.h"

#define declare_store_asset(type, table_name, func_suffix) void assets_store_##func_suffix(Game_Assets* assets, type* asset, const char* label)
#define define_store_asset(type, table_name, func_suffix) void assets_store_##func_suffix(Game_Assets* assets, type* asset, const char* label) {\
	type##_Node* node = &assets->table_name[get_hash_index(label, assets->table_name)];\
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
}

#define declare_get_asset(type, table_name, func_suffix) type* assets_get_##func_suffix(Game_Assets* assets, const char* name)
#define define_get_asset(type, table_name, func_suffix) type* assets_get_##func_suffix(Game_Assets* assets, const char* name) { \
	type* result = 0;\
	if (name == 0) return result;\
	type##_Node* node = &assets->table_name[get_hash_index(name, assets->table_name)];\
	while (node) { \
		if (node->name && (*node->name == *name) && SDL_strcmp(node->name, name) == 0) {\
			result = node->data;\
			break; \
		}\
		node = node->next;\
	}\
	return result; \
}

Game_Assets* new_game_assets(void);

SDL_bool assets_load_texture	(Game_Assets* assets, const char* file, const char* name);
SDL_bool assets_load_music	(Game_Assets* assets, const char* file, const char* name);
SDL_bool assets_load_sfx		(Game_Assets* assets, const char* file, const char* name);

declare_get_asset		(Mix_Music, music, music);
declare_get_asset		(Mix_Chunk, sfx, sfx);
declare_get_asset		(SDL_Texture, textures, texture);

declare_store_asset		(Mix_Music, music, music);
declare_store_asset		(Mix_Chunk, sfx, sfx);
declare_store_asset		(SDL_Texture, textures, texture);

Rectangle get_sprite_rect	(Game_Assets* assets, Game_Sprite* sprite);
Game_Sprite* divide_sprite	(Game_Assets* assets, Game_Sprite* sprite, int pieces);

STBTTF_Font* load_stbtt_font(const char* file_name, float font_size);

#endif
