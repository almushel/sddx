#include "platform.h"
#include "assets.h"
#include "graphics.h"
#include "game_math.h"

#define HUD_TOP 536
#define HUD_SPEED 4
#define MENU_COLOR (RGBA_Color){56, 56, 56, 255}

float hudDir = 0, hudAccumulator = 0;

void draw_score(Game_State* game) {
	char text_buffer[32] = ""; // NOTE: Hopefully able to contain highest practical score value?
	Vector2 screen_dim = platform_get_window_size();
	int screen_h = screen_dim.y;
	int screen_w = screen_dim.x;

	float text_size = 20;
	Vector2 text_pos = {8, screen_h - 8};
	render_text(game->font, 20, text_pos.x, text_pos.y, "Score:");
	
	text_pos.x += measure_text(game->font, text_size, "Score:");
	text_pos.y = screen_h - 7.5;
	platform_set_render_draw_color((RGBA_Color){255, 255, 255, 255});
	itoa(game->score.total, text_buffer, 10);
	render_text(game->font, text_size, text_pos.x, text_pos.y, text_buffer);

	text_pos = (Vector2){186, screen_h - 36};
	text_buffer[0] = 'x';
	itoa(game->score.multiplier, text_buffer+1, 10);
	render_text(game->font, text_size, text_pos.x, text_pos.y, text_buffer);

	int combo_decay_seconds = SCORE_COMBO_DECAY/TICK_RATE;
	float combo_seconds_remaining = (game->score.timer/(float)TICK_RATE);

	for (int t = 0; t < combo_decay_seconds; t++) {
		RGBA_Color rect_color = {125, 125, 125, 255};
		if ((int)(combo_seconds_remaining+0.5f) > t) rect_color = SD_BLUE;

		Rectangle rect = {8 + 32 * t, screen_h - 56, 26, 26};
		
		platform_set_render_draw_color(rect_color);
		platform_render_fill_rect(rect);
	}
}

void draw_player_lives(Game_State* game) {
		Vector2 screen_dim = platform_get_window_size();
		int screen_h = screen_dim.y;
		int screen_w = screen_dim.x;
	
		Poly2D lives_bg = {
			.vertices = {
				{ .x =  0 , .y = -20 },
				{ .x =  13, .y =  20 },
				{ .x = -13, .y =  20 },
			},
			.vert_count = 3,
		};
		render_fill_polygon(translate_poly2d(lives_bg, (Vector2){screen_w / 2 + 1, screen_h - 20}).vertices, lives_bg.vert_count, SD_BLUE);

		int player_width  = 0;
		int player_height = 0;		
		SDL_Texture* player_ship_texture = game_get_texture(game, "Player Ship");
		SDL_QueryTexture(player_ship_texture, 0, 0, &player_width, &player_height);
		float wScale = 1.0f / 1.7f;
		float hScale = wScale * ((float)player_height / (float)player_width);
		
		Game_Sprite player_lives_ship = {
			.texture_name = "Player Ship",
			.rotation_enabled = 1,
		};

		Transform2D transform = {
			.position = {screen_w / 2 + 1, screen_h - (float)player_height/1.5f * hScale},
			.scale.x = wScale,
			.scale.y = hScale,
			.angle = -90.0f,
		};

		render_draw_game_sprite(game, &player_lives_ship, transform, 1);
		
		char lives_str[8];
		itoa(game->player_state.lives, lives_str, 10);
		float lives_text_size = 32;
		Vector2 lives_text_pos = {2.0f + (float)screen_w / 2.0f, screen_h - 6};
		
		// Lives text shadow???
		platform_set_render_draw_color((RGBA_Color){0,0,0,255});
		lives_text_pos.x -= measure_text(game->font, lives_text_size+4, lives_str)/2.0f;
		render_text(game->font, lives_text_size+4, lives_text_pos.x, lives_text_pos.y+1, lives_str); // Player Lives
		
		// Lives text
		platform_set_render_draw_color((RGBA_Color){255,255,255,255});
		lives_text_pos.x = 2.0f + (float)screen_w / 2.0f - measure_text(game->font, lives_text_size, lives_str)/2.0f;
		render_text(game->font, lives_text_size, lives_text_pos.x, lives_text_pos.y, lives_str); // Player Lives
}

