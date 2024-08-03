#ifndef ENGINE_TYPES_H
#define ENGINE_TYPES_H

#include "SDL.h"
#include "SDL_mixer.h"

#define array_length(arr) (ptrdiff_t)(sizeof(arr) / sizeof(*(arr)))

typedef struct RGBA_Color {uint8_t r, g, b, a;} RGBA_Color;

typedef struct Lerp_Timer {
	float min, time, max, dir;
} Lerp_Timer;

typedef enum stbi_masks {
	STBI_MASK_R = 0x000000FF, 
	STBI_MASK_G = 0x0000FF00,
	STBI_MASK_B = 0x00FF0000,
	STBI_MASK_A = 0xFF000000,
} stbi_masks;

typedef struct Vector2 {
	float x, y;
} Vector2;

typedef struct iVector2 {
	int x, y;
} iVector2;

#define Vec2_Union(v2_name, fx, fy)		\
	union { 				\
		Vector2 v2_name; 		\
		struct {float fx, fy; }; 	\
	}

typedef struct Transform2D {
	Vec2_Union(position, x, y);
	Vec2_Union(scale, sx, sy);
	float angle;
} Transform2D;

#define Transform2D_Union			\
union {						\
	Transform2D transform;			\
	struct {float x, y, sx, sy, angle; };	\
	struct {Vector2 position, scale; };	\
}

typedef struct Rectangle {
	float x;
	float y;
	float w;
	float h;
} Rectangle;

#define MAX_POLY2D_VERTS 8
typedef struct Poly2D {
	Vector2 vertices[MAX_POLY2D_VERTS];
	Uint32 vert_count;
} Poly2D;

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
		Poly2D polygon;
		Rectangle rectangle;
		struct { float radius;};
	};
} Game_Shape;

typedef struct Game_Sprite {
	char* texture_name;
	Rectangle src_rect;
	Vector2 offset;
	SDL_bool rotation_enabled;
} Game_Sprite;

typedef struct Mix_Music_Node 	{ char* name; Mix_Music* data; 	 struct Mix_Music_Node* next; 	} Mix_Music_Node;
typedef struct Mix_Chunk_Node 	{ char* name; Mix_Chunk* data; 	 struct Mix_Chunk_Node* next; 	} Mix_Chunk_Node;
typedef struct SDL_Texture_Node { char* name; SDL_Texture* data; struct SDL_Texture_Node* next; } SDL_Texture_Node;

#include "external/stb_truetype.h"
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

typedef struct Game_Assets Game_Assets;

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
	Vector2 scale;
	float angle;
	float speed;

	float density;
//	Game_Sprite sprite;
	Game_Shape_Types shape;
	RGBA_Color colors[16];
	Uint32 color_count;

	float counter;
	enum Emitter_State {
		EMITTER_STATE_INACTIVE,
		EMITTER_STATE_ACTIVE,
		EMITTER_STATE_DEAD,
	} state;
} Particle_Emitter;

typedef struct Particle_System Particle_System;

typedef struct Game_State Platform_Game_State;

#endif
