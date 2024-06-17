#include "SDL_stdinc.h"
#include "platform.h"
#include "defs.h"
#include "game_math.h"
#include "ui.h"

#define MENU_COLOR (RGBA_Color){56, 56, 56, 255}

void thrust_meter_proc(ui_element* e) {
	float thrust_energy = e->val.f ? SDL_clamp(*(e->val.f), 0, PLAYER_THRUST_MAX) : 0;

	e->color = (RGBA_Color){
		(209 - (thrust_energy)),
		(2   * (thrust_energy)),
		(5   + (thrust_energy * 2.5)),
		255
	};

	float thrustDelta = thrust_energy / PLAYER_THRUST_MAX;
	float width1 = e->polygon.vertices[2].x - e->polygon.vertices[1].x;
	float width2 = e->polygon.vertices[3].x - e->polygon.vertices[0].x;
	e->polygon.vertices[2].x = e->polygon.vertices[1].x + (int)(thrustDelta * width1);
	e->polygon.vertices[3].x = e->polygon.vertices[0].x + (int)(thrustDelta * width2);
}

void weapon_heat_outer_proc(ui_element* e) {
	float weapon_heat = e->val.f ? SDL_clamp(*(e->val.f), 0, PLAYER_WEAPON_HEAT_MAX) : 0;
	e->color = 
		(weapon_heat < 100) ?
		(RGBA_Color){17, 17, 17, 255} :
		(RGBA_Color){255, 165, 0, 255};
}

void weapon_heat_inner_proc(ui_element* e) {
	float weapon_heat = e->val.f ? SDL_clamp(*(e->val.f), 0, PLAYER_WEAPON_HEAT_MAX) : 0;

	e->color = (RGBA_Color){
		SDL_clamp(109 +		weapon_heat, 0, 255),
		SDL_clamp(194 - 2 *	weapon_heat, 0, 255),
		SDL_clamp(255 - 2.5 *	weapon_heat, 0, 255),
		255
	};

	float heatDelta = weapon_heat / PLAYER_WEAPON_HEAT_MAX;
	float width1 = e->polygon.vertices[2].x - e->polygon.vertices[1].x;
	float width2 = e->polygon.vertices[3].x - e->polygon.vertices[0].x;
	e->polygon.vertices[2].x = e->polygon.vertices[1].x+(int)(heatDelta * width1);
	e->polygon.vertices[3].x = e->polygon.vertices[0].x+(int)(heatDelta * width2);
}

void weapon_label_proc(ui_element* e) {
	if (e->type != UI_TYPE_TEXT) {
		return;
	}
	switch (*(e->val.i)) {
		case PLAYER_WEAPON_MG: {
			e->text.str = "Machine Gun";
		} break;

		case PLAYER_WEAPON_MISSILE: {
			e->text.str = "Missiles";
		} break;

		case PLAYER_WEAPON_LASER: {
			e->text.str = "Laser";
		} break;

		default: {
			e->text.str = "Unknown";
		} break;
	}
	e->val.i = 0;
}

//TODO: Icon for "Unknown"
void weapon_icon_proc(ui_element* e) {
	if (e->type != UI_TYPE_TEXTURE || e->val.i == 0) {
		return;
	}

	switch (*(e->val.i)) {
		case PLAYER_WEAPON_MG: {
			e->texture.name = "HUD MG";
		} break;

		case PLAYER_WEAPON_MISSILE: {
			e->texture.name = "HUD Missile";
		} break;

		case PLAYER_WEAPON_LASER: {
			e->texture.name = "HUD Laser";
		} break;

		default: {} break;
	}
}

