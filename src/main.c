#include "SDL.h"
#include "game.h"

static SDL_Window *	window = 0;
static SDL_Renderer * 	renderer = 0;
static SDL_Texture * 	world_buffer = 0;

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

int platform_render_copy(SDL_Texture *texture, const Rectangle *src_rect, const Rectangle *dst_rect, const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip) {
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
			result = SDL_RenderCopyExF(renderer, texture, pi_src_rect, (SDL_FRect*)dst_rect, angle, center, flip);
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

int main(int argc, char* argv[]) {
	int screen_w = 1280;
	int screen_h = 720;

	SDL_Init(SDL_INIT_EVERYTHING);

	window = SDL_CreateWindow("Space Drifter DX", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_w, screen_h, SDL_WINDOW_RESIZABLE);
	if (window == NULL) {
		SDL_LogError(0, "%s", SDL_GetError());
		exit(1);
	}

	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); 
	if (renderer == NULL) {
		SDL_LogError(0, "%s", SDL_GetError());
		exit(1);
	}
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	SDL_GameControllerEventState(SDL_ENABLE);

	if (Mix_OpenAudio(48000, MIX_DEFAULT_FORMAT, 2, 512) != -1) {
		Mix_AllocateChannels(12);
	}

	Game_State* game = SDL_malloc(sizeof(Game_State));
	SDL_memset(game, 0, sizeof(Game_State));

	game->entities_size = 256;
	game->entities = SDL_malloc(sizeof(Entity) * game->entities_size);
	SDL_memset(game->entities, 0, sizeof(Entity) * game->entities_size);

	init_game(game);

	world_buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB32, SDL_TEXTUREACCESS_TARGET, game->world_w, game->world_h);
	if (!world_buffer) {
		SDL_Log("Creating world buffer failed. %s", SDL_GetError());
	}

	SDL_bool fit_world_to_screen = 1;
	double target_fps = (double)TARGET_FPS;
	double target_frame_time = 1000.0/target_fps;
	Uint64 last_count = SDL_GetPerformanceCounter();
	Uint64 current_count = last_count;

	SDL_bool running = true;
	while (running) {
		double dt = (double)(current_count - last_count) / (double)SDL_GetPerformanceFrequency();
		dt = dt/(1.0 / (double)TICK_RATE);
	
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYUP:
				case SDL_KEYDOWN: {
					process_key_event(&game->input, &event.key);
				} break;

				case SDL_CONTROLLERDEVICEADDED:
				case SDL_CONTROLLERDEVICEREMOVED:
				case SDL_CONTROLLERBUTTONDOWN:
				case SDL_CONTROLLERBUTTONUP:
				case SDL_CONTROLLERAXISMOTION: {
					process_controller_event(&game->input, &event);
				} break;

				case SDL_QUIT: {
					running = false;
				} break;
			}
			if (!running) break;
		}

#ifdef DEBUG
		if (is_key_released(&game->input, SDL_SCANCODE_EQUALS)) {
			fit_world_to_screen = !fit_world_to_screen;
		}
		
		if (is_key_released(&game->input, SDL_SCANCODE_GRAVE)) {
			SDL_Log("Break");
		}
#endif
		if (is_key_pressed(&game->input, SDL_SCANCODE_ESCAPE)) {
			SDL_Event quit_event;
			quit_event.type = SDL_QUIT;
			SDL_PushEvent(&quit_event);
		}

		update_game(game, dt);

		SDL_SetRenderTarget(renderer, world_buffer);
		draw_game_world(game);
		SDL_SetRenderTarget(renderer, 0);

		SDL_SetRenderDrawColor(renderer,0,0,0,0);
		SDL_RenderClear(renderer);

		SDL_GetWindowSize(window, &screen_w, &screen_h);

		float world_scale = 1;
		int world_offset_x = 1, world_offset_y = 1;

		if (fit_world_to_screen) {
			SDL_SetTextureScaleMode(world_buffer, SDL_ScaleModeBest);
			if (screen_h < screen_w) {
				world_scale = (float)screen_h / (float)game->world_h;
				world_offset_y = 0;
			} else {
				world_scale = (float)screen_w / (float)game->world_w;
				world_offset_x = 0;
			}
		} else {
			SDL_SetTextureScaleMode(world_buffer, SDL_ScaleModeNearest);
		}
		int world_w, world_h;
		SDL_QueryTexture(world_buffer, 0, 0, &world_w, &world_h);

		int scaled_world_w = (int)((float)world_w * world_scale);
		int scaled_world_h = (int)((float)world_h * world_scale);
		world_offset_x *= (screen_w - scaled_world_w)/2;
		world_offset_y *= (screen_h - scaled_world_h)/2;

		SDL_Rect world_draw_rect = {
			world_offset_x, world_offset_y,
			scaled_world_w, scaled_world_h,
		};
		
		SDL_RenderCopy(renderer, world_buffer, 0, &world_draw_rect);
		draw_game_ui(game);

		Uint64 frequency = SDL_GetPerformanceFrequency();

		double time_elapsed = (double)(SDL_GetPerformanceCounter() - current_count) / (double)frequency * 1000.0;
		precise_delay(target_frame_time - time_elapsed);

		last_count = current_count;
		current_count = SDL_GetPerformanceCounter();
		SDL_RenderPresent(renderer);

//		double frame_time = (double)(current_count - last_count) / (double)frequency * 1000.0;
//		SDL_Log("Frame time: %.4fms", frame_time);
//		SDL_Log("FPS: %f", 1000.0 / frame_time);
	}

	SDL_Quit();	

	return 0;
}
