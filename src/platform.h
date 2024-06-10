#ifndef PLATFORM_H
#define PLATFORM_H

#include "defs.h"

iVector2			platform_get_window_size		(void);
int			platform_set_render_target		(SDL_Texture *texture);

SDL_Texture*		platform_create_texture			(int w, int h, bool target);
SDL_Texture*		platform_create_texture_from_surface	(SDL_Surface* surface);
void 			platform_destroy_texture		(SDL_Texture* texture);
Vector2 		platform_get_texture_dimensions		(SDL_Texture* texture);
int 			platform_set_texture_alpha		(SDL_Texture* texture, uint8_t alpha);

int 			platform_render_clear			(void);
RGBA_Color 		platform_get_render_draw_color		(void);
int 			platform_set_render_draw_color		(RGBA_Color color);

int 			platform_render_copy			(SDL_Texture *texture,
								const Rectangle *src_rect, const Rectangle *dst_rect,
								const double angle, const Vector2 *center,
								const SDL_RendererFlip flip
														);
int 			platform_render_draw_points 		(Vector2* points, int count);
int 			platform_render_draw_lines		(const Vector2 *points, int count);
int 			platform_render_draw_rect		(Rectangle rect);

int 			platform_render_fill_rect		(Rectangle rect);

int 			platform_render_geometry		(SDL_Texture *texture,
								const SDL_Vertex *vertices, int num_vertices,
								const int *indices, int num_indices);

#endif