void score_timer_hud_proc(ui_element* e) {
	float timer = e->val.f ? *(e->val.f) : 0;
	int combo_decay_seconds = SCORE_COMBO_DECAY/TICK_RATE;
	float timer_seconds = (timer/(float)TICK_RATE);

	const int padding = (int)(e->rect.w+0.5f)/4;
	Vector2 offset = e->pos;
	for (int t = 0; t < combo_decay_seconds; t++) {
		platform_set_render_draw_color(
			((int)(timer_seconds+0.5f) > t) ?
			SD_BLUE :
			e->color
		);
		platform_render_fill_rect(translate_rect(e->rect, offset));
		offset.x += e->rect.w+padding;
	}
	e->type = 0; // Skip default draw_ui
}

void draw_HUD(Game_State* game, Rectangle bounds, float scale) {
	Poly2D meter_bg = {
		.vertices = {
			{ .x =  -125, .y =   22 },
			{ .x =  -113, .y =  -22 },
			{ .x =   113, .y =  -22 },
			{ .x =   125, .y =   22 }
		},
		.vert_count = 4,
	};
	Poly2D weapon_bg = {
		.vertices = {
			{ .x =  -125, .y =   32 },
			{ .x =  -113, .y =  -32 },
			{ .x =   100, .y =  -32 },
			{ .x =   100, .y =   32 }
		},
		.vert_count = 4,
	};

	ui_element score_hud = {
		.type = UI_TYPE_POLY,
		.pos = {100*scale,-30*scale},
		.polygon = scale_poly2d(weapon_bg, (Vector2){-1, 1}),
		.color = MENU_COLOR,
	};
	score_hud.x += bounds.x;
	score_hud.y += (bounds.y+bounds.h);
	ui_element score_children[] = {
		{
			.type = UI_TYPE_TEXT,
			.text = {
				.str = "Score: ",
				.size = 20
			},
			.val.i = &(game->score.total),
			.pos = {-92, 22},
			.color = WHITE,
		},
		{
			.type = UI_TYPE_TEXT,
			.text = {
				.str = "x",
				.size = 32,
			},
			.val.i = &(game->score.multiplier),
			.pos = {74, -4},
			.color = WHITE,
		},
		{
			.type = UI_TYPE_RECT,
			.pos = {-92, -26},
			.rect = {0,0, 26, 26},
			.color = {125, 125, 125, 255},
			.val.f = &(game->score.timer),
			.draw = score_timer_hud_proc,
		}
	};
	score_hud.children = score_children;
	score_hud.num_children = array_length(score_children);

	ui_element meter_hud = {
		.type = UI_TYPE_POLY,
		.pos = {0, -22*scale},
		.polygon = meter_bg,
		.color = MENU_COLOR,
	};
	meter_hud.x += bounds.x + (bounds.w / 2);
	meter_hud.y += (bounds.y+bounds.h);

	ui_element meter_children[] = {
		{
			.type = UI_TYPE_POLY,
			.pos = {1,-2},
			.color = SD_BLUE,
			.polygon = {
				.vertices = {
					{ .x =  0 , .y = -20 },
					{ .x =  13, .y =  20 },
					{ .x = -13, .y =  20 },
				},
				.vert_count = 3,
			},
		},
		{
			.type = UI_TYPE_TEXTURE,
			.pos = {1, 7},
			.angle = -90,
			.texture = {
				.name = "Player Ship",
				.dest = {0,0,27,30},
			},
		},
		{
			.type = UI_TYPE_TEXT,
			.val.i = &(game->player_state.lives),
			.text = {.size = 34, .align = "center" },
			.pos = {1, 16},
			.color = BLACK,
		},
		{
			.type = UI_TYPE_TEXT,
			.val.i = &(game->player_state.lives),
			.text = {.size = 30, .align = "center" },
			.pos = {1, 15},
			.color = WHITE,
		},
		{
			.type = UI_TYPE_TEXT,
			.pos = {-64,-8},
			.text = {
				.str = "Thrust Power",
				.size = 14,
				.align = "center",
			},
			.color = WHITE,
		},
		{
			.type = UI_TYPE_TEXT,
			.pos = {64, -8},
			.text = {
				.str = "Weapon Temp",
				.size = 14,
				.align = "center",
			},
			.color = WHITE,
		},
		{
			.type = UI_TYPE_POLY,
			.pos = {-60, 6},
			.color = {17, 17, 17, 255},
			.polygon = {
				.vertices = {
					{ .x =  -50, .y =   10 },
					{ .x =  -44, .y =  -10 },
					{ .x =   52, .y =  -10 },
					{ .x =   46, .y =   10 }
				},
				.vert_count = 4,
			},
		},
		{
			.type = UI_TYPE_POLY,
			.pos = {-60, 6},
			.color = SD_BLUE,
			.polygon = {
				.vertices = {
					{ .x =  -47, .y =   8 },
					{ .x =  -42, .y =  -8 },
					{ .x =   49, .y =  -8 },
					{ .x =   44, .y =   8 }
				},
				.vert_count = 4,
			},
			.val.f = &(game->player_state.thrust_energy),
			.draw = thrust_meter_proc,
		},

		{
			.type = UI_TYPE_POLY,
			.pos = {60, 6},
			.val.f = &(game->player_state.weapon_heat),
			.polygon = {
				.vertices = {
					{ .x =  -44, .y =   10 },
					{ .x =  -50, .y =  -10 },
					{ .x =   46, .y =  -10 },
					{ .x =   52, .y =   10 }
				},
				.vert_count = 4,
			},
			.color = {128, 128, 128, 255},
			.draw = weapon_heat_outer_proc,
		},
		{
			.type = UI_TYPE_POLY,
			.pos = {60, 6},
			.val.f = &(game->player_state.weapon_heat),
			.polygon = {
				.vertices = {
					{ .x =  -42, .y =   8 },
					{ .x =  -47, .y =  -8 },
					{ .x =   42, .y =  -8 },
					{ .x =   47, .y =   8 }
				},
				.vert_count = 4,
			},
			.color = {255, 0, 0, 255},
			.draw = weapon_heat_inner_proc,
		}


	};
	meter_hud.children = meter_children;
	meter_hud.num_children = array_length(meter_children);

	ui_element weapon_hud = {
		.type = UI_TYPE_POLY,
		.pos = {-100*scale, -30*scale},
		.polygon = weapon_bg,
		.color = MENU_COLOR,
	};
	weapon_hud.x += (bounds.x+bounds.w);
	weapon_hud.y += (bounds.y+bounds.h);
	int current_weapon = (game->player) ? (int)(game->player->type_data) : 0;
	ui_element weapon_children[] = {
		{
			.type = UI_TYPE_TEXT,
			.text = {
				.str = "Ammo",
				.size = 16,
				.align = "center",
			},
			.pos = (Vector2){-60, -18},
			.color = WHITE,
		},
		{
			.type = UI_TYPE_TEXT,
			.text ={
				.size = 36,
				.align = "center",
			},
			.val = &(game->player_state.ammo),
			.pos = {-60, 18},
			.color = SD_BLUE,
		},
		{
			.type = UI_TYPE_TEXT,
			.text = {
				.size = 16,
				.align = "center",
			},
			.pos = {40, -18},
			.color = WHITE,
			.val = &current_weapon, 
			.draw = weapon_label_proc
		},
		{
			.type = UI_TYPE_TEXTURE,
			.pos = {40, 6},
			.val = &current_weapon,
			.draw = weapon_icon_proc,
		}
	};
	weapon_hud.children = weapon_children;
	weapon_hud.num_children = array_length(weapon_children);

	ui_element hud[] = {
		score_hud,
		meter_hud,
		weapon_hud
	};

	for (int i = 0; i < array_length(hud); i++) {
		hud[i].polygon = scale_poly2d(hud[i].polygon, (Vector2){scale, scale});
		for (ui_element* child = hud[i].children; child != hud[i].children+hud[i].num_children; child++) {
			scale_ui_element(child, scale);
		}
		draw_ui_element(game, hud+i);
	}

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
