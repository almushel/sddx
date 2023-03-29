#ifndef ENTITY_SPAWN_H
#define ENTITY_SPAWN_H

float get_entity_score_value(Entity_Types type);
void spawn_wave(Game_State* game, int wave, int points_max);
void random_item_spawn(Game_State* game, Vector2 position, float accumulation);

#endif