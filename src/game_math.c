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

float vector2_length(Vector2 v) {
	float result = 0;

	result = sqrtf( (v.x * v.x) + (v.y * v.y) );

	return result;
}

Vector2 normalize_vector2(Vector2 v) {
	Vector2 result = v;

	float magnitude = vector2_length(v);
	result.x /= magnitude;
	result.y /= magnitude;

	return result;
}

Vector2 scale_vector2(Vector2 v, float scalar) {
	Vector2 result = v;

	result.x *= scalar;
	result.y *= scalar;

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

float dot_product_vector2(Vector2 v1, Vector2 v2) {
	float result = v1.x * v2.x + v1.y * v2.y;

	return result;
}

SDL_FRect translate_rect(SDL_FRect rect, Vector2 translation) {
	SDL_FRect result = rect;

	result.x += translation.x;
	result.y += translation.y;

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

Game_Poly2D scale_poly2d(Game_Poly2D polygon, Vector2 scale) {
	Game_Poly2D result = {0};
	result.vert_count = polygon.vert_count;

	for (int i = 0; i < polygon.vert_count; i++) {
		result.vertices[i].x = polygon.vertices[i].x * scale.x;
		result.vertices[i].y = polygon.vertices[i].y * scale.y;
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

Game_Shape scale_game_shape(Game_Shape shape, Vector2 scale) {
	Game_Shape result = shape;

	if (scale.x != 1.0f && scale.y != 1.0f) {
		switch(result.type) {
			case SHAPE_TYPE_RECT: 	{
				result.rectangle.w *= scale.x;
				result.rectangle.h *= scale.y;
			} break;
			
			case SHAPE_TYPE_CIRCLE: {
				result.radius *= (scale.x + scale.y) / 2.0f;
			} break;

			case SHAPE_TYPE_POLY2D: {
				result.polygon = scale_poly2d(shape.polygon, scale);
			} break;
		}
	}


	return result;
}

bool check_shape_collision(Vector2 p1, Game_Shape s1, Vector2 p2, Game_Shape s2, Vector2* overlap) {
	bool result = false;

	Game_Shape* shapes[2] = {&s1, &s2};
	Game_Poly2D colliders[2];

	for (int i = 0; i < 2; i++) {
		switch(shapes[i]->type) {
			case SHAPE_TYPE_POLY2D: {
				colliders[i] = shapes[i]->polygon;
			} break;

			case SHAPE_TYPE_RECT: {
				colliders[i].vertices[0] = (Vector2) {
					shapes[i]->rectangle.x,
					shapes[i]->rectangle.y,
				};
				colliders[i].vertices[1] = (Vector2) {
					shapes[i]->rectangle.x + shapes[i]->rectangle.w,
					shapes[i]->rectangle.y,
				};
				colliders[i].vertices[2] = (Vector2) {
					shapes[i]->rectangle.x + shapes[i]->rectangle.w,
					shapes[i]->rectangle.y + shapes[i]->rectangle.h,
				};
				colliders[i].vertices[3] = (Vector2) {
					shapes[i]->rectangle.x,
					shapes[i]->rectangle.y + shapes[i]->rectangle.h,
				};
				colliders[i].vert_count = 4;

			} break;

			case SHAPE_TYPE_CIRCLE: {
				float angle = 0;
				float angle_increment = 360.0f / (float)MAX_POLY2D_VERTS;
				for (int circle_vert = 0; circle_vert < MAX_POLY2D_VERTS; circle_vert++) {
					colliders[i].vertices[circle_vert].x = cos_deg(angle) * shapes[i]->radius;
					colliders[i].vertices[circle_vert].y = sin_deg(angle) * shapes[i]->radius;

					angle += angle_increment;
				}
				colliders[i].vert_count = MAX_POLY2D_VERTS;
			} break;
		}	
	}

	result = sc2d_check_poly2d(
		p1.x, p1.y, (float*)colliders[0].vertices, colliders[0].vert_count,
		p2.x, p2.y, (float*)colliders[1].vertices, colliders[1].vert_count,
		&overlap->x, &overlap->y
	);

	return result;
}