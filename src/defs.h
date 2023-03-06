#ifndef DEFS_H
#define DEFS_H

#define MATH_PI 3.14159265
#define RAD_TO_DEG(rads) rads * (180/MATH_PI)
#define DEG_TO_RAD(degs) degs * (MATH_PI/180)

typedef struct rgb_color {uint8_t r, g, b;} rgb_color;
#define CLEAR_COLOR (rgb_color){0,10,48}
#define SD_BLUE (rgb_color){109, 194, 255}

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define array_length(array) ( sizeof(array) / sizeof(array[0]) )

#define TARGET_FPS 60
#define TICK_RATE 60

#define PHYSICS_FRICTION 0.02f
#define PLAYER_FORWARD_THRUST 0.15f
#define PLAYER_LATERAL_THRUST 0.2f
#define PLAYER_TURN_SPEED 3.14f

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
		struct {Vector2 position, scale; float _a; };	\
		struct {float x, y, sx, sy, angle; };			\
	}

typedef struct Game_Sprite {
	SDL_Texture* texture;
	SDL_Rect rect;
} Game_Sprite;

typedef struct Entity {
	Transform2D_Union;
	float z;
	Vec2_Union(velocity, vx, vy);
	Game_Sprite sprite;
	SDL_bool despawning;
} Entity;

typedef struct Mix_Music_Node 	{ char* name; Mix_Music* data; struct Mix_Music_Node* next; } Mix_Music_Node;
typedef struct Mix_Chunk_Node 	{ char* name; Mix_Chunk* data; struct Mix_Chunk_Node* next; } Mix_Chunk_Node;
typedef struct SDL_Texture_Node { char* name; SDL_Texture* data; struct SDL_Texture_Node* next; } SDL_Texture_Node;

typedef struct Game_Assets {
	Mix_Music_Node music[8];
	Mix_Chunk_Node sfx[16];
	SDL_Texture_Node textures[16];
} Game_Assets;

typedef struct game_button_state {
	SDL_Scancode scan_code;
	SDL_bool pressed, held, released;
} game_button_state;

typedef union game_controller_state {
	struct {
		game_button_state thrust;
		game_button_state turn_left;
		game_button_state turn_right;
		game_button_state thrust_left;
		game_button_state thrust_right;
		game_button_state fire;
	};
	game_button_state list[6];
} game_controller_state;

typedef enum Particle_Shape {
	PARTICLE_SHAPE_UNDEFINED,
	PARTICLE_SHAPE_RECT,
	PARTICLE_SHAPE_COUNT,
} Particle_Shape;

typedef struct Particle {
	Transform2D_Union;
	Vec2_Union(velocity, vx, vy);
	float mass;
	float collision_radius;
	float life_left;

	Game_Sprite sprite;
	Particle_Shape shape;
	rgb_color color;
} Particle;

typedef struct Particle_Emitter {
	Entity* parent;
	// If parent is defined, position is relative to parent position and angle.
	union {
		Vector2 position;
		struct { float x, y; };
	};
	float angle;

	float density;
	Uint32 sprite;
	Particle_Shape shape;
	rgb_color colors[16];
	Uint32 color_count;

	float counter;
	float scale;
	SDL_bool active;
} Particle_Emitter;

#define MAX_PARTICLES 512
#define MAX_PARTICLE_EMITTERS 128
typedef struct Particle_System {
	Particle particles[MAX_PARTICLES];
	Uint32 particle_count;

	Particle_Emitter emitters[MAX_PARTICLE_EMITTERS];
	Uint32 emitter_count;
} Particle_System;

typedef struct Game_State {
	SDL_Window* window;
	SDL_Renderer* renderer;

	Game_Assets assets;

	Particle_System particle_system;

	game_controller_state player_controller;
	Entity player;
} Game_State;

static inline double sin_deg(double degrees) { return SDL_sin(DEG_TO_RAD(degrees)); }
static inline double cos_deg(double degrees) { return SDL_cos(DEG_TO_RAD(degrees)); }

float random(void);

#endif