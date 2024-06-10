#include "platform.h"
#include "defs.h"
#include "assets.h"
#include "graphics.h"
#include "game_math.h"
#include <stddef.h>

#define MENU_COLOR (RGBA_Color){56, 56, 56, 255}

float hudDir = 0, hudAccumulator = 0;

typedef struct hud_text {
		char* label;
		int font_size;
		int val;
		iVector2 pos;
		RGBA_Color color;
} hud_text;

void draw_score(Game_State* game) {
	char buf[64];
	iVector2 origin = {8, platform_get_window_size().y - 8};

	hud_text elements[] = {
		{
			.label = "Score: ",
			.font_size = 20,
			.val = game->score.total,
			.pos = {0},
			.color = WHITE,
		},
		{
			.label = "x",
			.font_size = 20,
			.val = game->score.multiplier,
			.pos = {178, -28},
			.color = WHITE,
		}
	};

	for (hud_text* e = elements; e != elements+array_length(elements); e++) {
		platform_set_render_draw_color(e->color);
		size_t len = SDL_strlcpy(buf, e->label, array_length(buf));
		SDL_itoa(e->val, buf+len, 10);
		render_text(game->font, e->font_size, origin.x+e->pos.x, origin.y+e->pos.y, buf);
	}

	int combo_decay_seconds = SCORE_COMBO_DECAY/TICK_RATE;
	float timer_seconds = (game->score.timer/(float)TICK_RATE);

	RGBA_Color rect_color = {125, 125, 125, 255};
	Rectangle rect = {0,0, 26, 26};
	Vector2 offset = {origin.x, origin.y-48};

	for (int t = 0; t < combo_decay_seconds; t++) {
		platform_set_render_draw_color(
			((int)(timer_seconds+0.5f) > t) ?
			SD_BLUE :
			rect_color
		);
		platform_render_fill_rect(translate_rect(rect, offset));
		offset.x += 32;
	}
}

void draw_player_lives(Game_State* game) {
	Vector2 origin;
	{
		iVector2 screen = platform_get_window_size();
		origin.x = (float)screen.x/2.0f;
		origin.y = screen.y;
	}
	Vector2 lives_pos = {1,-20};
	Poly2D lives_bg = {
		.vertices = {
			{ .x =  0 , .y = -20 },
			{ .x =  13, .y =  20 },
			{ .x = -13, .y =  20 },
		},
		.vert_count = 3,
	};

	render_fill_polygon(
		translate_poly2d(lives_bg, add_vector2(lives_pos, origin)).vertices,
		lives_bg.vert_count, SD_BLUE
	);

	Game_Sprite ship_sprite = {
		.texture_name = "Player Ship",
		.rotation_enabled = 1,
	};
	Vector2 sprite_dims = {27, 20};
	iVector2 texture_dims;
	{
		SDL_Texture* ship_texture = game_get_texture(game, "Player Ship");
		SDL_QueryTexture(ship_texture, 0, 0, &texture_dims.x, &texture_dims.y);
	}
	Transform2D transform = {
		.position = add_vector2(origin, (Vector2){1,-14}),
		.scale.x = sprite_dims.x/texture_dims.x,
		.scale.y = sprite_dims.y/texture_dims.y,
		.angle = -90.0f,
	};

	render_draw_game_sprite(game, &ship_sprite, transform, 1);

	hud_text labels[] = {
		{
			.val = game->player_state.lives,
			.font_size = 34,
			.pos = {1, -6},
			.color = BLACK,
		},
		{
			.val = game->player_state.lives,
			.font_size = 30,
			.pos = {1, -7},
			.color = WHITE,
		},
	};
	
	char buf[8];
	for (hud_text* e = labels; e != labels + array_length(labels); e++) {
		SDL_itoa(e->val, buf, 10);
		platform_set_render_draw_color(e->color);
		render_text_aligned(game->font, e->font_size, origin.x+e->pos.x, origin.y+e->pos.y, buf, "center");
	}
}

void draw_thrust_meter(Game_State* game) {
	Vector2 origin;
	{
		iVector2 screen = platform_get_window_size();
		origin.x = (float)screen.x / 2 - 60;
		origin.y = (float)screen.y - 16;
	}

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
		(209 - thrust_energy),
		(2   * thrust_energy),
		(5   + 2.5 * thrust_energy),
		255
	};

	render_fill_polygon(translate_poly2d(meterOuterPoly, origin).vertices, meterOuterPoly.vert_count, tmColorOuter);

	float thrustDelta = thrust_energy / PLAYER_THRUST_MAX;
	meterInnerPoly.vertices[2].x = -41 + (int)(thrustDelta * 90);
	meterInnerPoly.vertices[3].x = -41 + (int)(thrustDelta * 90) - 5;
	
	render_fill_polygon(translate_poly2d(meterInnerPoly, origin).vertices, meterInnerPoly.vert_count, tmColorInner);

	float text_size = 14;
	Vector2 offset = {-4,-14};
	render_text_aligned(game->font, text_size, origin.x+offset.x, origin.y+offset.y, "Thrust Power", "center");
}

