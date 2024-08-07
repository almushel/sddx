#ifndef GAME_H
#define GAME_H

#include "game_types.h"

void init_game(Game_State* game);
int update_game(Game_State* game, Game_Input* input, float dt);
void draw_game_world(Game_State* game);
void draw_game_ui(Game_State* game);

#endif
