#include "SDL2/SDL.h"
#include "assets.h"

void render_draw_circle(SDL_Renderer* renderer, int cx, int cy, int r) {
	SDL_Point points[4];
	float r_squared = (float)r * (float)r;

	uint8_t* y_used = SDL_malloc(sizeof(uint8_t) * (int)r);
	uint8_t* x_used = SDL_malloc(sizeof(uint8_t) * (int)r);
	SDL_memset(y_used, 0, sizeof(uint8_t) * (int)r);
	SDL_memset(x_used, 0, sizeof(uint8_t) * (int)r);

	for (int x = 0; x <= r; x++) {
		int y = (int)SDL_roundf(SDL_sqrtf(r_squared - (float)x*(float)x));

		points[0] = (SDL_Point){cx + x, cy + y};
		points[1] = (SDL_Point){cx + x, cy - y};
		points[2] = (SDL_Point){cx - x, cy + y};
		points[3] = (SDL_Point){cx - x, cy - y};
	
		SDL_RenderDrawPoints(renderer, points, 4);

		x_used[x] = 1;
		y_used[y] = 1;
	}

	for (int y = 0; y <= r; y++) {
		if (y_used[y]) continue;
		int x = (int)SDL_roundf(SDL_sqrtf(r_squared - (float)y*(float)y));

		points[0] = (SDL_Point){cx + x, cy + y};
		points[1] = (SDL_Point){cx + x, cy - y};
		points[2] = (SDL_Point){cx - x, cy + y};
		points[3] = (SDL_Point){cx - x, cy - y};
	
		SDL_RenderDrawPoints(renderer, points, 4);
	}

	SDL_free(x_used);
	SDL_free(y_used);
}

void render_draw_circlef(SDL_Renderer* renderer, float cx, float cy, float r) {
	SDL_FPoint points[4];
	float r_squared = r * r;
	
	uint8_t* y_used = SDL_malloc(sizeof(uint8_t) * (int)r);
	uint8_t* x_used = SDL_malloc(sizeof(uint8_t) * (int)r);
	SDL_memset(y_used, 0, sizeof(uint8_t) * (int)r);
	SDL_memset(x_used, 0, sizeof(uint8_t) * (int)r);

	for (int x = 0; x <= r; x++) {
		float fx = (float)x;
		float fy = SDL_sqrtf(r_squared - fx*fx);

		points[0] = (SDL_FPoint){cx + fx, cy + fy};
		points[1] = (SDL_FPoint){cx + fx, cy - fy};
		points[2] = (SDL_FPoint){cx - fx, cy + fy};
		points[3] = (SDL_FPoint){cx - fx, cy - fy};
	
		SDL_RenderDrawPointsF(renderer, points, 4);

		x_used[x] = 1;
		y_used[(int)fy] = 1;
	}

	for (int y = 0; y <= r; y++) {
		if (y_used[y]) continue;
		float fy = (float)y;
		float fx = SDL_sqrtf(r_squared - fy*fy);

		points[0] = (SDL_FPoint){cx + fx, cy + fy};
		points[1] = (SDL_FPoint){cx + fx, cy - fy};
		points[2] = (SDL_FPoint){cx - fx, cy + fy};
		points[3] = (SDL_FPoint){cx - fx, cy - fy};
	
		SDL_RenderDrawPointsF(renderer, points, 4);
	}

	SDL_free(x_used);
	SDL_free(y_used);
}

void render_fill_circle(SDL_Renderer* renderer, int cx, int cy, int r) {
	SDL_Point points[4];
	int radius = (int)r;
	int r_squared = radius*radius;

	for (int y = 0; y <= radius; y++) {
		for (int x = 0; x <= radius; x++) {
			if(x*x + y*y <= r_squared + r) {
				points[0] = (SDL_Point){cx + x, cy + y};
				points[1] = (SDL_Point){cx + x, cy - y};
				points[2] = (SDL_Point){cx - x, cy + y};
				points[3] = (SDL_Point){cx - x, cy - y};

				SDL_RenderDrawPoints(renderer, points, 4);
			}
		}
	}
}

