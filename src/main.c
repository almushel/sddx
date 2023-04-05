#define DEBUG 1
#include "defs.h"
#include "graphics.c"
#include "assets.c"
#include "particles.c"
#include "entities.c"
#include "hud.c"
#include "score.c"

void process_key_event(SDL_KeyboardEvent* event, Game_Controller_State* input) {
	if (event->repeat == 0) {
		switch (event->keysym.scancode) {
			case SDL_SCANCODE_ESCAPE: {
				SDL_Event quit_event;
				quit_event.type = SDL_QUIT;
				SDL_PushEvent(&quit_event);
			} return;
		}

		Game_Button_State* button;
		for (int i = 0; i < array_length(input->list); i++) {
			button = input->list + i;
			if (button->scan_code == event->keysym.scancode) {
				button->pressed = event->state == SDL_PRESSED;
				button->released = event->state == SDL_RELEASED;
			}
		}
	}
}

void load_game_assets(Game_State* game) {
	game_load_texture(game, "assets/images/player.png", "Player Ship");
	game_load_texture(game, "assets/images/grappler_hook.png", "Grappler Hook");
	game_load_texture(game, "assets/images/grappler.png", "Enemy Grappler");
	game_load_texture(game, "assets/images/missile.png", "Projectile Missile");
	game_load_texture(game, "assets/images/ufo.png", "Enemy UFO");
	SDL_SetTextureAlphaMod(game_get_texture(game, "Enemy UFO"), (Uint8)(255.0f * 0.7f));
	game_load_texture(game, "assets/images/tracker.png", "Enemy Tracker");
	game_load_texture(game, "assets/images/turret_base.png", "Enemy Turret Base");
	game_load_texture(game, "assets/images/turret_cannon.png", "Enemy Turret Cannon");
	game_load_texture(game, "assets/images/hud_missile.png", "HUD Missile");
	game_load_texture(game, "assets/images/hud_laser.png", "HUD Laser");
	game_load_texture(game, "assets/images/hud_mg.png", "HUD MG");

	//Generative textures
	game_store_texture(game, generate_drifter_texture(game), "Enemy Drifter");
	game_store_texture(game, generate_item_texture(game, game_get_texture(game, "Projectile Missile")), "Item Missile");
	game_store_texture(game, generate_item_texture(game, game_get_texture(game, "Player Ship")), "Item LifeUp");
	game_store_texture(game, generate_item_texture(game, 0), "Item Laser");

	if (Mix_OpenAudio(48000, AUDIO_S16SYS, 2, 4096) != -1) {
		Mix_AllocateChannels(32);
		game_load_music(game, "assets/audio/WrappingAction.mp3", "Wrapping Action");
		game_load_sfx(game, "assets/audio/PlayerShot.mp3", "Player Shot");
		game_load_sfx(game, "assets/audio/PlayerSpawn.mp3", "Player Spawn");
//		game_load_sfx(game, "assets/audio/WeaponPickup.mp3", "Weapon Pickup");
//		game_load_sfx(game, "assets/audio/PlayerLaser.mp3", "Player Laser");
//		game_load_sfx(game, "assets/audio/PlayerMissile.mp3", "Player Missile");
	} else {
		SDL_Log(SDL_GetError());
		exit(1);
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

	int screen_w = 1280;
	int screen_h = 720;

	SDL_Init(SDL_INIT_EVERYTHING);

	Game_State* game = SDL_malloc(sizeof(Game_State));
	SDL_memset(game, 0, sizeof(Game_State));

	game->entities_size = 512;
	game->entities = SDL_malloc(sizeof(Entity) * game->entities_size);
	SDL_memset(game->entities, 0, sizeof(Entity) * game->entities_size);

	game->window = SDL_CreateWindow("Space Drifter DX", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_w, screen_h, SDL_WINDOW_RESIZABLE);
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

	game->world_w = 800;
	game->world_h = 600;
	game->world_buffer = SDL_CreateTexture(game->renderer, SDL_PIXELFORMAT_ARGB32, SDL_TEXTUREACCESS_TARGET, game->world_w, game->world_h);
	if (!game->world_buffer) {
		SDL_Log("Creating world buffer failed. %s", SDL_GetError());
	}
	game->DEBUG_fit_world_to_screen = 1;

	load_game_assets(game);
	game->font = load_stbtt_font(game->renderer, "c:/windows/fonts/times.ttf", 32);
	Mix_PlayMusic(game_get_music(game, "Wrapping Action"), -1);

	{ // Generate star field
		int stars_per_row = 20;
		int stars_per_column = STARFIELD_STAR_COUNT / stars_per_row;
		Vector2 star_offset = {game->world_w / stars_per_row, game->world_h / stars_per_column};
		Vector2 deviation = {star_offset.x / 1.5f, star_offset.y / 1.5f};
		Vector2 next_position = {0};
		for (int star_y = 0; star_y < stars_per_column; star_y++) {
			for (int star_x = 0; star_x < stars_per_row; star_x++) {
				int star_index = star_y * stars_per_row + star_x;
				
				game->starfield.positions[star_index].x = next_position.x + (deviation.x/2.0f) + (random() * deviation.x);
				game->starfield.positions[star_index].y = next_position.y + (deviation.y/2.0f) + (random() * deviation.y);
				
				game->starfield.colors[star_index].r = 100 + (uint8_t)(random() * 155.0f);
				game->starfield.colors[star_index].g = 100 + (uint8_t)(random() * 155.0f);
				game->starfield.colors[star_index].b = 100 + (uint8_t)(random() * 155.0f);

				game->starfield.timers[star_index] = random() * STAR_TWINKLE_INTERVAL;
				game->starfield.twinkle_direction[star_index] = (random() > 0.5f);
			
				next_position.x += star_offset.x;
			}
			next_position.x = 0.0f;
			next_position.y += star_offset.y;
		}

	}
	game->player_controller = (Game_Controller_State){
		.thrust.scan_code = SDL_SCANCODE_W,
		.thrust_left.scan_code = SDL_SCANCODE_E,
		.thrust_right.scan_code = SDL_SCANCODE_Q,
		.turn_left.scan_code = SDL_SCANCODE_A,
		.turn_right.scan_code = SDL_SCANCODE_D,
		.fire.scan_code = SDL_SCANCODE_SPACE,
	};

	Game_Controller_State new_player_controller = {0};
	
	get_new_entity(game); // reserve 0
	for (int i = ENTITY_TYPE_PLAYER+1; i < ENTITY_TYPE_SPAWN_WARP; i++) {
		Uint32 entity_id = spawn_entity(game, ENTITY_TYPE_SPAWN_WARP, (Vector2){random() * (float)game->world_w, random() * (float)game->world_h});
		if (entity_id) {
			Entity* entity = get_entity(game, entity_id);
			entity->type_data = ENTITY_TYPE_ENEMY_DRIFTER; //i;
		}
	}

//	spawn_entity(game, ENTITY_TYPE_ENEMY_GRAPPLER, (Vector2){random() * (float)game->world_w, random() * (float)game->world_h});

	game->player_state.current_weapon = PLAYER_WEAPON_MG;
	game->player_state.lives = 3;
	game->player_state.ammo = 0;
	game->player_state.weapon_heat = 0;
	game->player_state.thrust_energy = THRUST_MAX;

	double target_fps = (double)TARGET_FPS;
	double target_frame_time = 1000.0/target_fps;
	Uint64 last_count = SDL_GetPerformanceCounter();
	Uint64 current_count = last_count;

	SDL_bool running = SDL_TRUE;
	while (running) {
		double dt = (double)(current_count - last_count) / (double)SDL_GetPerformanceFrequency();
		dt = dt/(1.0 / (double)TICK_RATE);

		new_player_controller = (Game_Controller_State) {
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
#if DEBUG					
					if (event.key.keysym.scancode == SDL_SCANCODE_EQUALS) {
						game->DEBUG_fit_world_to_screen = !game->DEBUG_fit_world_to_screen;
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_GRAVE) {
						SDL_Log("Break");
					}
					else
#endif
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

		Game_Button_State* current_button,* new_button;
		for (int i = 0; i < array_length(new_player_controller.list); i++) {
			new_button = new_player_controller.list + i;
			current_button = game->player_controller.list + i;

			current_button->pressed = (!current_button->held && new_button->pressed);
			current_button->released = (current_button->held && new_button->released);

			if (new_button->pressed) current_button->held = SDL_TRUE;
			else if (new_button->released) current_button->held = SDL_FALSE;
		}

		if (!game->player 						&& 
			game->player_controller.fire.held	&&
			game->player_state.lives > 0
			) {
			game->player = get_entity(game,
				spawn_entity(game, ENTITY_TYPE_PLAYER, (Vector2){(float)(float)game->world_w/2.0f, (float)(float)game->world_h})
			);
			Mix_PlayChannel(-1, game_get_sfx(game, "Player Spawn"), 0);
			//game->player_state.lives--;
		}

		update_score_timer(&game->score, dt);
		update_entities(game, dt);
		update_particle_emitters(&game->particle_system, dt);
		update_particles(&game->particle_system, dt);

		SDL_SetRenderDrawColor(game->renderer,0,0,0,0);
		SDL_RenderClear(game->renderer);

		// Start world draw
		SDL_SetRenderTarget(game->renderer, game->world_buffer);
		SDL_SetRenderDrawColor(game->renderer,CLEAR_COLOR.r,CLEAR_COLOR.g,CLEAR_COLOR.b,255);
		SDL_RenderClear(game->renderer);
		SDL_SetRenderDrawBlendMode(game->renderer, SDL_BLENDMODE_BLEND);
		for (int star_index = 0; star_index < STARFIELD_STAR_COUNT; star_index++) {
			game->starfield.timers[star_index] += dt * (float)(1 - (2 * (int)game->starfield.twinkle_direction[star_index]));
			
			if (game->starfield.timers[star_index] >= STAR_TWINKLE_INTERVAL) game->starfield.twinkle_direction[star_index] = 1;
			else if (game->starfield.timers[star_index] <= 0) game->starfield.twinkle_direction[star_index] = 0;

			float alpha = SDL_clamp(255.0f * (game->starfield.timers[star_index] / STAR_TWINKLE_INTERVAL), 0.0f, 255.0f);

			SDL_SetRenderDrawColor(game->renderer, 
				game->starfield.colors[star_index].r,
				game->starfield.colors[star_index].g, 
				game->starfield.colors[star_index].b, 
				(Uint8)alpha
			);
			SDL_RenderDrawPointF(game->renderer, game->starfield.positions[star_index].x, game->starfield.positions[star_index].y);
		}
		SDL_SetRenderDrawBlendMode(game->renderer, SDL_BLENDMODE_NONE);
		draw_particles(game, game->renderer);
		draw_entities(game);
		// End of world draw

		SDL_SetRenderTarget(game->renderer, 0);
		SDL_GetWindowSizeInPixels(game->window, &screen_w, &screen_h);

		float world_scale = 1;
		int world_offset_x = 1, world_offset_y = 1;

#if DEBUG
		if (game->DEBUG_fit_world_to_screen) {
			SDL_SetTextureScaleMode(game->world_buffer, SDL_ScaleModeBest);
			if (screen_h < screen_w) {
				world_scale = (float)screen_h / (float)game->world_h;
				world_offset_y = 0;
			} else {
				world_scale = (float)screen_w / (float)game->world_w;
				world_offset_x = 0;
			}
		} else {
			SDL_SetTextureScaleMode(game->world_buffer, SDL_ScaleModeNearest);
		}
#endif
		int scaled_world_w = (int)((float)game->world_w * world_scale);
		int scaled_world_h = (int)((float)game->world_h * world_scale);
		world_offset_x *= (screen_w - scaled_world_w)/2;
		world_offset_y *= (screen_h - scaled_world_h)/2;

		SDL_Rect world_draw_rect = {
			world_offset_x, world_offset_y,
			scaled_world_w, scaled_world_h,
		};
		
		SDL_RenderCopy(game->renderer, game->world_buffer, 0, &world_draw_rect);
		draw_HUD(game);

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