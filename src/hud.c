#include "SDL_render.h"
#include "SDL_stdinc.h"
#include "platform.h"
#include "defs.h"
#include "assets.h"
#include "graphics.h"
#include "game_math.h"
#include <stddef.h>

#define MENU_COLOR (RGBA_Color){56, 56, 56, 255}

typedef struct ui_element {
	Vec2_Union(pos, x, y);
	float angle;
	RGBA_Color color;
	int val;

	enum UI_Types {
		UI_TYPE_UNDEFINED = 0,
		UI_TYPE_TEXT,
		UI_TYPE_RECT,
		UI_TYPE_POLY,
		UI_TYPE_TEXTURE
	} type;

	union {
		Poly2D polygon;
		Rectangle rect;
		struct {Rectangle dest; char* name; } texture;
		struct {int size; char* str; char* align; } text;
	};

	void (*draw)(struct ui_element* element);
	
	int num_children;
	struct ui_element* children;
} ui_element;

void thrust_meter_proc(ui_element* e) {
	float thrust_energy = SDL_clamp(e->val, 0, PLAYER_THRUST_MAX);

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
	float weapon_heat = SDL_clamp(e->val, 0, PLAYER_WEAPON_HEAT_MAX);
	e->color = 
		(weapon_heat < 100) ?
		(RGBA_Color){17, 17, 17, 255} :
		(RGBA_Color){255, 165, 0, 255};
}

void weapon_heat_inner_proc(ui_element* e) {
	float weapon_heat = weapon_heat = SDL_clamp(e->val, 0, PLAYER_WEAPON_HEAT_MAX);

	e->color = (RGBA_Color){
		SDL_clamp(109 +		weapon_heat, 0, 255),
		SDL_clamp(194 - 2 *	weapon_heat, 0, 255),
		SDL_clamp(255 - 2.5 *	weapon_heat, 0, 255),
		255
	};

	float heatDelta = weapon_heat / PLAYER_WEAPON_HEAT_MAX;
	e->polygon.vertices[2].x = e->polygon.vertices[1].x+(int)(heatDelta * 90);
	e->polygon.vertices[3].x = e->polygon.vertices[0].x+(int)(heatDelta * 90);
}


void draw_ui_element(Game_State* game, ui_element* e) {
	if (e->draw != 0) {
		e->draw(e);
	}

	switch(e->type) {
		case UI_TYPE_TEXTURE: {
			SDL_Texture* texture = game_get_texture(game, e->texture.name);
			if (texture) {
				Rectangle dest = e->texture.dest;
				if (dest.w == 0 && dest.h == 0) {
					int w, h;
					SDL_QueryTexture(texture, NULL, NULL, &w, &h);
					dest.w = w;
					dest.h = h;
				}
				Vector2 pos = {e->pos.x-dest.w/2.0f, e->pos.y-dest.h/2.0f};
				dest = translate_rect(dest, pos);

				platform_render_copy(texture, 0, &dest, e->angle, 0, 0);
			}
		} break;

		case UI_TYPE_TEXT: {
			char buf[64];
			size_t len = 0;
			if (e->text.str != 0 && e->text.str[0] != '\0') {
				len = SDL_strlcpy(buf, e->text.str, array_length(buf));
			}
			SDL_itoa(e->val, buf+len, 10);
			platform_set_render_draw_color(e->color);
			render_text_aligned(game->font, e->text.size, e->pos.x, e->pos.y, buf, e->text.align);
		} break;

		case UI_TYPE_POLY: {
			Poly2D p = translate_poly2d(e->polygon, e->pos);
			render_fill_polygon(p.vertices, p.vert_count, e->color);
		} break;

		case UI_TYPE_RECT: {
			Rectangle rect = translate_rect(e->rect, e->pos);
			platform_set_render_draw_color(e->color);
			platform_render_draw_rect(rect);
		} break;

		default: break;
	}

	for (int i = 0; i < e->num_children; i++) {
		draw_ui_element(game, e->children+i);
	}
}

