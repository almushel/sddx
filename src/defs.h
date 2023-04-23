#ifndef DEFS_H
#define DEFS_H

#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"
#include "game_input.h"

typedef struct RGBA_Color {uint8_t r, g, b, a;} RGBA_Color;
#define CLEAR_COLOR (RGBA_Color){0, 10, 48 , 255}
#define SD_BLUE (RGBA_Color){109, 194, 255, 255}

#define array_length(array) ( sizeof(array) / sizeof(array[0]) )

#define TARGET_FPS 60
#define TICK_RATE 60

#define STAR_TWINKLE_INTERVAL 180.0f

enum stbi_masks {
	STBI_MASK_R = 0x000000FF, 
	STBI_MASK_G = 0x0000FF00,
	STBI_MASK_B = 0x00FF0000,
	STBI_MASK_A = 0xFF000000,
} stbi_masks;

typedef struct Vector2 {
	float x, y;
} Vector2;

#define Vec2_Union(v2_name, fx, fy) \
	union { 						\
		Vector2 v2_name; 			\
		struct {float fx, fy; }; 	\
	}

typedef struct Transform2D {
	Vec2_Union(position, x, y);
	Vec2_Union(scale, sx, sy);
	float angle;
} Transform2D;

#define Transform2D_Union								\
	union {												\
		Transform2D transform;							\
		struct {float x, y, sx, sy, angle; };			\
		struct {Vector2 position, scale; };				\
	}

#define MAX_POLY2D_VERTS 8
typedef struct Game_Poly2D {
	Vector2 vertices[MAX_POLY2D_VERTS];
	Uint32 vert_count;
} Game_Poly2D;

typedef struct Game_Sprite {
	char* texture_name;
	SDL_Rect src_rect;
	Vector2 offset;
	SDL_bool rotation_enabled;
} Game_Sprite;

typedef enum Game_Shape_Types {
	SHAPE_TYPE_UNDEFINED,
	SHAPE_TYPE_CIRCLE,
	SHAPE_TYPE_RECT,
	SHAPE_TYPE_POLY2D,
	SHAPE_TYPE_COUNT,
} Game_Shape_Types;

typedef struct Game_Shape {
	Game_Shape_Types type;
	union {
		Game_Poly2D polygon;
		SDL_FRect rectangle;
		struct { float radius;};
	};
} Game_Shape;

typedef struct Mix_Music_Node 	{ char* name; Mix_Music* data; 	 struct Mix_Music_Node* next; 	} Mix_Music_Node;
typedef struct Mix_Chunk_Node 	{ char* name; Mix_Chunk* data; 	 struct Mix_Chunk_Node* next; 	} Mix_Chunk_Node;
typedef struct SDL_Texture_Node { char* name; SDL_Texture* data; struct SDL_Texture_Node* next; } SDL_Texture_Node;

#include "stb/stb_truetype.h"
typedef struct STBTTF_Font {
	stbtt_fontinfo* info;
	stbtt_packedchar* chars;
	SDL_Texture* atlas;
	int texture_size;
	float size;
	float scale;
	int ascent;
	int baseline;
} STBTTF_Font;

typedef struct Game_Assets {
	Mix_Music_Node music[8];
	Mix_Chunk_Node sfx[16];
	SDL_Texture_Node textures[16];
} Game_Assets;

typedef struct Particle {
	Transform2D_Union;
	Vec2_Union(velocity, vx, vy);
	float mass;
	float timer;

	Game_Sprite sprite;
	Game_Shape shape;
	RGBA_Color color;
} Particle;

typedef struct Particle_Emitter {
	union {
		Vector2 position;
		struct { float x, y; };
	};
	float angle;
	float speed;

	float density;
//	Game_Sprite sprite;
	Game_Shape_Types shape;
	RGBA_Color colors[16];
	Uint32 color_count;

	float counter;
	float scale;
	enum Emitter_State {
		EMITTER_STATE_INACTIVE,
		EMITTER_STATE_ACTIVE,
		EMITTER_STATE_DEAD,
	} state;
} Particle_Emitter;

#define MAX_PARTICLES 512
#define MAX_PARTICLE_EMITTERS 128
typedef struct Particle_System {
	Particle particles[MAX_PARTICLES];
	Uint32 particle_count;

	Particle_Emitter emitters[MAX_PARTICLE_EMITTERS];
	Uint32 dead_emitters[MAX_PARTICLE_EMITTERS];
	Uint32 emitter_count;
	Uint32 dead_emitter_count;
} Particle_System;

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
	int current_wave;
	int spawn_points_max;

} Score_System;

typedef enum Player_Weapons {
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

typedef struct Game_State {
	STBTTF_Font* font;
	Game_Assets assets;
	Game_Input input;

	int world_w, world_h;

	Entity* entities;
	Uint32 entity_count;
	Uint32 entities_size;
	
	Game_Starfield starfield;
	Particle_System particle_system;
	Score_System score;

	Game_Player_Controller player_controller;
	Entity* player;
	struct {
		int lives;
		int ammo;
		float weapon_heat;
		float thrust_energy;
		
	} player_state;
} Game_State;

#endif