void draw_weapon_heat(Game_State* game) {
	Vector2 origin;
	{
		iVector2 screen = platform_get_window_size();
		origin.x = ((float)screen.x / 2) + 60;
		origin.y = screen.y - 16;
	}

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
	
	if (weapon_heat < 100)	hmColorOuter = (RGBA_Color){17, 17, 17, 255};
	else			hmColorOuter = (RGBA_Color){255, 165, 0, 255};

	hmColorInner = (RGBA_Color){
		SDL_clamp(109 +		weapon_heat, 0, 255),
		SDL_clamp(194 - 2 *	weapon_heat, 0, 255),
		SDL_clamp(255 - 2.5 *	weapon_heat, 0, 255),
		255
	};

	render_fill_polygon(translate_poly2d(heatOuterPoly, origin).vertices, heatOuterPoly.vert_count, hmColorOuter);

	float heatDelta = weapon_heat / PLAYER_WEAPON_HEAT_MAX;
	heatInnerPoly.vertices[2].x = heatInnerPoly.vertices[1].x+(int)(heatDelta * 90);
	heatInnerPoly.vertices[3].x = heatInnerPoly.vertices[0].x+(int)(heatDelta * 90);
	
	render_fill_polygon(translate_poly2d(heatInnerPoly, origin).vertices, heatInnerPoly.vert_count, hmColorInner);

	float text_size = 14;
	Vector2 offset = {-4, -14};
	render_text_aligned(game->font, text_size, origin.x+offset.x, origin.y+offset.y, "Weapon Temp", "center");
}

void draw_active_weapon(Game_State* game) {
	if (game->player == 0) {
		return;
	}
	Vector2 origin;
	{
		iVector2 screen = platform_get_window_size();
		origin.x = screen.x - 160;
		origin.y = screen.y;
	}

	//TODO: Default label and icon for "Unknown" (player is dead)
	char* weapon_label;
	SDL_Texture* hud_weapon;
	switch (game->player->type_data) {
		case PLAYER_WEAPON_MISSILE: {
			weapon_label = "Missile";
			hud_weapon = game_get_texture(game, "HUD Missile");
		} break;

		case PLAYER_WEAPON_LASER: {
			weapon_label = "Laser";
			hud_weapon = game_get_texture(game, "HUD Laser");
		} break;

		default: {
			weapon_label = "Machine Gun";
			hud_weapon = game_get_texture(game, "HUD MG");;
		} break;
	}

	int hw_width, hw_height;
	SDL_QueryTexture(hud_weapon, 0, 0, &hw_width, &hw_height);
	
	char ammo_str[8];
	SDL_itoa(game->player_state.ammo, ammo_str, 10);

	hud_text labels[] = {
		{
			.label = "Ammo",
			.font_size = 16,
			.pos = {0,-hw_height},
			.color = WHITE,
		},
		{
			.label = ammo_str,
			.font_size = 36,
			.pos = {0, -16},
			.color = SD_BLUE,
		},
		{
			.label = weapon_label,
			.font_size = 16,
			.pos = {hw_width, -hw_height},
			.color = WHITE,
		}

	};

	for (hud_text* e = labels; e != labels+array_length(labels); e++) {
		platform_set_render_draw_color(e->color);
		render_text_aligned(game->font, e->font_size, origin.x+e->pos.x, origin.y+e->pos.y, e->label, "center");
		
	}
	render_draw_texture(hud_weapon, origin.x+(float)hw_width/2.0f, origin.y-hw_height, 0, 0);
}

void draw_HUD(Game_State* game) {
	iVector2 screen = platform_get_window_size();

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

	struct hud_poly {
		Vector2 pos;
		Poly2D poly;
		RGBA_Color color;
	};

	render_fill_polygon(
		translate_poly2d(weapon_and_score_bg, (Vector2){100, screen.y - 30}).vertices,
		weapon_and_score_bg.vert_count, MENU_COLOR
	);
	render_fill_polygon(
		translate_poly2d(meterBG, (Vector2){(float)screen.x / 2, screen.y - 22}).vertices,
		meterBG.vert_count, MENU_COLOR
	);
	render_fill_polygon(
		translate_poly2d(weapon_and_score_bg, (Vector2){screen.x - 100, screen.y - 30}).vertices,
		weapon_and_score_bg.vert_count, MENU_COLOR
	);

	draw_player_lives(game);
	draw_thrust_meter(game);
	draw_weapon_heat(game);
	draw_score(game);
	draw_active_weapon(game);

#ifdef DEBUG
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
		SDL_itoa(values[i], buffer + SDL_strlen(labels[i]), 10);
		render_text(game->font, font_size, 8.0f, 48.0f + (font_size * 1.25 * i), buffer);
	}
}
#endif
	}
