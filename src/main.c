#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "defs.h"
#include "assets.c"
#include "particles.c"

void draw_texture(SDL_Renderer* renderer, SDL_Texture* texture, float x, float y, float angle, SDL_bool centered) {
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

void process_key_event(SDL_KeyboardEvent* event, game_controller_state* input) {
	if (event->repeat == 0) {
		switch (event->keysym.scancode) {
			case SDL_SCANCODE_ESCAPE: {
				SDL_Event quit_event;
				quit_event.type = SDL_QUIT;
				SDL_PushEvent(&quit_event);
			} return;
		}

		game_button_state* button;
		for (int i = 0; i < array_length(input->list); i++) {
			button = input->list + i;
			if (button->scan_code == event->keysym.scancode) {
				button->pressed = event->state == SDL_PRESSED;
				button->released = event->state == SDL_RELEASED;
			}
		}
	}
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

	SDL_Init(SDL_INIT_EVERYTHING);

	Game_State* game = SDL_malloc(sizeof(Game_State));
	SDL_memset(game, 0, sizeof(Game_State));

	game->window = SDL_CreateWindow("Space Drifter DX", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	if (game->window == NULL) {
		SDL_LogError(0, SDL_GetError());
		exit(1);
	}

	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

	game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); 
	if (game->renderer == NULL) {
		SDL_LogError(0, SDL_GetError());
		exit(1);
	}

	game->player_controller = (game_controller_state){0};
	
	game->player_controller.thrust.scan_code = SDL_SCANCODE_W;
	game->player_controller.thrust_left.scan_code = SDL_SCANCODE_E;
	game->player_controller.thrust_right.scan_code = SDL_SCANCODE_Q;
	game->player_controller.turn_left.scan_code = SDL_SCANCODE_A;
	game->player_controller.turn_right.scan_code = SDL_SCANCODE_D;
	game->player_controller.fire.scan_code = SDL_SCANCODE_SPACE;

	game_controller_state new_player_controller = {0};
	
	game_load_texture(game, "assets/images/player.png", "Player Ship");

	if (Mix_OpenAudio(48000, AUDIO_S16SYS, 2, 2048) != -1) {
		Mix_AllocateChannels(16);
		game_load_music(game, "assets/audio/WrappingAction.mp3", "Wrapping Action");
		game_load_sfx(game, "assets/audio/PlayerShot.mp3", "Player Shot");
	} else {
		SDL_Log(SDL_GetError());
		exit(1);
	}

	Mix_PlayMusic(game_get_music(game, "Wrapping Action"), -1);


	game->player = (Entity){0};
	game->player.x = SCREEN_WIDTH / 2;
	game->player.y = SCREEN_HEIGHT / 2;
	game->player.angle = 270;
	game->player.sprite.texture = game_get_texture(game, "Player Ship");

	Particle_Emitter* main_thruster = get_particle_emitter(&game->particle_system);
	*main_thruster = (Particle_Emitter){0};
	main_thruster->parent = &game->player;
	main_thruster->angle = 180;
	main_thruster->density = 2.0f;
	main_thruster->colors[0] = SD_BLUE;
	main_thruster->color_count = 1;

	Particle_Emitter* left_thruster = get_particle_emitter(&game->particle_system);
	*left_thruster = *main_thruster;
	left_thruster->angle = 90.0f;
	Particle_Emitter* right_thruster = get_particle_emitter(&game->particle_system);
	*right_thruster = *main_thruster;
	right_thruster->angle = -90.0f;


	double target_fps = (double)TARGET_FPS;
	double target_frame_time = 1000.0/target_fps;
	Uint64 last_count = SDL_GetPerformanceCounter();
	Uint64 current_count = last_count;

	SDL_bool running = SDL_TRUE;
	while (running) {
		double dt = (double)(current_count - last_count) / (double)SDL_GetPerformanceFrequency();
		dt = dt/(1.0 / (double)TICK_RATE);

		new_player_controller = (game_controller_state) {
			.thrust.scan_code 		= 	game->player_controller.thrust.scan_code,
			.thrust_left.scan_code 	= 	game->player_controller.thrust_left.scan_code,
			.thrust_right.scan_code = 	game->player_controller.thrust_right.scan_code,
			.turn_left.scan_code 	= 	game->player_controller.turn_left.scan_code,
			.turn_right.scan_code 	= 	game->player_controller.turn_right.scan_code,
			.fire.scan_code 		= 	game->player_controller.fire.scan_code,
		};
	
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYUP: {
					process_key_event(&event.key, &new_player_controller);
				} break;
				case SDL_KEYDOWN: {
					process_key_event(&event.key, &new_player_controller);
				} break;

				case SDL_QUIT: {
					running = SDL_FALSE;
				} break;
			}
		}
		if (!running) break;

		game_button_state* current_button,* new_button;
		for (int i = 0; i < array_length(new_player_controller.list); i++) {
			new_button = new_player_controller.list + i;
			current_button = game->player_controller.list + i;

			current_button->pressed = (!current_button->held && new_button->pressed);
			current_button->released = (current_button->held && new_button->released);

			if (new_button->pressed) current_button->held = SDL_TRUE;
			else if (new_button->released) current_button->held = SDL_FALSE;
		}

		Entity* player = &game->player;

		if (game->player_controller.turn_left.held) player->angle -= PLAYER_TURN_SPEED * dt;
		if (game->player_controller.turn_right.held) player->angle += PLAYER_TURN_SPEED * dt;
		
		if (game->player_controller.thrust.held) {
			player->vx += cos_deg(player->angle) * PLAYER_FORWARD_THRUST * dt;
			player->vy += sin_deg(player->angle) * PLAYER_FORWARD_THRUST * dt;
		}
		main_thruster->active = game->player_controller.thrust.held;

		if (game->player_controller.thrust_left.held) {
			player->vx += cos_deg(player->angle + 90) * PLAYER_LATERAL_THRUST * dt;
			player->vy += sin_deg(player->angle + 90) * PLAYER_LATERAL_THRUST * dt;
		}
		right_thruster->active = game->player_controller.thrust_left.held;
		
		if (game->player_controller.thrust_right.held) {
			player->vx += cos_deg(player->angle - 90) * PLAYER_LATERAL_THRUST * dt;
			player->vy += sin_deg(player->angle - 90) * PLAYER_LATERAL_THRUST * dt;
		}
		left_thruster->active = game->player_controller.thrust_right.held;
		
		if (game->player_controller.fire.pressed)  {
			explode_sprite(&game->particle_system, &player->sprite, player->x, player->y, player->angle, 6);
			//explode_at_point(&game->particle_system, player->x, player->y, 0, 0, 1, 0, 0);
			Mix_PlayChannel(-1, game_get_sfx(game, "Player Shot"), 0);
		}
		player->x += player->vx * dt;
		player->y += player->vy * dt;

		player->vx *= 1.0 - (PHYSICS_FRICTION*dt);
		player->vy *= 1.0 - (PHYSICS_FRICTION*dt);

		if (player->x < 0) player->x = SCREEN_WIDTH + player->x;
		else if (player->x > SCREEN_WIDTH) player->x -= SCREEN_WIDTH;

		if (player->y < 0) player->y = SCREEN_HEIGHT + player->y;
		else if (player->y > SCREEN_HEIGHT) player->y -= SCREEN_HEIGHT;

		update_particle_emitters(&game->particle_system, dt);
		update_particles(&game->particle_system, dt);

		SDL_SetRenderDrawColor(game->renderer,CLEAR_COLOR.r,CLEAR_COLOR.g,CLEAR_COLOR.b,255);	
		SDL_RenderClear(game->renderer);

		draw_particles(game, game->renderer);
		draw_game_sprite(game, &player->sprite, player->transform, 1);

		Uint64 frequency = SDL_GetPerformanceFrequency();

		double time_elapsed = (double)(SDL_GetPerformanceCounter() - current_count) / (double)frequency * 1000.0;
		precise_delay(target_frame_time - time_elapsed);

		last_count = current_count;
		current_count = SDL_GetPerformanceCounter();
		SDL_RenderPresent(game->renderer);

//		double frame_time = (double)(current_count - last_count) / (double)frequency * 1000.0;
//		SDL_Log("Frame time: %.4fms", frame_time);
//		SDL_Log("FPS: %f", 1000.0 / frame_time);
	}

	SDL_Quit();	

	return 0;
}