void weapon_label_proc(ui_element* e) {
	if (e->type != UI_TYPE_TEXT) {
		return;
	}
	switch (e->val) {
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
}

//TODO: Icon for "Unknown"
void weapon_icon_proc(ui_element* e) {
	if (e->type != UI_TYPE_TEXTURE || e->val == 0) {
		return;
	}

	switch (e->val) {
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
	int combo_decay_seconds = SCORE_COMBO_DECAY/TICK_RATE;
	float timer_seconds = (e->val/(float)TICK_RATE);

	const int padding = 32;
	Vector2 offset = e->pos;
	for (int t = 0; t < combo_decay_seconds; t++) {
		platform_set_render_draw_color(
			((int)(timer_seconds+0.5f) > t) ?
			SD_BLUE :
			e->color
		);
		platform_render_fill_rect(translate_rect(e->rect, offset));
		offset.x += padding;
	}
	e->type = 0; // Skip default draw_ui
}

void draw_HUD(Game_State* game) {
	iVector2 screen = platform_get_window_size();
	Poly2D meter_bg = {
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

	ui_element score_hud = {
		.type = UI_TYPE_POLY,
		.pos = {100, screen.y-30},
		.polygon = weapon_and_score_bg,
		.color = MENU_COLOR,
	};
	ui_element score_children[] = {
		{
			.type = UI_TYPE_TEXT,
			.text = {
				.str = "Score: ",
				.size = 20
			},
			.val = game->score.total,
			.pos = {8, screen.y-8},
			.color = WHITE,
		},
		{
			.type = UI_TYPE_TEXT,
			.text = {
				.str = "x",
				.size = 20,
			},
			.val = game->score.multiplier,
			.pos = {186, screen.y-36},
			.color = WHITE,
		},
		{
			.type = UI_TYPE_RECT,		
			.pos = {8, screen.y-56},
			.rect = {0,0, 26, 26},
			.color = {125, 125, 125, 255},
			.val = game->score.timer,
			.draw = score_timer_hud_proc,
		}
	};
	score_hud.children = score_children;
	score_hud.num_children = array_length(score_children);

	ui_element meter_hud = {
		.type = UI_TYPE_POLY,
		.pos = {(float)screen.x / 2, screen.y - 22},
		.polygon = meter_bg,
		.color = MENU_COLOR,
	};
	ui_element meter_children[] = {
		{
			.type = UI_TYPE_POLY,
			.pos = {(float)screen.x/2.0f+1.0f, screen.y-20},
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
			.pos = {(float)screen.x/2.0f+1, screen.y-15},
			.angle = -90,
			.texture = {
				.name = "Player Ship",
				.dest = {0,0,27,30},
			},
		},
		{
			.type = UI_TYPE_TEXT,
			.val = game->player_state.lives,
			.text = {.size = 34, .align = "center" },
			.pos = {(float)screen.x/2.0f+1.0f, screen.y-6},
			.color = BLACK,
		},
		{
			.type = UI_TYPE_TEXT,
			.val = game->player_state.lives,
			.text = {.size = 30, .align = "center" },
			.pos = {(float)screen.x/2.0f+1.0f, screen.y-7},
			.color = WHITE,
		},
		{
			.type = UI_TYPE_TEXT,
			.pos = {(float)screen.x/2-64,screen.y-30},
			.text = {
				.str = "Thrust Power",
				.size = 14,
				.align = "center",
			},
			.color = WHITE,
		},
		{
			.type = UI_TYPE_TEXT,
			.pos = {screen.x/2.0f+64, screen.y-30},
			.text = {
				.str = "Weapon Temp",
				.size = 14,
				.align = "center",
			},
			.color = WHITE,
		},
		{
			.type = UI_TYPE_POLY,
			.pos = {(float)screen.x/2.0f - 60, screen.y-16},
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
			.pos = {(float)screen.x/2.0f - 60, screen.y-16},
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
			.val = game->player_state.thrust_energy,
			.draw = thrust_meter_proc,
		},

		{
			.type = UI_TYPE_POLY,
			.pos = {((float)screen.x / 2) + 60, screen.y-16},
			.val = game->player_state.weapon_heat,
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
			.pos = {((float)screen.x / 2) + 60, screen.y-16},
			.val = game->player_state.weapon_heat,
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
		.pos = {screen.x-100, screen.y-30},
		.polygon = weapon_and_score_bg,
		.color = MENU_COLOR,
	};
	ui_element weapon_children[] = {
		{
			.type = UI_TYPE_TEXT,
			.text = {
				.str = "Ammo",
				.size = 16,
				.align = "center",
			},
			.pos = {screen.x-160, screen.y - 48},
			.color = WHITE,
		},
		{
			.type = UI_TYPE_TEXT,
			.text ={
				.size = 36,
				.align = "center",
			},
			.val = game->player_state.ammo,
			.pos = {screen.x-160, screen.y-16},
			.color = SD_BLUE,
		},
		{
			.type = UI_TYPE_TEXT,
			.text = {
				.size = 16,
				.align = "center",
			},
			.pos = {screen.x-60, screen.y-48},
			.color = WHITE,
			.val = game->player ? game->player->type_data : 0,
			.draw = weapon_label_proc
		},
		{
			.type = UI_TYPE_TEXTURE,
			.pos = {screen.x-60, screen.y-24},
			.val = game->player ? game->player->type_data : 0,
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
