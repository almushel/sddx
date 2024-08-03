#ifndef GAME_ASSETS_H
#define GAME_ASSETS_H

#include "types.h"

#define declare_store_asset(type, func_suffix) void assets_store_##func_suffix(Game_Assets* assets, type* asset, const char* label)
#define declare_get_asset(type, func_suffix) type* assets_get_##func_suffix(Game_Assets* assets, const char* name)

Game_Assets* new_game_assets(void);

SDL_bool assets_load_texture	(Game_Assets* assets, const char* file, const char* name);
SDL_bool assets_load_music	(Game_Assets* assets, const char* file, const char* name);
SDL_bool assets_load_sfx	(Game_Assets* assets, const char* file, const char* name);

declare_get_asset		(Mix_Music, music);
declare_get_asset		(Mix_Chunk, sfx);
declare_get_asset		(SDL_Texture, texture);

declare_store_asset		(Mix_Music, music);
declare_store_asset		(Mix_Chunk, sfx);
declare_store_asset		(SDL_Texture, texture);

Rectangle get_sprite_rect	(Game_Assets* assets, Game_Sprite* sprite);
Game_Sprite* divide_sprite	(Game_Assets* assets, Game_Sprite* sprite, int pieces);

STBTTF_Font* load_stbtt_font	(const char* file_name, float font_size);

#endif