// TO-DO: Figure out why sub-pixel rendering doesn't seem to be working here. Very clear pixel jitter on moving objects.
void render_fill_circlef(SDL_Renderer* renderer, float cx, float cy, float r) {
	SDL_FPoint points[4];
	int radius = (int)SDL_roundf(r);
	float r_squared = r*r;

	for (int y = 0; y <= radius; y++) {
		for (int x = 0; x <= radius; x++) {
			float fx = (float)x;
			float fy = (float)y;

			if(fx*fx + fy*fy <= r_squared + r) {
				points[0] = (SDL_FPoint){cx + fx, cy + fy};
				points[1] = (SDL_FPoint){cx + fx, cy - fy};
				points[2] = (SDL_FPoint){cx - fx, cy + fy};
				points[3] = (SDL_FPoint){cx - fx, cy - fy};

				SDL_RenderDrawPointsF(renderer, points, 4);
			}
		}
	}
}

static inline float lerp(float start, float end, float t) {
	t = SDL_clamp(t, 0.0f, 1.0f);

	return (1.0f - t) * start + (t * end);
}

void render_fill_circlef_linear_gradient(SDL_Renderer* renderer, float cx, float cy, float r, RGB_Color start_color, RGB_Color end_color) {
	SDL_FPoint points[4];
	int radius = (int)SDL_roundf(r);
	float r_squared = r*r;

	for (int y = 0; y <= radius; y++) {
		for (int x = 0; x <= radius; x++) {
			float fx = (float)x;
			float fy = (float)y;

			float t = (fx*fx + fy*fy) / (r_squared + r);

			if (t <= 1.0f) {
				SDL_SetRenderDrawColor(renderer, 
					(Uint8)lerp(start_color.r, end_color.r, t),
					(Uint8)lerp(start_color.g, end_color.g, t),
					(Uint8)lerp(start_color.b, end_color.b, t),
					255
				);

				points[0] = (SDL_FPoint){cx + fx, cy + fy};
				points[1] = (SDL_FPoint){cx + fx, cy - fy};
				points[2] = (SDL_FPoint){cx - fx, cy + fy};
				points[3] = (SDL_FPoint){cx - fx, cy - fy};

				SDL_RenderDrawPointsF(renderer, points, 4);
			}
		}
	}
}

void render_draw_texture(SDL_Renderer* renderer, SDL_Texture* texture, float x, float y, float angle, SDL_bool centered) {
	if (texture) {
		int dest_w, dest_h;

		SDL_QueryTexture(texture, NULL, NULL, &dest_w, &dest_h);

		SDL_FRect dest_rect;
		dest_rect.x = x;
		dest_rect.y = y;
		dest_rect.w = (float)dest_w;
		dest_rect.h = (float)dest_h;

		if (centered == SDL_TRUE) {
			dest_rect.x -= dest_rect.w/2.0f;
			dest_rect.y -= dest_rect.h/2.0f;
		}

		SDL_RenderCopyExF(renderer, texture, NULL, &dest_rect, angle, 0, SDL_FLIP_NONE);
	}
}

void render_draw_game_sprite(Game_State* game, Game_Sprite* sprite, Transform2D transform, SDL_bool centered) {
	SDL_Texture* texture =  game_get_texture(game, sprite->texture_name);

	if (texture) {
		SDL_Rect sprite_rect = get_sprite_rect(game, sprite);

		SDL_FRect dest_rect;
		dest_rect.x = transform.x;
		dest_rect.y = transform.y;
		dest_rect.w = (float)sprite_rect.w;
		dest_rect.h = (float)sprite_rect.h;

		if (transform.sx > 0.0f && transform.sy > 0.0f) {
			dest_rect.w *= transform.sx;
			dest_rect.h *= transform.sy;

			if (centered == SDL_TRUE) {
				dest_rect.x -= dest_rect.w/2.0f;
				dest_rect.y -= dest_rect.h/2.0f;
			}

			SDL_RenderCopyExF(game->renderer, texture, &sprite_rect, &dest_rect, transform.angle * (float)(int)sprite->rotation, 0, SDL_FLIP_NONE);
		}
	}
}