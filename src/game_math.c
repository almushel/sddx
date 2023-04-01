#include "defs.h"
#include "game_math.h"

#define SIMPLE_COLLISION_2D_IMPLEMENTATION
#include "sc2d/src/sc2d.h"

float sin_deg  (float degrees) 		{ return SDL_sinf(DEG_TO_RAD(degrees)); }
float cos_deg  (float degrees) 		{ return SDL_cosf(DEG_TO_RAD(degrees)); }
float atan2_deg(float y, float x) 	{ return RAD_TO_DEG(SDL_atan2f(y, x )); }
float normalize_degrees(float degrees) {
	float result = degrees;
	while (result < 0) result += 360.0f; 
	while (result > 360.0f) result -= 360.0f;
	
	return result;
}

float lerp(float start, float end, float t) {
	t = SDL_clamp(t, 0.0f, 1.0f);

	return (1.0f - t) * start + (t * end);
}

// Returns a pseudo-random value between 0 and 1
float random(void) {
	static SDL_bool seeded = 0;
	if (!seeded) {
		srand(42);
		seeded = 1;
	}

	float result = (float)(rand() % 1000) / 1000.0f;

	return result;
}

Vector2 scale_vector2(Vector2 v, float scalar) {
	Vector2 result = v;

	result.x *= scalar;
	result.y *= scalar;

	return result;
}

Vector2 normalize_vector2(Vector2 v) {
	Vector2 result = v;

	float magnitude = sqrtf( (v.x*v.x) + (v.y*v.y) );
	result.x /= magnitude;
	result.y /= magnitude;

	return result;
}

Vector2 rotate_vector2(Vector2 v, float degrees) {
	Vector2 result = v;
	if (degrees) {
		result.x = v.x * cos_deg(degrees) - v.y * sin_deg(degrees);
		result.y = v.x * sin_deg(degrees) + v.y * cos_deg(degrees);
	}

	return result;
}

Game_Poly2D translate_poly2d(Game_Poly2D polygon, Vector2 translation) {
	Game_Poly2D result = {0};
	result.vert_count = polygon.vert_count;

	for (int i = 0; i < polygon.vert_count; i++) {
		result.vertices[i].x = polygon.vertices[i].x + translation.x;
		result.vertices[i].y = polygon.vertices[i].y + translation.y;
	}

	return result;
}

Vector2 wrap_world_coords(float x, float y, float min_x, float min_y, float max_x, float max_y) {
	Vector2 result = {x, y};

	if (result.x < min_x) result.x = max_x + result.x;
	else if (result.x > max_x) result.x -= max_x;

	if (result.y < min_y) result.y = max_y + result.y;
	else if (result.y > max_y) result.y -= max_y;

	return result;
}