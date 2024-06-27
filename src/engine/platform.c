#include "platform.h"
#include "../game/game.h"
#include "input.h"
#include "math.h"

#include "assets.c"
#include "graphics.c"
#include "input.c"
#include "math.c"
#include "particles.c"
#include "ui.c"

static SDL_Window *	window = 0;
static SDL_Renderer * 	renderer = 0;
static SDL_Texture * 	world_buffer = 0;

typedef struct Game_State Game_State;

iVector2 platform_get_window_size(void) {
	iVector2 result = {0};

	if (window) {
		SDL_GetWindowSize(window, &result.x, &result.y);
	}

	return result;
}

SDL_Texture* platform_create_texture(int w, int h, SDL_bool target) {
	SDL_Texture* result = 0;
	
	if (renderer) {
		result = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, target ? SDL_TEXTUREACCESS_TARGET : SDL_TEXTUREACCESS_STATIC, w, h);
		SDL_SetTextureBlendMode(result, SDL_BLENDMODE_BLEND);
	} else {
		SDL_SetError("platform_create_texture(): renderer does not exist.");
	}

	return result;
}

SDL_Texture* platform_create_texture_from_surface(SDL_Surface* surface) {
	SDL_Texture*  result = 0;
	
	if (renderer) {
		result = SDL_CreateTextureFromSurface(renderer, surface);
	} else {
		SDL_SetError("platform_create_texture_from_surface(): renderer does not exist.");
	}

	return result;
}

void platform_destroy_texture(SDL_Texture* texture) {
	SDL_DestroyTexture(texture);
}

int platform_set_texture_alpha(SDL_Texture* texture, uint8_t alpha) {
	int result = SDL_SetTextureAlphaMod(texture, alpha);
	
	return result;
}

int platform_set_render_target(SDL_Texture *texture) {
	int result = -1;

	if (renderer) {
		result = SDL_SetRenderTarget(renderer, texture);
	} else {
		SDL_SetError("platform_set_render_target(): renderer does not exist.");
	}

	return result;
}

int platform_render_clear(void) {
	int result = -1;

	if (renderer) {
		result = SDL_RenderClear(renderer);
	} else {
		SDL_SetError("platform_render_clear(): renderer does not exist.");
	}

	return result;
}

Vector2 platform_get_texture_dimensions(SDL_Texture* texture) {
	Vector2 result = {0};

	int w, h;

	if (SDL_QueryTexture(texture, NULL, NULL, &w, &h) != -1) {
		result.x = w;
		result.y = h;
	}

	return result;
}

RGBA_Color platform_get_render_draw_color(void) {
	RGBA_Color result;
	if (renderer) {
		SDL_GetRenderDrawColor(renderer, &result.r, &result.g, &result.b, &result.a);
	}
	return result;
}

int platform_set_render_draw_color(RGBA_Color color) {
	int result = -1;
	
	if (renderer) {
		result = SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	} else {
		SDL_SetError("platform_render_set_draw_color(): renderer does not exist.");
	}
	
	return result;
}

int platform_render_copy(SDL_Texture *texture, const Rectangle *src_rect, const Rectangle *dst_rect, const double angle, const Vector2 *center, const SDL_RendererFlip flip) {
	int result = -1;

	if (renderer) {
		SDL_Rect int_src_rect;
		SDL_Rect* pi_src_rect = 0;
		if (src_rect) {
			int_src_rect = (SDL_Rect) {
				(int)src_rect->x, (int)src_rect->y,(int)src_rect->w,(int)src_rect->h,
			};
			pi_src_rect = &int_src_rect;
		}
		
		if (angle || center || flip) {
			result = SDL_RenderCopyExF(renderer, texture, pi_src_rect, (SDL_FRect*)dst_rect, angle, (SDL_FPoint*)center, flip);
		} else {
			result = SDL_RenderCopyF  (renderer, texture, pi_src_rect, (SDL_FRect*)dst_rect);
		}
	} else {
		SDL_SetError("platform_render_copy(): renderer does not exist.");
	}

	return result;
}

int platform_render_draw_points (Vector2* points, int count) {
	int result = -1;
	if (renderer) {
		result = SDL_RenderDrawPointsF(renderer, (SDL_FPoint*)points, count);
	} else {
		SDL_SetError("platform_render_draw_points(): renderer does not exist.");
	}

	return result;
}

int platform_render_draw_rect(Rectangle rect) {
	int result = -1;

	if (renderer) {	
		result = SDL_RenderDrawRectF(renderer, (SDL_FRect*)&rect);
	} else {
		SDL_SetError("platform_render_draw_rect(): renderer does not exist.");
	}

	return result;
}

int platform_render_draw_lines(const Vector2 *points, int count) {
	int result = -1;
	if (renderer) {
		if (count == 2) {
			SDL_RenderDrawLineF(renderer, points[0].x, points[0].y, points[1].x, points[1].y);
		} else {
			SDL_RenderDrawLinesF(renderer, (const SDL_FPoint*)points, count);
		}
	} else {
		SDL_SetError("platform_render_draw_points(): renderer does not exist.");
	}

	return result;
}

int platform_render_fill_rect(Rectangle rect) {
	int result = -1;

	if (renderer) {	
		result = SDL_RenderFillRectF(renderer, (SDL_FRect*)&rect);
	} else {
		SDL_SetError("platform_render_fill_rect(): renderer does not exist.");
	}

	return result;
}

