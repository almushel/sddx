#include "defs.h"
#include "platform.h"
#include "game_input.c"
#include "assets.h"
#include "graphics.c"
#include "particles.c"
#include "entities.c"
#include "hud.c"
#include "score.c"

#define clamp(value, min, max) (value > max) ? max : (value < min) ? min : value;

#define SCENE_TRANSITION_TIME 30.0f
#define STARTING_LIVES 1

static Game_Scene current_scene;
static Game_Scene next_scene;
static float scene_transition_timer;

static void switch_game_scene(Game_Scene new_scene) {
	next_scene = new_scene;
	scene_transition_timer = SCENE_TRANSITION_TIME;
}

static void spawn_player(Game_State* game) {
	game->player = get_entity(game,
		spawn_entity(game, ENTITY_TYPE_PLAYER, (Vector2){(float)(float)game->world_w/2.0f, (float)(float)game->world_h})
	);
	
	game->player->type_data = PLAYER_WEAPON_MG;
	game->player_state.ammo = 0;
	Mix_PlayChannel(-1, game_get_sfx(game, "Player Spawn"), 0);
}

void load_game_assets(Game_State* game) {
	game_load_texture(game, "assets/images/player.png", "Player Ship");

	// Projectiles
	game_load_texture(game, "assets/images/missile.png", "Projectile Missile");

	// Enemies
	game_load_texture(game, "assets/images/grappler_hook.png", "Grappler Hook");
	game_load_texture(game, "assets/images/grappler.png", "Enemy Grappler");
	game_load_texture(game, "assets/images/ufo.png", "Enemy UFO");
//	SDL_SetTextureAlphaMod(game_get_texture(game, "Enemy UFO"), (Uint8)(255.0f * 0.7f));
	game_load_texture(game, "assets/images/tracker.png", "Enemy Tracker");
	game_load_texture(game, "assets/images/turret_base.png", "Enemy Turret Base");
	game_load_texture(game, "assets/images/turret_cannon.png", "Enemy Turret Cannon");

	// HUD
	game_load_texture(game, "assets/images/hud_missile.png", "HUD Missile");
	game_load_texture(game, "assets/images/hud_laser.png", "HUD Laser");
	game_load_texture(game, "assets/images/hud_mg.png", "HUD MG");

	//Generative textures
	game_store_texture(game, generate_item_texture(game, game_get_texture(game, "Projectile Missile")), "Item Missile");
	game_store_texture(game, generate_item_texture(game, game_get_texture(game, "Player Ship")), "Item LifeUp");
	game_store_texture(game, generate_item_texture(game, 0), "Item Laser");

	game_load_music(game, "assets/audio/WrappingAction.mp3", "Wrapping Action");
	game_load_sfx(game, "assets/audio/PlayerShot.mp3", "Player Shot");
	game_load_sfx(game, "assets/audio/PlayerSpawn.mp3", "Player Spawn");
	game_load_sfx(game, "assets/audio/WeaponPickup.mp3", "Weapon Pickup");
	game_load_sfx(game, "assets/audio/PlayerLaser.mp3", "Player Laser");
	game_load_sfx(game, "assets/audio/PlayerMissile.mp3", "Player Missile");
}

static void reset_game(Game_State* game) {
	game->entity_count = 1;
	game->particle_system.particle_count = 0;
	game->particle_system.dead_emitter_count = 0;
	game->particle_system.emitter_count = 1;
}

void init_game(Game_State* game) {
	game->world_w = 800;
	game->world_h = 600;

	game->font = load_stbtt_font("c:/windows/fonts/times.ttf", 64);
	load_game_assets(game);
//	Mix_PlayMusic(game_get_music(game, "Wrapping Action"), -1);

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
	};

	reset_game(game);
	spawn_entity(game, ENTITY_TYPE_DEMOSHIP, (Vector2){game->world_w/2.0f, game->world_h/2.0f});
}

void restart_game(Game_State* game) {
	game->player_state.lives = STARTING_LIVES;
	game->player_state.ammo = 0;
	game->player_state.weapon_heat = 0;
	// TO-DO: Use "heat" units for thrust (like weapon heat) 
	game->player_state.thrust_energy = PLAYER_THRUST_MAX;

	spawn_player(game);

#if 0
	for (int i = ENTITY_TYPE_PLAYER+1; i < ENTITY_TYPE_SPAWN_WARP; i++) {
		Uint32 entity_id = spawn_entity(game, ENTITY_TYPE_SPAWN_WARP, (Vector2){random() * (float)game->world_w, random() * (float)game->world_h});
		if (entity_id) {
			Entity* entity = get_entity(game, entity_id);
			entity->type_data = i;
		}
	}
#endif

}

