#ifndef PLATFORM_H
#define PLATFORM_H

#include "SDL2/SDL_surface.h"
#include "defs.h"

Vector2 platform_get_window_size(void);
int platform_set_render_target(SDL_Texture *texture);

SDL_Texture* 	platform_create_texture					(Uint32 format, int access, int w, int h);
SDL_Texture* 	platform_create_texture_from_surface	(SDL_Surface* surface);
Vector2 		platform_get_texture_dimensions			(SDL_Texture* texture);

int 			platform_render_clear					(void);
RGBA_Color 		platform_get_render_draw_color			(void);
int 			platform_set_render_draw_color			(RGBA_Color color);

int 			platform_render_copy					(SDL_Texture *texture, const SDL_Rect *src_rect, const SDL_FRect *dst_rect,
														 const double angle  , const SDL_FPoint *center, const SDL_RendererFlip flip
														);
int 			platform_render_draw_points 			(Vector2* points, int count);
int 			platform_render_draw_lines				(const Vector2 *points, int count);
int 			platform_render_draw_rect				(SDL_FRect rect);

int 			platform_render_fill_rect				(SDL_FRect rect);

int 			platform_render_geometry				(SDL_Texture *texture, const SDL_Vertex *vertices, int num_vertices, const int *indices, int num_indices);

#endif