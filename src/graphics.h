#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "SDL2/SDL.h"

void render_circle(SDL_Renderer* renderer, int cx, int cy, int r);
void render_circlef(SDL_Renderer* renderer, float cx, float cy, float r);
void render_fill_circle(SDL_Renderer* renderer, int cx, int cy, int r);
void render_fill_circlef(SDL_Renderer* renderer, float cx, float cy, float r);

#endif