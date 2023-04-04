#ifndef GAME_MATH_H
#define GAME_MATH_H

#include "sc2d/src/sc2d.h"

#define MATH_PI 3.14159265
#define RAD_TO_DEG(rads) rads * (180/MATH_PI)
#define DEG_TO_RAD(degs) degs * (MATH_PI/180)

float sin_deg  (float degrees);
float cos_deg  (float degrees);
float atan2_deg(float y, float x);
float normalize_degrees(float degrees);

Vector2 normalize_vector2(Vector2 v);
Vector2 scale_vector2(Vector2 v, float scalar);
Vector2 rotate_vector2(Vector2 v, float degrees);
float dot_product_vector2(Vector2 v1, Vector2 v2);
Vector2 wrap_world_coords(float x, float y, float min_x, float min_y, float max_x, float max_y);

SDL_FRect translate_rect(SDL_FRect rect, Vector2 translation);

Game_Poly2D translate_poly2d(Game_Poly2D polygon, Vector2 translation);
Game_Poly2D scale_poly2d(Game_Poly2D polygon, Vector2 scale);

Game_Shape scale_game_shape(Game_Shape shape, Vector2 scale);
bool check_shape_collision(Vector2 p1, Game_Shape s1, Vector2 p2, Game_Shape s2, Vector2* overlap);

float lerp(float start, float end, float t);
float random(void);

#endif