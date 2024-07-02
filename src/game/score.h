#ifndef GAME_SCORE_H
#define GAME_SCORE_H

#include "game_types.h"

#define SCORE_COMBO_DECAY (5 * TICK_RATE)

void add_score		(Game_State* game, float score_value);
void update_score_timer	(Score_System* score, float dt);
void end_score_combo	(Score_System* score);

int* get_score_table();
void write_score_table(int* db);
int push_to_score_table(int new_score);

#endif
