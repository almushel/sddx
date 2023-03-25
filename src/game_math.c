#include "defs.h"

#define SIMPLE_COLLISION_2D_IMPLEMENTATION
#include "sc2d/src/sc2d.h"

Game_Poly2D translate_poly2d(Game_Poly2D polygon, Vector2 translation) {
	Game_Poly2D result = {0};
	result.vert_count = polygon.vert_count;

	for (int i = 0; i < polygon.vert_count; i++) {
		result.vertices[i].x = polygon.vertices[i].x + translation.x;
		result.vertices[i].y = polygon.vertices[i].y + translation.y;
	}

	return result;
}