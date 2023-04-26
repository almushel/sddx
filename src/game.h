#ifndef GAME_H
#define GAME_H

#include "defs.h"

void init_game(Game_State* game);
void update_game(Game_State* game, float dt);
void draw_game_world(Game_State* game);
void draw_game_ui(Game_State* game);


#endif