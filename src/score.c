#include "SDL_rwops.h"
#include "defs.h"
#include "entities.h"
#include "score.h"

#define LIFE_UP_MILESTONE 5000
#define DB_FILE_NAME "scores.sddx"

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
	spawn_entity(game, ENTITY_TYPE_ITEM_LIFEUP, (Vector2){randomf() * game->world_w, randomf() * game->world_h});
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

int* get_score_table() {
    int* db = SDL_malloc(sizeof(int) * SCORE_TABLE_LENGTH);
    SDL_RWops* db_file = SDL_RWFromFile(DB_FILE_NAME, "rb");
    if (db_file == NULL) {
	for (int i = 0; i < SCORE_TABLE_LENGTH; i++) {
	    db[i] = (SCORE_TABLE_LENGTH-i)*1000;
	}
    } else {
	SDL_RWread(db_file, db, sizeof(int), SCORE_TABLE_LENGTH);
	SDL_RWclose(db_file);
    }

    return db;
}

void write_score_table(int* db) {
    SDL_RWops* db_file = SDL_RWFromFile(DB_FILE_NAME, "wb");
    if (db_file != NULL) {
	SDL_RWwrite(db_file, db, sizeof(int), SCORE_TABLE_LENGTH);
	SDL_RWclose(db_file);
    }
}

SDL_bool push_to_score_table(int new_score) {
    SDL_bool result = 0;
    int* db = get_score_table();
    
    for (int i = 0; i < SCORE_TABLE_LENGTH; i++) {
	if (new_score > db[i]) {
	    for (int e = SCORE_TABLE_LENGTH-1; e > i; e--) {
		db[e] = db[e-1];
	    }
	    db[i] = new_score;
	    result = 1;
	    break;
	}
    }

    write_score_table(db);
    SDL_free(db);
    return result;
}
