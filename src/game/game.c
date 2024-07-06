#include "SDL_gamecontroller.h"
#include "SDL_mixer.h"
#include "SDL_render.h"
#include "SDL_scancode.h"
#include "SDL_stdinc.h"
#include "../engine/assets.h"
#include "../engine/graphics.h"
#include "../engine/math.h"
#include "../engine/platform.h"
#include "../engine/ui.h"

#include "entities.h"
#include "game_types.h"
#include "score.h"

#include "entities.c"
#include "hud.c"
#include "score.c"

#define clamp(value, min, max) (value > max) ? max : (value < min) ? min : value;

#define SCENE_TRANSITION_TIME 45.0f
#define PAUSE_TRANSITION_TIME 15.0f
#define STARTING_LIVES 3
#define STAR_TWINKLE_INTERVAL 180.0f

static void spawn_player(Game_State* game) {
	game->player = 
		spawn_entity(
			game->entities, game->particle_system,
			ENTITY_TYPE_PLAYER,
			(Vector2){(float)game->world_w/2.0f, (float)game->world_h}
		);
	
	Entity* player = get_entity(game->entities, game->player);
	if (player) {
		player->type_data = PLAYER_WEAPON_MG;
	}
	game->player_state.ammo = 0;
	Mix_PlayChannel(-1, assets_get_sfx(game->assets, "Player Spawn"), 0);
}

void load_game_assets(Game_State* game) {
	if (game->assets == 0) {
		game->assets = new_game_assets();
	}

	assets_load_texture(game->assets, "assets/images/player.png", "Player Ship");

	// Projectiles
	assets_load_texture(game->assets, "assets/images/missile.png", "Projectile Missile");

	// Enemies
	assets_load_texture(game->assets, "assets/images/grappler_hook.png", "Grappler Hook");
	assets_load_texture(game->assets, "assets/images/grappler.png", "Enemy Grappler");
	assets_load_texture(game->assets, "assets/images/ufo.png", "Enemy UFO");
	SDL_SetTextureAlphaMod(assets_get_texture(game->assets, "Enemy UFO"), (Uint8)(255.0f * 0.7f));
	assets_load_texture(game->assets, "assets/images/tracker.png", "Enemy Tracker");
	assets_load_texture(game->assets, "assets/images/turret_base.png", "Enemy Turret Base");
	assets_load_texture(game->assets, "assets/images/turret_cannon.png", "Enemy Turret Cannon");

	// HUD
	assets_load_texture(game->assets, "assets/images/hud_missile.png", "HUD Missile");
	assets_load_texture(game->assets, "assets/images/hud_laser.png", "HUD Laser");
	assets_load_texture(game->assets, "assets/images/hud_mg.png", "HUD MG");

	//Generative textures
	// TODO: Figure why these seem to be destroyed on window resize (and fix it)
	assets_store_texture(game->assets, generate_item_texture(assets_get_texture(game->assets, "Projectile Missile")), "Item Missile");
	assets_store_texture(game->assets, generate_item_texture(assets_get_texture(game->assets, "Player Ship")), "Item LifeUp");
	SDL_Texture* laser_icon = generate_laser_icon();
	assets_store_texture(game->assets, generate_item_texture(laser_icon), "Item Laser");
	SDL_DestroyTexture(laser_icon);

	assets_load_music(game->assets, "assets/audio/music_wrapping_action.mp3", "Wrapping Action");
	assets_load_music(game->assets, "assets/audio/music_space_drifter.mp3", "Space Drifter");
	assets_load_music(game->assets, "assets/audio/music_game_over.mp3", "Game Over");

	assets_load_sfx(game->assets, "assets/audio/menu_confirm.mp3", "Menu Confirm");

	assets_load_sfx(game->assets, "assets/audio/weapon_pickup.mp3", "Weapon Pickup");
	assets_load_sfx(game->assets, "assets/audio/life_up.mp3", "Life Up");
	assets_load_sfx(game->assets, "assets/audio/player_shot.mp3", "Player Shot");
	assets_load_sfx(game->assets, "assets/audio/player_spawn.mp3", "Player Spawn");
	assets_load_sfx(game->assets, "assets/audio/player_laser.mp3", "Player Laser");
	assets_load_sfx(game->assets, "assets/audio/player_missile.mp3", "Player Missile");
	assets_load_sfx(game->assets, "assets/audio/player_death.mp3", "Player Death");
	
	assets_load_sfx(game->assets, "assets/audio/enemy_death.mp3", "Enemy Death");
	assets_load_sfx(game->assets, "assets/audio/turret_fire.mp3", "Turret Fire");
	assets_load_sfx(game->assets, "assets/audio/grappler_fire.mp3", "Grappler Fire");
	assets_load_sfx(game->assets, "assets/audio/hook_impact.mp3", "Hook Impact");

	Mix_VolumeChunk(assets_get_sfx(game->assets, "Player Laser"), 64);
	Mix_VolumeChunk(assets_get_sfx(game->assets, "Player Missile"), 64);
}