void draw_thrust_meter(Game_State* game) {
	Vector2 screen_dim = platform_get_window_size();
	int screen_h = screen_dim.y;
	int screen_w = screen_dim.x;

	Poly2D meterOuterPoly = {
		.vertices = {
			{ .x =  -50, .y =   10 },
			{ .x =  -44, .y =  -10 },
			{ .x =   52, .y =  -10 },
			{ .x =   46, .y =   10 }
		},
		.vert_count = 4,
	};

	Poly2D meterInnerPoly = {
		.vertices = {
			{ .x =  -47, .y =   8 },
			{ .x =  -42, .y =  -8 },
			{ .x =   49, .y =  -8 },
			{ .x =   44, .y =   8 }
		},
		.vert_count = 4,
	};

	float thrust_energy = SDL_clamp(game->player_state.thrust_energy, 0, PLAYER_THRUST_MAX);

	RGBA_Color tmColorOuter = {17, 17, 17, 255};
	RGBA_Color tmColorInner = {
		(209 - 		 thrust_energy),
		(2   * 		 thrust_energy),
		(5   + 2.5 * thrust_energy),
		255
	};

	render_fill_polygon(translate_poly2d(meterOuterPoly, (Vector2){screen_w / 2 - 60, screen_h - 16}).vertices, meterOuterPoly.vert_count, tmColorOuter);

	float thrustDelta = thrust_energy / PLAYER_THRUST_MAX;
	meterInnerPoly.vertices[2].x = -41 + (int)(thrustDelta * 90);
	meterInnerPoly.vertices[3].x = -41 + (int)(thrustDelta * 90) - 5;
	
	render_fill_polygon(translate_poly2d(meterInnerPoly, (Vector2){screen_w / 2 - 60, screen_h - 16}).vertices, meterInnerPoly.vert_count, tmColorInner);

	float text_size = 14;
	Vector2 text_pos = {screen_w / 2 - 56, screen_h - 30};
	text_pos.x -= measure_text(game->font, text_size, "Thrust Power")/2.0f;
	render_text(game->font, text_size, text_pos.x, text_pos.y, "Thrust Power");
}

void draw_weapon_heat(Game_State* game) {
	Vector2 screen_dim = platform_get_window_size();
	int screen_h = screen_dim.y;
	int screen_w = screen_dim.x;

	Poly2D heatOuterPoly = {
		.vertices = {
			{ .x =  -44, .y =   10 },
			{ .x =  -50, .y =  -10 },
			{ .x =   46, .y =  -10 },
			{ .x =   52, .y =   10 }
		},
		.vert_count = 4,
	};

	Poly2D heatInnerPoly = {
		.vertices = {
			{ .x =  -42, .y =   8 },
			{ .x =  -47, .y =  -8 },
			{ .x =   42, .y =  -8 },
			{ .x =   47, .y =   8 }
		},
		.vert_count = 4,
	};

	RGBA_Color hmColorOuter = {128, 128, 128, 255};
	RGBA_Color hmColorInner = {255, 0, 0, 255};

	float weapon_heat = 0;
	if (game->player) weapon_heat = SDL_clamp(game->player_state.weapon_heat, 0, PLAYER_WEAPON_HEAT_MAX);
	
	if (weapon_heat < 100) hmColorOuter = (RGBA_Color){17, 17, 17, 255};
	else hmColorOuter = (RGBA_Color){255, 165, 0, 255};

	hmColorInner = (RGBA_Color){
		SDL_clamp(109 + 	  weapon_heat, 0, 255),
		SDL_clamp(194 - 2 *   weapon_heat, 0, 255),
		SDL_clamp(255 - 2.5 * weapon_heat, 0, 255),
		255
	};

	render_fill_polygon(translate_poly2d(heatOuterPoly, (Vector2){screen_w / 2 + 60, screen_h - 16}).vertices, heatOuterPoly.vert_count, hmColorOuter);

	float heatDelta = weapon_heat / PLAYER_WEAPON_HEAT_MAX;
	heatInnerPoly.vertices[2].x = -41 + (int)(heatDelta * 90) - 5;
	heatInnerPoly.vertices[3].x = -41 + (int)(heatDelta * 90);
	
	render_fill_polygon(translate_poly2d(heatInnerPoly, (Vector2){screen_w / 2 + 60, screen_h - 16}).vertices, heatInnerPoly.vert_count, hmColorInner);

	float text_size = 14;
	Vector2 text_pos;
	text_pos.x = screen_w / 2 + 56 - measure_text(game->font, text_size, "Weapon Temp")/2.0f;
	text_pos.y = screen_h - 30;
	render_text(game->font, text_size, text_pos.x, text_pos.y, "Weapon Temp");
}

