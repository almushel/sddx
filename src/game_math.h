#include "sc2d/src/sc2d.h"

Game_Poly2D translate_poly2d(Game_Poly2D polygon, Vector2 translation);

float sin_deg  (float degrees);
float cos_deg  (float degrees);
float atan2_deg(float y, float x);
float normalize_degrees(float degrees);

float lerp(float start, float end, float t);

float random(void);

Vector2 wrap_world_coords(float x, float y, float min_x, float min_y, float max_x, float max_y);