void init_game(Game_State* game) {
	game->entities = create_entity_system();

	game->fit_world_to_screen = 1;
	game->world_w = 800;
	game->world_h = 600;

	game->assets = new_game_assets();
	game->particle_system = new_particle_system();
	game->font = load_stbtt_font("assets/Orbitron-Regular.ttf", 64);

	load_game_assets(game);

	{ // Generate star field
		int stars_per_row = 20;
		int stars_per_column = STARFIELD_STAR_COUNT / stars_per_row;
		Vector2 star_offset = {game->world_w / stars_per_row, game->world_h / stars_per_column};
		Vector2 deviation = {star_offset.x / 1.5f, star_offset.y / 1.5f};
		Vector2 next_position = {0};
		for (int star_y = 0; star_y < stars_per_column; star_y++) {
			for (int star_x = 0; star_x < stars_per_row; star_x++) {
				int star_index = star_y * stars_per_row + star_x;
				
				game->starfield.positions[star_index].x = next_position.x + (deviation.x/2.0f) + (randomf() * deviation.x);
				game->starfield.positions[star_index].y = next_position.y + (deviation.y/2.0f) + (randomf() * deviation.y);
				
				game->starfield.colors[star_index].r = 100 + (uint8_t)(randomf() * 155.0f);
				game->starfield.colors[star_index].g = 100 + (uint8_t)(randomf() * 155.0f);
				game->starfield.colors[star_index].b = 100 + (uint8_t)(randomf() * 155.0f);

				game->starfield.timers[star_index] = randomf() * STAR_TWINKLE_INTERVAL;
				game->starfield.twinkle_direction[star_index] = (randomf() > 0.5f);
			
				next_position.x += star_offset.x;
			}
			next_position.x = 0.0f;
			next_position.y += star_offset.y;
		}
	}

	game->scene = -1;
	game->next_scene = GAME_SCENE_MAIN_MENU;
	game->scene_timer = game->scene_transition_time = SCENE_TRANSITION_TIME;

	game->player_controller = (Game_Player_Controller) {
		.thrust = {
			.key = SDL_SCANCODE_W,
			.button = SDL_CONTROLLER_BUTTON_DPAD_UP,
		},

		.thrust_left = {
			.key = SDL_SCANCODE_Q,
			.button = SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
		},
		
		.thrust_right= {
			.key = SDL_SCANCODE_E,
			.button = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
		},
		
		.turn_left = {
			.key = SDL_SCANCODE_A,
			.button = SDL_CONTROLLER_BUTTON_DPAD_LEFT,
		},

		.turn_right = {
			.key = SDL_SCANCODE_D,
			.button = SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
		},

		.fire = {
			.key = SDL_SCANCODE_SPACE,
			.button = SDL_CONTROLLER_BUTTON_A,
		},

		.menu = {
			.key = SDL_SCANCODE_P,
			.button = SDL_CONTROLLER_BUTTON_START,
		},
	};

	int* scores = get_score_table();
	if (scores) {
		SDL_memcpy(game->score.high_scores, scores, sizeof(int) * SCORE_TABLE_LENGTH);
		SDL_free(scores);
	}
	game->score.latest_score_index = -1;

	reset_entity_system(game->entities);
	game->enemy_count = 1;
}

void restart_game(Game_State* game) {
	game->enemy_count = 0;

	game->score = (Score_System){0};

	game->player_state.lives = STARTING_LIVES;
	game->player_state.ammo = 0;
	game->player_state.weapon_heat = 0;
	// TODO: Use "heat" units for thrust (like weapon heat) 
	game->player_state.thrust_energy = PLAYER_THRUST_MAX;

	spawn_player(game);
#if DEBUG && 0
	for (int i = ENTITY_TYPE_PLAYER+1; i < ENTITY_TYPE_SPAWN_WARP; i++) {
		Uint32 entity_id = spawn_entity(
			game->entities, game->particle_system, 
			ENTITY_TYPE_SPAWN_WARP,
			(Vector2){randomf() * (float)game->world_w, randomf() * (float)game->world_h}
		);
		Entity* entity = get_entity(game->entities, entity_id);
		if (entity) {
			entity->type_data = i;
		}
	}
#endif

}