void draw_active_weapon(Game_State* game) {
	Vector2 screen_dim = platform_get_window_size();
	int screen_h = screen_dim.y;
	int screen_w = screen_dim.x;

	char* weapon_label = "Machine Gun";
	SDL_Texture* hud_weapon = game_get_texture(game, "HUD MG");;
	if (game->player) {
		switch (game->player->type_data) {
			case PLAYER_WEAPON_MISSILE: {
				weapon_label = "Missile";
				hud_weapon = game_get_texture(game, "HUD Missile");
			} break;

			case PLAYER_WEAPON_LASER: {
				weapon_label = "Laser";
				hud_weapon = game_get_texture(game, "HUD Laser");
			} break;
		}
	}

	//wAmmo = p1.activeWeapon != "Machine Gun" ? p1.ammo : INFINITY_SYMBOL;
	int hw_width, hw_height;
	SDL_QueryTexture(hud_weapon, 0, 0, &hw_width, &hw_height);
	
	// Ammo label
	float text_size = 14;
	Vector2 text_pos = {screen_w - 160, screen_h - hw_height};
	platform_set_render_draw_color((RGBA_Color){255, 255, 255, 255});
	text_pos.x -= measure_text(game->font, text_size, "Ammo")/2.0f;
	render_text(game->font, text_size, text_pos.x, text_pos.y, "Ammo");
	
	// Ammo count
	char ammo_str[8];
	itoa(game->player_state.ammo, ammo_str, 10);
	text_pos.x = (screen_w - 160) - measure_text(game->font, 34, ammo_str)/2.0f;
	text_pos.y = screen_h - 16;
	render_text(game->font, 34, text_pos.x, text_pos.y, ammo_str);

	// Active weapon label
	if (weapon_label) {
		text_pos.x = (screen_w - hw_width / 2) - measure_text(game->font, text_size, weapon_label)/2.0f;
		text_pos.y = screen_h - hw_height;
		render_text(game->font, text_size, text_pos.x, text_pos.y, weapon_label); // Active weapon name
	}
		
	// Active weapon icon
	render_draw_texture(hud_weapon, screen_w - hw_width, screen_h - hw_height, 0, 0);
}

void draw_HUD(Game_State* game) {
	Vector2 screen_dim = platform_get_window_size();
	int screen_h = screen_dim.y;
	int screen_w = screen_dim.x;
//	updateHUDTransition();

	if (true) {//gameState == gameStarted) {

		Poly2D meterBG = {
			.vertices = {
				{ .x =  -125, .y =   22 },
				{ .x =  -113, .y =  -22 },
				{ .x =   113, .y =  -22 },
				{ .x =   125, .y =   22 }
			},
			.vert_count = 4,
		};

		Poly2D weapon_and_score_bg = {
			.vertices = {
				{ .x =  -125, .y =   32 },
				{ .x =  -113, .y =  -32 },
				{ .x =   113, .y =  -32 },
				{ .x =   125, .y =   32 }
			},
			.vert_count = 4,
		};

//		alpha = 0.6;
		render_fill_polygon(translate_poly2d(weapon_and_score_bg, (Vector2){100, screen_h - 30}).vertices, weapon_and_score_bg.vert_count, MENU_COLOR);
		render_fill_polygon(translate_poly2d(meterBG, (Vector2){screen_w / 2, screen_h - 22}).vertices, meterBG.vert_count, MENU_COLOR);
		render_fill_polygon(translate_poly2d(weapon_and_score_bg, (Vector2){screen_w - 100, screen_h - 30}).vertices, weapon_and_score_bg.vert_count, MENU_COLOR);
		
		platform_set_render_draw_color((RGBA_Color){255, 255, 255, 255});

		draw_player_lives(game);
		draw_thrust_meter(game);
		draw_weapon_heat(game);
		draw_score(game);
		draw_active_weapon(game);

#if DEBUG
{
		char buffer[128];;
		char* labels[] = {
			"Current Wave: ",
			"Spawn Points Max: ",
			"Spawn Type Max: "
		};
		
		int values[array_length(labels)] = {
			game->score.current_wave,
			game->score.spawn_points_max,
			SDL_clamp((game->score.current_wave / WAVE_ESCALATION_RATE), 0, ENTITY_TYPE_ENEMY_GRAPPLER - ENTITY_TYPE_ENEMY_DRIFTER),
		};

		float font_size = 16.0f;

		platform_set_render_draw_color(WHITE);
		for (int i = 0; i < array_length(labels); i++) {
			SDL_strlcpy(buffer, labels[i], 128);
			itoa(values[i], buffer + SDL_strlen(labels[i]), 10);
			render_text(game->font, font_size, 8.0f, 48.0f + (font_size * 1.25 * i), buffer);
		}
}
#endif
	}
}