void update_game(Game_State* game, float dt) {
	if (next_scene == current_scene) {
		switch(current_scene) {
			case GAME_SCENE_MAIN_MENU: {
				if (is_game_control_pressed(&game->input, &game->player_controller.fire)) {
					switch_game_scene(GAME_SCENE_GAMEPLAY);
				}
			} break;
			
			case GAME_SCENE_GAMEPLAY: {
				if (game->player_state.lives > 0) {
					if (!game->player && is_game_control_pressed(&game->input, &game->player_controller.fire)) {
						spawn_player(game);
						game->player_state.lives--;
					} 
				} else if (!game->player) {
					switch_game_scene(GAME_SCENE_GAME_OVER);
				}
			} break;
			
			case GAME_SCENE_GAME_OVER: {
				if (is_game_control_pressed(&game->input, &game->player_controller.fire)) {
					switch_game_scene(GAME_SCENE_HIGH_SCORES);
				}
			} break;
			
			case GAME_SCENE_HIGH_SCORES: {
				if (is_game_control_pressed(&game->input, &game->player_controller.fire)) {
					switch_game_scene(GAME_SCENE_MAIN_MENU);
				}
			} break;
		}
	} else if (scene_transition_timer > 0.0f) {
		scene_transition_timer -= dt;
	} else { // NOTE: Do I want to trigger this when the switch starts ( in switch_game_scene() )?
		switch(next_scene) {
			case GAME_SCENE_MAIN_MENU: {
//	Despawn does not work as expected for asteroids because violent death and "despawn" death are not
//	differentiated when dead entities are removed
//				for (int i = 1; i < game->entity_count; i++) {
//					game->entities[i].state = ENTITY_STATE_DESPAWNING;
//					game->entities[i].timer = 30.0f;
//				}
				reset_game(game);
				spawn_entity(game, ENTITY_TYPE_DEMOSHIP, (Vector2){game->world_w/2.0f, game->world_h/2.0f});
			} break;

			case GAME_SCENE_GAMEPLAY: {				
				for (int i = 1; i < game->entity_count; i++) {
					game->entities[i].state = ENTITY_STATE_DESPAWNING;
					game->entities[i].timer = 30.0f;
				}

				restart_game(game);
			} break;
		}

		current_scene = next_scene;
	}

	for (int star_index = 0; star_index < STARFIELD_STAR_COUNT; star_index++) {
		game->starfield.timers[star_index] += dt * (float)(1 - (2 * (int)game->starfield.twinkle_direction[star_index]));
		
		if (game->starfield.timers[star_index] >= STAR_TWINKLE_INTERVAL) game->starfield.twinkle_direction[star_index] = 1;
		else if (game->starfield.timers[star_index] <= 0) game->starfield.twinkle_direction[star_index] = 0;
	}

	update_score_timer(&game->score, dt);
	update_entities(game, dt);
	update_particle_emitters(&game->particle_system, dt);
	update_particles(&game->particle_system, dt);

	poll_input(&game->input); // Clear held and released states
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

	draw_particles(game);
	draw_entities(game);
}

void draw_main_menu(Game_State* game) {
	Vector2 offset = platform_get_window_size();
	offset.x /= 2.0f;
	offset.y /= 2.5f;

	bool controller_enabled = (SDL_NumJoysticks() > 0);

//		ctx.shadowBlur = 10;
//		ctx.shadowColor = 'black';
	platform_set_render_draw_color(SD_BLUE);
	render_text_aligned(game->font, 64, offset.x, offset.y, "Space Drifter", "center");
//		ctx.shadowBlur = 10;
//		ctx.shadowColor = 'red';
	char* press_start_str = controller_enabled ? "Press ENTER to begin!" : "Press START to begin!";
	platform_set_render_draw_color((RGBA_Color){255, 165, 0, 255});
	render_text_aligned(game->font, 20, offset.x, offset.y + 50, press_start_str, "center");

	platform_set_render_draw_color((RGBA_Color){105, 105, 105, 128});
	platform_render_fill_rect((Rectangle){offset.x - 130, offset.y + 75, 260, 145});

//		ctx.font = '15px Orbitron';
	platform_set_render_draw_color((RGBA_Color){255,255,255,255});

	offset.x -= 110;
	render_text(game->font, 15, offset.x, offset.y + 100, "<");
	render_text(game->font, 15, offset.x, offset.y + 120, ">");
	render_text(game->font, 15, offset.x, offset.y + 140, "^");

	if (controller_enabled) {
		render_text(game->font, 15, offset.x, offset.y + 160,"A" );
		render_text(game->font, 15, offset.x, offset.y + 180,"LB");
		render_text(game->font, 15, offset.x, offset.y + 200,"RB");
	} else {
		render_text(game->font, 15, offset.x, offset.y + 160, "Spacebar");
		render_text(game->font, 15, offset.x, offset.y + 180, "Q");
		render_text(game->font, 15, offset.x, offset.y + 200, "E");
	}

//		ctx.textAlign = "right";
	offset.x += 220;
	render_text_aligned(game->font, 15, offset.x, offset.y + 100,	"Rotate Left"	, "right");
	render_text_aligned(game->font, 15, offset.x, offset.y + 120,	"Rotate Right"	, "right");
	render_text_aligned(game->font, 15, offset.x, offset.y + 140,	"Accelerate"	, "right");
	render_text_aligned(game->font, 15, offset.x, offset.y + 160,	"Fire"			, "right");
	render_text_aligned(game->font, 15, offset.x, offset.y + 180,	"Thrust Left"	, "right");
	render_text_aligned(game->font, 15, offset.x, offset.y + 200,	"Thrust Right"	, "right");

	if (controller_enabled) {
		platform_set_render_draw_color((RGBA_Color){255, 0, 0, 255});
		render_text(game->font, 15, 8, platform_get_window_size().y - 8.0f, "Gamepad Enabled");
	}
}