void update_game(Game_State* game, Game_Input* input, float dt) {
#if DEBUG
	if (is_key_released(&game->input, SDL_SCANCODE_R)) {
		game->next_scene = GAME_SCENE_MAIN_MENU;
		game->scene_timer = game->scene_transition_time = SCENE_TRANSITION_TIME;
	}
#endif

	game->input = *input;
	if (game->next_scene == game->scene) {
		switch(game->scene) {
			case GAME_SCENE_MAIN_MENU: {
				if (is_game_control_pressed(&game->input, &game->player_controller.fire)) {
					Mix_PlayChannel(-1, assets_get_sfx(game->assets, "Menu Confirm"), 0);
					game->next_scene = GAME_SCENE_GAMEPLAY;
					game->scene_timer = game->scene_transition_time = SCENE_TRANSITION_TIME;
					despawn_entities(game->entities);
				}
			} break;
			
			case GAME_SCENE_GAMEPLAY: {
				update_score_timer(&game->score, dt);
				if (game->player_state.lives > 0) {
					if (!game->player && is_game_control_pressed(&game->input, &game->player_controller.fire)) {
						spawn_player(game);
						game->player_state.lives--;
					} 
				} else if (game->player == 0) {
					game->next_scene = GAME_SCENE_GAME_OVER;
					game->scene_timer = game->scene_transition_time = SCENE_TRANSITION_TIME;
				} 
				if (is_game_control_pressed(&game->input, &game->player_controller.menu)) {
					Mix_PauseMusic();
					Mix_PlayChannel(-1, assets_get_sfx(game->assets, "Menu Confirm"), 0);
					game->next_scene = GAME_SCENE_PAUSED;
					game->scene_timer = game->scene_transition_time = PAUSE_TRANSITION_TIME;
				}
			} break;

			case GAME_SCENE_PAUSED: {
				if (is_game_control_pressed(&game->input, &game->player_controller.menu)) {
					Mix_ResumeMusic();
					Mix_PlayChannel(-1, assets_get_sfx(game->assets, "Menu Confirm"), 0);
					game->next_scene = GAME_SCENE_GAMEPLAY;
					game->scene_timer = game->scene_transition_time = PAUSE_TRANSITION_TIME;
				}
			} break;
			
			case GAME_SCENE_GAME_OVER: {
				if (is_game_control_pressed(&game->input, &game->player_controller.fire)) {
					Mix_PlayChannel(-1, assets_get_sfx(game->assets, "Menu Confirm"), 0);
					game->score.latest_score_index = push_to_score_table(game->score.total);
					int* scores = get_score_table();
					if (scores) {
						SDL_memcpy(game->score.high_scores, scores, sizeof(int) * SCORE_TABLE_LENGTH);
						SDL_free(scores);
					}
					game->next_scene = GAME_SCENE_HIGH_SCORES;
					game->scene_timer = game->scene_transition_time = SCENE_TRANSITION_TIME;
				}
			} break;
			
			case GAME_SCENE_HIGH_SCORES: {
				if (is_game_control_pressed(&game->input, &game->player_controller.fire)) {
					Mix_PlayChannel(-1, assets_get_sfx(game->assets, "Menu Confirm"), 0);
					game->next_scene = GAME_SCENE_MAIN_MENU;
					game->scene_timer = game->scene_transition_time = SCENE_TRANSITION_TIME;

					despawn_entities(game->entities);
					game->enemy_count = 1; // Prevent the spawn system from triggering in menu

				}
			} break;
		}
	} else if (game->scene_timer > 0.0f) {
		game->scene_timer -= dt;
	} else {
		switch(game->next_scene) {
			case GAME_SCENE_MAIN_MENU: {
				Uint32 warp_id = spawn_entity(
					game->entities, game->particle_system, ENTITY_TYPE_SPAWN_WARP, (Vector2){game->world_w/2.0f, game->world_h});
				Entity* demo_warp = get_entity(game->entities, warp_id);
				if (demo_warp) {
					demo_warp->type_data = ENTITY_TYPE_DEMOSHIP;
				}
				Mix_PlayMusic(assets_get_music(game->assets, "Space Drifter"), -1);
			} break;

			case GAME_SCENE_GAMEPLAY: {				
				if (game->scene == GAME_SCENE_MAIN_MENU) {
					restart_game(game);
					Mix_PlayMusic(assets_get_music(game->assets, "Wrapping Action"), -1);
				}
			} break;
		
			case GAME_SCENE_GAME_OVER: {
				Mix_PauseMusic();
				Mix_PlayChannel(-1, assets_get_sfx(game->assets, "Game Over"), 0);
			} break;

			default: { break; }
		}

		game->scene = game->next_scene;
	}

	for (int star_index = 0; star_index < STARFIELD_STAR_COUNT; star_index++) {
		game->starfield.timers[star_index] += dt * (float)(1 - (2 * (int)game->starfield.twinkle_direction[star_index]));
		
		if (game->starfield.timers[star_index] >= STAR_TWINKLE_INTERVAL) game->starfield.twinkle_direction[star_index] = 1;
		else if (game->starfield.timers[star_index] <= 0) game->starfield.twinkle_direction[star_index] = 0;
	}

	if (game->scene != GAME_SCENE_PAUSED && game->next_scene != GAME_SCENE_PAUSED) {
		update_entities(game, dt);
		update_particles(game->particle_system, dt);
	}

	// TODO: Implement better conditions for this.
	// If there are as many dead entities as entities minus reserved 0 entity and player (if currently alive)
	if (game->enemy_count == 0) {
		game->score.current_wave++;
		game->score.spawn_points_max++;
		spawn_wave(game, game->score.current_wave, game->score.spawn_points_max);
	}
}

