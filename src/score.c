#include "defs.h"
#include "entities.h"
#include "score.h"


#define HIGH_SCORE_TABLE_LENGTH 10
#define LIFE_UP_MILESTONE 5000

static void update_score_multiplier(Score_System* score) {
	const int combo_score_multipliers[] = {10, 25, 50, 100};

    score->multiplier = 1;
	for (int i = array_length(combo_score_multipliers) - 1; i >= 0; i--) {
        if (score->combo >= combo_score_multipliers[i]) {
            score->multiplier = i + 2;
            break;
        }
    }
}

void add_score(Game_State* game, float score_value) {
    Score_System* score = &game->score;

    score->combo++;
    update_score_multiplier(score);

    int new_score = score->total + (int)(score_value * 100.0f * (float)score->multiplier);
    if (new_score / LIFE_UP_MILESTONE > score->total / LIFE_UP_MILESTONE) {
		spawn_entity(game, ENTITY_TYPE_ITEM_LIFEUP, (Vector2){random() * game->world_w, random() * game->world_h});
    }
    score->total = new_score;
    score->timer = SCORE_COMBO_DECAY;
}

void update_score_timer(Score_System* score, float dt) {
    score->timer -= dt;
    if (score->timer <= 0)  {
		score->combo = 0;
		update_score_multiplier(score);
	}
}

void end_score_combo(Score_System* score) {
    score->timer = 0;
    score->combo = 1;
}

/*
// TO-DO: Read from and write to text/binary score file
void update_score_table() {}
void reset_score_table() {}
*/