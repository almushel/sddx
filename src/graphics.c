#include "SDL2/SDL.h"

void render_circle(SDL_Renderer* renderer, int cx, int cy, int r) {
	float r_squared = (float)r * (float)r;
	SDL_Point points[4];

	uint8_t* y_used = SDL_malloc(sizeof(uint8_t) * (int)r);
	SDL_memset(y_used, 0, sizeof(uint8_t) * (int)r);
	uint8_t* x_used = SDL_malloc(sizeof(uint8_t) * (int)r);
	SDL_memset(x_used, 0, sizeof(uint8_t) * (int)r);

	for (int x = 0; x <= r; x++) {
		if (x_used[x]) continue;
		float fx = (float)x;
		int y = (int)SDL_roundf(SDL_sqrtf(r_squared - fx*fx));

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
		float fy = (float)y;
		int x = (int)SDL_roundf(SDL_sqrtf(r_squared - fy*fy));

		points[0] = (SDL_Point){cx + x, cy + y};
		points[1] = (SDL_Point){cx + x, cy - y};
		points[2] = (SDL_Point){cx - x, cy + y};
		points[3] = (SDL_Point){cx - x, cy - y};
	
		SDL_RenderDrawPoints(renderer, points, 4);

		y_used[y] = 1;
	}

	SDL_free(x_used);
	SDL_free(y_used);
}

void render_circlef(SDL_Renderer* renderer, float cx, float cy, float r) {
	SDL_FPoint points[4];
	float r_squared = r * r;
	
	uint8_t* y_used = SDL_malloc(sizeof(uint8_t) * (int)r);
	SDL_memset(y_used, 0, sizeof(uint8_t) * (int)r);
	uint8_t* x_used = SDL_malloc(sizeof(uint8_t) * (int)r);
	SDL_memset(x_used, 0, sizeof(uint8_t) * (int)r);

	for (int x = 0; x <= r; x++) {
		if (x_used[x]) continue;
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

		y_used[y] = 1;
	}

	SDL_free(x_used);
	SDL_free(y_used);
}

void render_fill_circle(SDL_Renderer* renderer, int cx, int cy, int r) {
	int radius = (int)r;
	int r_squared = radius*radius;

	for (int y = 0; y <= radius; y++) {
		for (int x = 0; x <= radius; x++) {
			if(x*x + y*y <= r_squared) {
				SDL_Point points[4];
				points[0] = (SDL_Point){cx + x, cy + y};
				points[1] = (SDL_Point){cx + x, cy - y};
				points[2] = (SDL_Point){cx - x, cy + y};
				points[3] = (SDL_Point){cx - x, cy - y};

				SDL_RenderDrawPoints(renderer, points, 4);
			}
		}
	}
}

void render_fill_circlef(SDL_Renderer* renderer, float cx, float cy, float r) {
	int radius = (int)r;
	float r_squared = r*r;

	for (int y = 0; y <= radius; y++) {
		for (int x = 0; x <= radius; x++) {
			float fx = (float)x;
			float fy = (float)y;

			if(fx*fx + fy*fy <= r_squared) {
				SDL_FPoint points[4];
				points[0] = (SDL_FPoint){cx + x, cy + y};
				points[1] = (SDL_FPoint){cx + x, cy - y};
				points[2] = (SDL_FPoint){cx - x, cy + y};
				points[3] = (SDL_FPoint){cx - x, cy - y};

				SDL_RenderDrawPointsF(renderer, points, 4);
			}
		}
	}
}