void draw_game_world(Game_State* game) {
	platform_set_render_draw_color(CLEAR_COLOR);
	platform_render_clear();
	
	for (int star_index = 0; star_index < STARFIELD_STAR_COUNT; star_index++) {
		float alpha = 255.0f * (game->starfield.timers[star_index] / STAR_TWINKLE_INTERVAL);
		RGBA_Color color = game->starfield.colors[star_index];
		color.a = (uint8_t)clamp(alpha, 0.0f, 255.0f);

		platform_set_render_draw_color(color);
		platform_render_draw_points(game->starfield.positions + star_index, 1);
	}

	draw_particles(game->particle_system, game->assets);
	draw_entities(game->entities, game->assets, game->world_w, game->world_h);
}

void draw_main_menu(Game_State* game, Rectangle bounds, float scale) {
	Vector2 origin = {
		.x = bounds.x+(bounds.w/2.0f),
		.y = bounds.y+(bounds.h/2.5f)
	};

	bool controller_enabled = (SDL_NumJoysticks() > 0);

	ui_element main_menu = new_ui_text(origin, 0, SD_BLUE, "Space Drifter", 64, "center");
	ui_element children[] = {
		new_ui_text(
			(Vector2){20, 50}, 0,(RGBA_Color){255, 165, 0, 255},  
			controller_enabled ? "Press START to begin!" : "Press FIRE to begin!",
			20, "center"
		),
		new_ui_rect((Vector2){-130, 75}, 0,(RGBA_Color){105, 105, 105, 128}, (Rectangle){0,0,260,145}),

		new_ui_text((Vector2){-110,100}, 0, WHITE, "<", 15, ""),
		new_ui_text((Vector2){-110,120}, 0, WHITE, ">", 15, ""),
		new_ui_text((Vector2){-110,140}, 0, WHITE, "^", 15, ""),

		new_ui_text((Vector2){-110,160}, 0, WHITE, controller_enabled ? "A" : "Spacebar", 15, ""),
		new_ui_text((Vector2){-110,180}, 0, WHITE, controller_enabled ? "LB" : "Q", 15, ""),
		new_ui_text((Vector2){-110,200}, 0, WHITE, controller_enabled ? "RB" : "E", 15, ""),
		
		new_ui_text((Vector2){110,100}, 0, WHITE, "Rotate Left", 15, "right"),
		new_ui_text((Vector2){110,120}, 0, WHITE, "Rotate Right", 15, "right"),
		new_ui_text((Vector2){110,140}, 0, WHITE, "Accelerate", 15, "right"),

		new_ui_text((Vector2){110,160}, 0, WHITE, "Fire", 15, "right"),
		new_ui_text((Vector2){110,180}, 0, WHITE, "Thrust Left", 15, "right"),
		new_ui_text((Vector2){110,200}, 0, WHITE, "Thrust Right", 15, "right"),
	};
	main_menu.children = children;
	main_menu.num_children = array_length(children);

	scale_ui_element(&main_menu, scale);
	main_menu.pos = origin;

	draw_ui_element(&main_menu, game->font);

	if (controller_enabled) {
		platform_set_render_draw_color((RGBA_Color){255, 0, 0, 255});
		render_text(game->font, 15, 8, platform_get_window_size().y - 8.0f, "Gamepad Enabled");
	}
}

