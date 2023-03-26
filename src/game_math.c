#include "defs.h"

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

Game_Poly2D translate_poly2d(Game_Poly2D polygon, Vector2 translation) {
	Game_Poly2D result = {0};
	result.vert_count = polygon.vert_count;

	for (int i = 0; i < polygon.vert_count; i++) {
		result.vertices[i].x = polygon.vertices[i].x + translation.x;
		result.vertices[i].y = polygon.vertices[i].y + translation.y;
	}

	return result;
}