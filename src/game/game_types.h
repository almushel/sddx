#ifndef DEFS_H
#define DEFS_H

#include "../engine/input.h"
#include "../engine/types.h"

#define CLEAR_COLOR (RGBA_Color){0, 10, 48 , 255}
#define SD_BLUE (RGBA_Color){109, 194, 255, 255}
#define WHITE (RGBA_Color){255,255,255,255}
#define BLACK (RGBA_Color){0,0,0,255}
#define RED (RGBA_Color){255, 0, 0, 255}

#define array_length(arr) (ptrdiff_t)(sizeof(arr) / sizeof(*(arr)))

#define TARGET_FPS 60
#define TICK_RATE 60

#define STARFIELD_STAR_COUNT 500
typedef struct Game_Starfield {
	Vector2 positions[STARFIELD_STAR_COUNT];
	float timers[STARFIELD_STAR_COUNT];
	RGBA_Color colors[STARFIELD_STAR_COUNT];
	SDL_bool twinkle_direction[STARFIELD_STAR_COUNT];
} Game_Starfield;

typedef struct Score_System {
	int total;
	int combo;
	int multiplier;
	float timer;

	float item_accumulator;
	// NOTE: current_wave and spawn_points_max are currently always the same value
	// Should this be changed to some kind of non-linear relationship or 
	// should points max just be removed?
	// (This is accurate to the original, but is dumb)
	int current_wave;
	int spawn_points_max;

} Score_System;

typedef enum Player_Weapons {
	PLAYER_WEAPON_UNDEFINED = 0,
	PLAYER_WEAPON_MG,
	PLAYER_WEAPON_MISSILE,
	PLAYER_WEAPON_LASER,
} Player_Weapons;

typedef struct Entity {
	Transform2D_Union;
	float z;
	float target_angle;
	Vec2_Union(velocity, vx, vy);
	float timer;

	Game_Sprite sprites[4];
	Uint32 sprite_count;
	Game_Shape shape;

	RGBA_Color color;

	Uint32 particle_emitters[3];
	Uint8 emitter_count;
	
	Uint8 type;
	Uint8 state;
	Uint8 team;
	Uint8 type_data;
} Entity;

typedef struct Entity_System Entity_System;

typedef enum Game_Scene {
	GAME_SCENE_MAIN_MENU,
	GAME_SCENE_GAMEPLAY,
	GAME_SCENE_GAME_OVER,
	GAME_SCENE_HIGH_SCORES,
} Game_Scene;

typedef struct Game_State {
	STBTTF_Font* font;
	Game_Assets* assets;
	Game_Input input;

	int world_w, world_h;
	SDL_bool fit_world_to_screen;

	Entity_System* entities;
	Uint32 enemy_count;

	Game_Starfield starfield;
	Particle_System* particle_system;
	Score_System score;

	Game_Player_Controller player_controller;
	Uint32 player;
	struct {
		int lives;
		int ammo;
		float weapon_heat;
		float thrust_energy;
	} player_state;
} Game_State;

#endif
