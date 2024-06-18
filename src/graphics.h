#ifndef GAME_GRAPHICS_H
#define GAME_GRAPHICS_H

#include "defs.h"

float 	measure_text				(STBTTF_Font* font, float size, const char* text);
void 	render_text				(STBTTF_Font* font, float size, float x, float y, const char* text);
void 	render_text_aligned			(STBTTF_Font* font, float size, float x, float y, const char* text, const char* alignment);

void 	render_draw_texture			(SDL_Texture* texture, float x, float y, float angle, SDL_bool centered);
void 	render_draw_game_sprite			(Game_State* game, Game_Sprite* sprite, Transform2D transform, SDL_bool centered);
void 	render_draw_circle			(int cx, int cy, int r);
void 	render_draw_circlef			(float cx, float cy, float r);
void 	render_fill_circle			(int cx, int cy, int r);
void 	render_fill_circlef			(float cx, float cy, float r);
void 	render_fill_circlef_linear_gradient	(float cx, float cy, float r, RGBA_Color start_color, RGBA_Color end_color);
void 	render_draw_polygon			(Vector2* points, int num_points);
void 	render_fill_polygon			(Vector2* points, int num_points, RGBA_Color color);
void 	render_draw_triangle			(Vector2 v1, Vector2 v2, Vector2 v3);
void 	render_fill_triangle			(Vector2 v1, Vector2 v2, Vector2 v3, RGBA_Color color);
void 	render_draw_game_shape			(Vector2 position, Game_Shape shape, RGBA_Color color);
void 	render_fill_game_shape			(Vector2 position, Game_Shape shape, RGBA_Color color);

#endif
