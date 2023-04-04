#ifndef GAME_GRAPHICS_H
#define GAME_GRAPHICS_H

#include "SDL2/SDL.h"
#include "defs.h"

void render_draw_texture(SDL_Renderer* renderer, SDL_Texture* texture, float x, float y, float angle, SDL_bool centered);
void render_draw_game_sprite(Game_State* game, Game_Sprite* sprite, Transform2D transform, SDL_bool centered);
void render_draw_circle(SDL_Renderer* renderer, int cx, int cy, int r);
void render_draw_circlef(SDL_Renderer* renderer, float cx, float cy, float r);
void render_fill_circle(SDL_Renderer* renderer, int cx, int cy, int r);
void render_fill_circlef(SDL_Renderer* renderer, float cx, float cy, float r);
void render_fill_circlef_linear_gradient(SDL_Renderer* renderer, float cx, float cy, float r, RGB_Color start_color, RGB_Color end_color);
void render_draw_polygon(SDL_Renderer* renderer, SDL_FPoint* points, int num_points);
void render_fill_polygon(SDL_Renderer* renderer, SDL_FPoint* points, int num_points, RGB_Color color);
void render_draw_triangle(SDL_Renderer* renderer, Vector2 v1, Vector2 v2, Vector2 v3);
void render_fill_triangle(SDL_Renderer* renderer, Vector2 v1, Vector2 v2, Vector2 v3, RGB_Color color);

void render_draw_game_shape(SDL_Renderer* renderer, Vector2 position, Game_Shape shape, RGB_Color color);
void render_fill_game_shape(SDL_Renderer* renderer, Vector2 position, Game_Shape shape, RGB_Color color);


#endif