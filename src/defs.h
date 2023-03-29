#ifndef DEFS_H
#define DEFS_H

#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"

#define MATH_PI 3.14159265
#define RAD_TO_DEG(rads) rads * (180/MATH_PI)
#define DEG_TO_RAD(degs) degs * (MATH_PI/180)

typedef struct RGB_Color {uint8_t r, g, b;} RGB_Color;
#define CLEAR_COLOR (RGB_Color){0,10,48}
#define SD_BLUE (RGB_Color){109, 194, 255}

#define array_length(array) ( sizeof(array) / sizeof(array[0]) )

#define TARGET_FPS 60
#define TICK_RATE 60

#define PHYSICS_FRICTION 0.02f
#define PLAYER_FORWARD_THRUST 0.15f
#define PLAYER_LATERAL_THRUST 0.2f
#define PLAYER_TURN_SPEED 3.14f

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

typedef struct Game_Poly2D {
	Vector2 vertices[8];
	Uint32 vert_count;
} Game_Poly2D;

typedef struct Game_Sprite {
	char* texture_name;
	SDL_Rect rect;
	SDL_bool rotation;
} Game_Sprite;

typedef enum Primitive_Shapes {
	PRIMITIVE_SHAPE_UNDEFINED,
	PRIMITIVE_SHAPE_CIRCLE,
	PRIMITIVE_SHAPE_RECT,
	PRIMITIVE_SHAPE_COUNT,
} Primitive_Shapes;

typedef struct Mix_Music_Node 	{ char* name; Mix_Music* data; struct Mix_Music_Node* next; } Mix_Music_Node;
typedef struct Mix_Chunk_Node 	{ char* name; Mix_Chunk* data; struct Mix_Chunk_Node* next; } Mix_Chunk_Node;
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

typedef struct Game_Button_State {
	SDL_Scancode scan_code;
	SDL_bool pressed, held, released;
} Game_Button_State;

typedef union Game_Controller_State {
	struct {
		Game_Button_State thrust;
		Game_Button_State turn_left;
		Game_Button_State turn_right;
		Game_Button_State thrust_left;
		Game_Button_State thrust_right;
		Game_Button_State fire;
	};
	Game_Button_State list[6];
} Game_Controller_State;

typedef struct Particle {
	Transform2D_Union;
	Vec2_Union(velocity, vx, vy);
	float mass;
	float collision_radius;
	float timer;

	Game_Sprite sprite;
	Primitive_Shapes shape;
	RGB_Color color;
	Uint32 parent;
} Particle;

typedef struct Particle_Emitter {
	union {
		Vector2 position;
		struct { float x, y; };
	};
	float angle;

	Uint32 parent;

	float density;
//	Game_Sprite sprite;
	Primitive_Shapes shape;
	RGB_Color colors[16];
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
	RGB_Color colors[STARFIELD_STAR_COUNT];
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

typedef enum Entity_Types {
	ENTITY_TYPE_UNDEFINED,
	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_BULLET,
	ENTITY_TYPE_ENEMY_DRIFTER,
	ENTITY_TYPE_ENEMY_UFO,
	ENTITY_TYPE_ENEMY_TRACKER,
	ENTITY_TYPE_ENEMY_TURRET,
	ENTITY_TYPE_ENEMY_GRAPPLER,
	ENTITY_TYPE_ITEM_MISSILE,
	ENTITY_TYPE_ITEM_LIFEUP,
	ENTITY_TYPE_ITEM_LASER,
	ENTITY_TYPE_SPAWN_WARP,
	ENTITY_TYPE_COUNT
} Entity_Types;

typedef enum Entity_States {
	ENTITY_STATE_UNDEFINED,
	ENTITY_STATE_SPAWNING,
	ENTITY_STATE_ACTIVE,
	ENTITY_STATE_DESPAWNING,
	ENTITY_STATE_DYING,
	ENTITY_STATE_DEAD,
	ENTITY_STATE_COUNT,
} Entity_States;

typedef enum Entity_Teams {
	ENTITY_TEAM_UNDEFINED = 0,
	ENTITY_TEAM_PLAYER,
	ENTITY_TEAM_ENEMY,
} Entity_Teams;

typedef struct Player_Entity_Data {
	Uint32 main_thruster;
	Uint32 left_thruster;
	Uint32 right_thruster;
} Player_Entity_Data;

typedef struct Tracker_Entity_Data {
	Uint32 thruster;
} Tracker_Entity_Data;

typedef struct Spawn_Warp_Entity_Data {
	Uint32 spawn_type;
} Spawn_Warp_Entity_Data;

typedef union {
	Player_Entity_Data player;
	Tracker_Entity_Data tracker;
	Spawn_Warp_Entity_Data spawn_warp;
} Entity_Data;

typedef struct Entity {
	Transform2D_Union;
	float z;
	float target_angle;
	Vec2_Union(velocity, vx, vy);
	float collision_radius;
	float timer;

	Game_Sprite sprites[4];
	Uint32 sprite_count;

	Entity_Data data;
	RGB_Color color;
	Primitive_Shapes shape;
	Entity_Types type;
	Entity_States state;
	Entity_Teams team;
} Entity;

typedef struct Game_State {
	SDL_Window* window;
	SDL_Renderer* renderer;
	STBTTF_Font* font;
	Game_Assets assets;

	SDL_Texture* world_buffer;
	int world_w, world_h;
#if DEBUG
	SDL_bool DEBUG_fit_world_to_screen;
#endif

	Entity* entities;
	Uint32 entity_count;
	Uint32 entities_size;
	
	Game_Starfield starfield;
	Particle_System particle_system;
	Score_System score;

	Uint32* dead_entities;
	Uint32 dead_entities_count;
	Uint32 dead_entities_size;

	Game_Controller_State player_controller;
	Entity* player;
	struct {
		enum Player_Weapon {
			PLAYER_WEAPON_MG,
			PLAYER_WEAPON_MISSILE,
			PLAYER_WEAPON_LASER,
		} current_weapon;
		
		int lives;
		int ammo;
		float weapon_heat;
		float thrust_energy;
		
	} player_state;
} Game_State;

#endif