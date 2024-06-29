#include "math.h"

#define sc2d_atan2 SDL_atan2f
#define sc2d_fabsf SDL_fabsf
#define sc2d_hypotf hypotf
#define sc2d_min(i, j) SDL_min(i, j) 
#define sc2d_max(i, j) SDL_max(i, j)
#define SIMPLE_COLLISION_2D_IMPLEMENTATION
#include "external/sc2d.h"

#ifndef _WIN32
float hypotf(float x, float y) {
	float result = SDL_sqrtf(SDL_powf(x, 2) + SDL_powf(y, 2));
	return result;
}
#endif
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

float angle_rotation_to_target(Vector2 origin, Vector2 target, float angle, float tolerance) {
	float result = 0;
	
	float target_angle = normalize_degrees( atan2_deg(target.y - origin.y, target.x - origin.x) );
	float angle_delta = cos_deg(target_angle)*sin_deg(angle) - sin_deg(target_angle)*cos_deg(angle);

	if (angle_delta < -tolerance) result = 1;
	else if (angle_delta > tolerance) result = -1;
	
	return result;
}

// Returns a pseudo-random value between 0 and 1
float randomf(void) {
	float result = (float)(rand() % 1000) / 1000.0f;

	return result;
}

float vector2_length(Vector2 v) {
	float result = 0;

	if (v.x || v.y) {
		result = SDL_sqrt( (v.x * v.x) + (v.y * v.y) );
	}

	return result;
}

Vector2 normalize_vector2(Vector2 v) {
	Vector2 result = v;

	if (v.x || v.y) {
		float magnitude = vector2_length(v);
		result.x /= magnitude;
		result.y /= magnitude;
	}

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

Vector2 add_vector2(Vector2 v1, Vector2 v2) {
	Vector2 result = {
		v1.x + v2.x,
		v1.y + v2.y
	};

	return result;
}

// Sbutracts v2 from v1
Vector2 subtract_vector2(Vector2 v1, Vector2 v2) {
	Vector2 result = {
		v1.x - v2.x,
		v1.y - v2.y
	};

	return result;
}

float dot_product_vector2(Vector2 v1, Vector2 v2) {
	float result = v1.x * v2.x + v1.y * v2.y;

	return result;
}

Rectangle translate_rect(Rectangle rect, Vector2 translation) {
	Rectangle result = rect;

	result.x += translation.x;
	result.y += translation.y;

	return result;
}

Rectangle scale_rect(Rectangle rect, Vector2 scale) {
	Rectangle result = {
		.x = rect.x*scale.x,
		.w = rect.w*scale.x,

		.y = rect.y*scale.y,
		.h = rect.h*scale.y,
	};

	return result;
}

Rectangle fit_rect(Rectangle inner, Rectangle outer) {
	Rectangle result = inner;
	float scale;
	if (outer.h < outer.w) {
		scale = outer.h / inner.h;
	} else {
		scale = outer.w / inner.w;
	}

	result.w *= scale;
	result.h *= scale;

	return result;
}

Rectangle center_rect(Rectangle inner, Rectangle outer) {
	Rectangle result = inner;
	result.x = (outer.w-inner.w)/2.0f;
	result.y = (outer.h-inner.h)/2.0f;

	return result;	
}

Poly2D rect_to_poly2D(Rectangle rect) {
	Poly2D result = {
		.vert_count = 4,
		.vertices = {
			{rect.x, rect.y},
			{rect.x + rect.w, rect.y},
			{rect.x + rect.w, rect.y + rect.h},
			{rect.x, rect.y + rect.h},
		},
	};

	return result;
}

Poly2D generate_poly2D(int vert_count, float r_min, float r_max) {
	Poly2D result = {
		.vert_count = vert_count,
	};
	
	for (int i = 0; i < vert_count; i++) {
		float point_dist = r_min + randomf() * (r_max - r_min);
		float new_angle = 360.0f / (float)vert_count * (float)i;
		
		result.vertices[i].x = cos_deg(new_angle) * point_dist;
		result.vertices[i].y = sin_deg(new_angle) * point_dist;
	}

	return result;
}

Poly2D translate_poly2d(Poly2D polygon, Vector2 translation) {
	Poly2D result = {0};
	result.vert_count = polygon.vert_count;

	for (int i = 0; i < polygon.vert_count; i++) {
		result.vertices[i].x = polygon.vertices[i].x + translation.x;
		result.vertices[i].y = polygon.vertices[i].y + translation.y;
	}

	return result;
}

Poly2D rotate_poly2d(Poly2D p, float degrees) {
	Poly2D result = p;

	for (int i = 0; i < p.vert_count; i++) {
		result.vertices[i] = rotate_vector2(p.vertices[i], degrees);
	}

	return result;
}

Poly2D scale_poly2d(Poly2D polygon, Vector2 scale) {
	Poly2D result = {0};
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
				result.rectangle.x *= scale.x;
				result.rectangle.y *= scale.y;
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

Game_Shape rotate_game_shape(Game_Shape shape, float degrees) {
	Game_Shape result = shape;

	if (degrees) {
		switch(result.type) {
			case SHAPE_TYPE_RECT: {
				result.type = SHAPE_TYPE_POLY2D;
				result.polygon = rect_to_poly2D(shape.rectangle);
				result.polygon = rotate_poly2d(result.polygon, degrees);
			} break;

			case SHAPE_TYPE_POLY2D: {
				result.polygon = rotate_poly2d(shape.polygon, degrees);
			} break;
		}
	}

	return result;
}

SDL_bool check_shape_collision(Transform2D t1, Game_Shape s1, Transform2D t2, Game_Shape s2, Vector2* overlap) {
	SDL_bool result = false;

	Transform2D* transforms[2] = {&t1, &t2};
	Game_Shape* shapes[2] = {&s1, &s2};
	Poly2D colliders[2];

	for (int i = 0; i < 2; i++) {
		switch(shapes[i]->type) {
			case SHAPE_TYPE_POLY2D: {
				colliders[i] = shapes[i]->polygon;
			} break;

			case SHAPE_TYPE_RECT: {
				if (shapes[i]->rectangle.w == 0 || shapes[i]->rectangle.h == 0) return result;

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
				if (shapes[i]->radius == 0) return result;

				float angle = 0;
				float angle_increment = 360.0f / (float)MAX_POLY2D_VERTS;
				for (int circle_vert = 0; circle_vert < MAX_POLY2D_VERTS; circle_vert++) {
					colliders[i].vertices[circle_vert].x = cos_deg(angle) * shapes[i]->radius;
					colliders[i].vertices[circle_vert].y = sin_deg(angle) * shapes[i]->radius;

					angle += angle_increment;
				}
				colliders[i].vert_count = MAX_POLY2D_VERTS;
			} break;

			default: break;
		}
		
		colliders[i] = scale_poly2d(colliders[i], transforms[i]->scale);
		colliders[i] = rotate_poly2d(colliders[i], transforms[i]->angle);
	}

	result = sc2d_check_poly2d(
		t1.x, t1.y, (float*)colliders[0].vertices, colliders[0].vert_count,
		t2.x, t2.y, (float*)colliders[1].vertices, colliders[1].vert_count,
		&overlap->x, &overlap->y
	);

	return result;
}