void draw_scene_ui(Game_State* game, Game_Scene scene, float t) {
	iVector2 screen = platform_get_window_size();
	Rectangle screen_rect = {0,0,screen.x, screen.y};
	Rectangle world_rect = {0,0,game->world_w, game->world_h};
	if (game->fit_world_to_screen) {
		world_rect = fit_rect(world_rect, screen_rect);
	}
	world_rect = center_rect(world_rect, screen_rect);
	float scale = world_rect.w/(float)game->world_w;
	
	switch(scene) {
		case GAME_SCENE_MAIN_MENU: {
			draw_main_menu(game, world_rect, scale*t);
		} break;

		case GAME_SCENE_GAMEPLAY: {
			draw_HUD(game, world_rect, scale, 1.0f-t);
		} break;

		case GAME_SCENE_PAUSED: {
			ui_element paused = {0};
			Rectangle bg = {
				.x = -world_rect.w/2.0f,
				.y = -world_rect.h/2.0f,
				.w = world_rect.w,
				.h = world_rect.h
			};
			ui_element children[] = {
				new_ui_rect(
					(Vector2){0}, 0, 
					(RGBA_Color){0,0,0,120},
					bg
				),
				new_ui_text(
					(Vector2) {0},
					0, ORANGE, "PAUSED", 64, "center"
				),
			};
			paused.children = children;
			paused.num_children = 2;

			scale_ui_element(&paused, scale*t);
			paused.pos = (Vector2) {
				.x = screen_rect.w/2,
				.y = screen_rect.h/2
			};
			draw_ui_element(&paused, game->font);
		} break;

		case GAME_SCENE_GAME_OVER: {
			ui_element game_over = {0};
			ui_element children[] = {
				new_ui_text((Vector2){0, -64}, 0, RED, "GAME_OVER", 64, "center"),
				new_ui_text((Vector2){0,   0}, 0, WHITE, "Final Score", 48, "center"),
				new_ui_text((Vector2){0,  48}, &(game->score.total), WHITE, "", 32, "center"),
			};
			game_over.children = children;
			game_over.num_children = array_length(children);

			scale_ui_element(&game_over, scale*t);
			game_over.pos = (Vector2){world_rect.x+(world_rect.w/2.0f), world_rect.y+(world_rect.h/2.0f)},
			draw_ui_element(&game_over, game->font);
		} break;

		case GAME_SCENE_HIGH_SCORES: {
			int* scores = game->score.high_scores;

			ui_element high_score = new_ui_text((Vector2){0}, NULL, WHITE, "HIGH_SCORES", 48, "center");
			ui_element score_list[SCORE_TABLE_LENGTH];
			high_score.children = score_list;
			high_score.num_children = array_length(score_list);

			float padding = 40;
			Vector2 pos = {0, padding};
			for (int i = 0; i < array_length(score_list); i++) {
				if (i == game->score.latest_score_index) {
					const char* label = "New!  ";
					char buf[32];
					SDL_itoa(scores[i], buf, 10);
					float width = (float)measure_text(game->font, 32, buf)/2.0f;
					width += (float)measure_text(game->font, 32, label);

					score_list[i] = new_ui_text(
						(Vector2){pos.x-width, pos.y},
						(scores+i), 
						ORANGE, (char*)label, 32, "left"
					);
				} else {
					score_list[i] = new_ui_text(pos, (scores+i), WHITE, "", 32, "center");
				}
				pos.y += padding;
			}

			scale_ui_element(&high_score, scale*t);
			high_score.pos = (Vector2) {
				world_rect.x+(world_rect.w/2.0f),
				world_rect.y+(world_rect.h/4.0f)
			};
			draw_ui_element(&high_score, game->font);
		} break;
		
		default: {}
	}
}

void draw_game_ui(Game_State* game) {
	float t = game->scene != game->next_scene
		? game->scene_timer/game->scene_transition_time
		: 1.0f;
	draw_scene_ui(game, game->scene, smooth_start(t, 3));

	if (game->scene != game->next_scene) {
		draw_scene_ui(game, game->next_scene, smooth_stop(1.0f-t, 2));
	}
}
