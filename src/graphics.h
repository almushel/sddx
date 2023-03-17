#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "SDL2/SDL.h"
#include "defs.h"

void render_draw_texture(SDL_Renderer* renderer, SDL_Texture* texture, float x, float y, float angle, SDL_bool centered);
void render_draw_game_sprite(Game_State* game, Game_Sprite* sprite, Transform2D transform, SDL_bool centered);
void render_draw_circle(SDL_Renderer* renderer, int cx, int cy, int r);
void render_draw_circlef(SDL_Renderer* renderer, float cx, float cy, float r);
void render_fill_circle(SDL_Renderer* renderer, int cx, int cy, int r);
void render_fill_circlef(SDL_Renderer* renderer, float cx, float cy, float r);

#endif