int platform_render_geometry(SDL_Texture *texture, const SDL_Vertex* vertices, int num_vertices, const int *indices, int num_indices) {
	int result = -1;
	
	if (renderer) {
		result = SDL_RenderGeometry(renderer, texture, vertices, num_vertices, indices, num_indices);
	}

	return result;
}

#define DELAY_TOLERANCE 1.2
static void precise_delay(double ms) {
	if (ms > 0) {
		Uint64 ms_count = (Uint64)(ms / 1000.0 * (double)SDL_GetPerformanceFrequency()); // Delay time in terms of Performance Counter
		Uint64 start_count = SDL_GetPerformanceCounter(); // Performance count before delay
		Uint64 target = start_count + ms_count;
		if (ms > DELAY_TOLERANCE) SDL_Delay((Uint32)(ms - DELAY_TOLERANCE));
		Uint64 end_count = SDL_GetPerformanceCounter(); // Performance count after delay

		// If delay reached or passed, no need to spin
		if (end_count < target) {
			while(SDL_GetPerformanceCounter() < target) {_mm_pause(); } // spin
		} else if (end_count > target) {
			SDL_Log("SDL_Delay overshot %fms target. Slept for %fms", ms, (double)(end_count - start_count) / (double)SDL_GetPerformanceFrequency() * 1000.0);
		}
	}
}

void platform_init(Platform_State* platform) {
	SDL_Init(SDL_INIT_EVERYTHING);

	window = SDL_CreateWindow(
			platform->title,
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			platform->screen.x, platform->screen.y,
			SDL_WINDOW_RESIZABLE
	);

	if (window == NULL) {
		SDL_LogError(0, "%s", SDL_GetError());
		SDL_Quit();
	}

	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); 
	if (renderer == NULL) {
		SDL_LogError(0, "%s", SDL_GetError());
		SDL_Quit();
	}
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	SDL_GameControllerEventState(SDL_ENABLE);

	if (Mix_OpenAudio(48000, MIX_DEFAULT_FORMAT, 2, 512) != -1) {
		Mix_AllocateChannels(12);
	}


	world_buffer = SDL_CreateTexture(
		renderer, 
		SDL_PIXELFORMAT_ARGB32, SDL_TEXTUREACCESS_TARGET, 
		platform->world.x, platform->world.y
	);
	if (!world_buffer) {
		SDL_Log("Creating world buffer failed. %s", SDL_GetError());
		SDL_Quit();
	}
}

#define TICK_RATE 60
SDL_bool platform_update_and_render(Platform_State* platform, Platform_Game_State* game, Game_Input* input) {
	SDL_bool running = 1;

	double dt = (double)(platform->current_count - platform->last_count) / (double)SDL_GetPerformanceFrequency();
	dt = dt/(1.0 / (double)TICK_RATE);

	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_KEYUP:
			case SDL_KEYDOWN: {
				process_key_event(input, &event.key);
			} break;

			case SDL_CONTROLLERDEVICEADDED:
			case SDL_CONTROLLERDEVICEREMOVED:
			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
			case SDL_CONTROLLERAXISMOTION: {
				process_controller_event(input, &event);
			} break;

			case SDL_QUIT: {
				running = 0;
			} break;
		}
		if (!running) break;
	}

#ifdef DEBUG
	if (is_key_released(&game->input, SDL_SCANCODE_EQUALS)) {
		game->fit_world_to_screen = !game->fit_world_to_screen;
	}
	
	if (is_key_released(&game->input, SDL_SCANCODE_GRAVE)) {
		SDL_Log("Break");
	}
#endif
	if (is_key_pressed(input, SDL_SCANCODE_ESCAPE)) {
		running = 0;
	}

	update_game(game, input, dt);

	SDL_SetRenderTarget(renderer, world_buffer);
	draw_game_world(game);
	SDL_SetRenderTarget(renderer, 0);

	SDL_SetRenderDrawColor(renderer,0,0,0,0);
	SDL_RenderClear(renderer);

	SDL_GetWindowSize(window, &platform->screen.x, &platform->screen.y);

	Rectangle world_rect = {0,0, platform->world.x, platform->world.y};
	if (game->fit_world_to_screen) {
		SDL_SetTextureScaleMode(world_buffer, SDL_ScaleModeBest);
		world_rect = fit_rect(world_rect, (Rectangle){0,0, platform->screen.x, platform->screen.y});
	} else {
		SDL_SetTextureScaleMode(world_buffer, SDL_ScaleModeNearest);
	}
	world_rect.x = (platform->screen.x - world_rect.w)/2;
	world_rect.y = (platform->screen.y - world_rect.h)/2;

	SDL_Rect world_draw_rect = {
		world_rect.x, world_rect.y,
		world_rect.w, world_rect.h,
	};
	
	SDL_RenderCopy(renderer, world_buffer, 0, &world_draw_rect);
	draw_game_ui(game);

	Uint64 frequency = SDL_GetPerformanceFrequency();
	double time_elapsed = (double)(SDL_GetPerformanceCounter() - platform->current_count) / (double)frequency * 1000.0;
	precise_delay(platform->target_frame_time - time_elapsed);

	platform->last_count = platform->current_count;
	platform->current_count = SDL_GetPerformanceCounter();
	SDL_RenderPresent(renderer);
	
	return running;
}