void draw_scene_ui(Game_State* game, Game_Scene scene) {
	switch(scene) {
		case GAME_SCENE_MAIN_MENU: {
			draw_main_menu(game);
		} break;

		case GAME_SCENE_GAMEPLAY: {
			draw_HUD(game);
		} break;

		case GAME_SCENE_GAME_OVER: {
			char score_buf[24];
			SDL_itoa(game->score.total, score_buf, 10);
			Vector2 screen = platform_get_window_size();
			Vector2 offset = {screen.x/2.0f, screen.y/2.0f - 96};

			platform_set_render_draw_color(RED);
			render_text_aligned(game->font, 64, offset.x, offset.y - 96, "GAME OVER", "center");
			offset.y += 64;
			platform_set_render_draw_color(WHITE);
			render_text_aligned(game->font, 48, offset.x, offset.y, "Final Score:", "center");
			offset.y += 48;
			render_text_aligned(game->font, 32, offset.x, offset.y, score_buf, "center");
		} break;

		case GAME_SCENE_HIGH_SCORES: {
			char score_buf[24];
			SDL_itoa(game->score.total, score_buf, 10);
			Vector2 screen = platform_get_window_size();
			Vector2 offset = {screen.x/2.0f, screen.y/4.0f};
			platform_set_render_draw_color(WHITE);
			render_text_aligned(game->font, 48, offset.x, offset.y, "HIGH SCORES", "center");
			offset.y += 64.0f;

			Vector2 line[2] = {
				{screen.x*0.25f, offset.y},
				{screen.x*0.75f, offset.y}
			};
			for (int i = 0; i < 10; i++) {
				line[0].y = line[1].y = offset.y-30;
				platform_render_draw_lines(line, 2);

				render_text_aligned(game->font, 32, offset.x, offset.y, score_buf, 0);
				offset.y += 40;
			}

			line[0].y = line[1].y = offset.y-30;
			platform_render_draw_lines(line, 2);
			
		} break;
		
		default: {}
	}
}

void draw_game_ui(Game_State* game) {
	SDL_Texture* target = 0;

	if (current_scene != next_scene) {
		Vector2 screen = platform_get_window_size();
		target = platform_create_texture((int)screen.x, (int)screen.y, true);
		
		unsigned char alpha = lerp(0.0f, 255.0f, scene_transition_timer/SCENE_TRANSITION_TIME);
		platform_set_texture_alpha(target, alpha);
		
		platform_set_render_target(target);
		platform_set_render_draw_color((RGBA_Color){0});
		platform_render_clear();
	}

	draw_scene_ui(game, current_scene);

	if (target) {
		platform_set_render_target(0);
		render_draw_texture(target, 0, 0, 0, false);

		unsigned char alpha = lerp(0.0f, 255.0f, 1.0f - scene_transition_timer/SCENE_TRANSITION_TIME);
		platform_set_texture_alpha(target, alpha);

		platform_set_render_target(target);
		
		platform_set_render_draw_color((RGBA_Color){0});
		platform_render_clear();		
		draw_scene_ui(game, next_scene);
		
		platform_set_render_target(0);
		
		render_draw_texture(target, 0, 0, 0, false);
		platform_destroy_texture(target);
	}
}