#ifndef GAME_ASSETS_H
#define GAME_ASSETS_H

#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"
#include "defs.h"

#define declare_game_store_asset(type, table_name, func_suffix) void game_store_##func_suffix(Game_State* game, type* asset, const char* label)
#define define_game_store_asset(type, table_name, func_suffix) void game_store_##func_suffix(Game_State* game, type* asset, const char* label) {\
	type##_Node* node = &game->assets.table_name[get_hash_index(label, game->assets.table_name)];\
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

#define declare_game_get_asset(type, table_name, func_suffix) type* game_get_##func_suffix(Game_State* game, const char* name)
#define define_game_get_asset(type, table_name, func_suffix) type* game_get_##func_suffix(Game_State* game, const char* name) { \
	type* result = 0;\
	if (name == 0) return result;\
	type##_Node* node = &game->assets.table_name[get_hash_index(name, game->assets.table_name)];\
	while (node) { \
		if (node->name && (*node->name == *name) && SDL_strcmp(node->name, name) == 0) {\
			result = node->data;\
			break; \
		}\
		node = node->next;\
	}\
	return result; \
}

SDL_bool game_load_texture	(Game_State* game, const char* file, const char* name);
SDL_bool game_load_music	(Game_State* game, const char* file, const char* name);
SDL_bool game_load_sfx		(Game_State* game, const char* file, const char* name);

declare_game_get_asset		(Mix_Music, music, music);
declare_game_get_asset		(Mix_Chunk, sfx, sfx);
declare_game_get_asset		(SDL_Texture, textures, texture);

declare_game_store_asset	(Mix_Music, music, music);
declare_game_store_asset	(Mix_Chunk, sfx, sfx);
declare_game_store_asset	(SDL_Texture, textures, texture);

Rectangle get_sprite_rect	(Game_State* game, Game_Sprite* sprite);
Game_Sprite* divide_sprite	(Game_State* game, Game_Sprite* sprite, int pieces);

STBTTF_Font* load_stbtt_font(const char* file_name, float font_size);

#endif
