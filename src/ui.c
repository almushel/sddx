#include "assets.h"
#include "game_math.h"
#include "graphics.h"
#include "platform.h"
#include "ui.h"

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
			if (e->val.i) {
				SDL_itoa(*(e->val.i), buf+len, 10);
			}
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
		(e->children+i)->pos = add_vector2(e->pos, (e->children+i)->pos);
		draw_ui_element(game, e->children+i);
	}
}

void scale_ui_element(ui_element* e, float scale) {
	e->pos = scale_vector2(e->pos, scale);
	switch(e->type) {
		case UI_TYPE_TEXT: {
			e->text.size *= scale;
		} break;

		case UI_TYPE_POLY: {
			e->polygon = scale_poly2d(e->polygon, (Vector2){scale, scale});
		} break;

		case UI_TYPE_RECT: {
			e->rect = scale_rect(e->rect, (Vector2){scale, scale});
		} break;

		case UI_TYPE_TEXTURE: {
			e->texture.dest = scale_rect(e->texture.dest, (Vector2){scale, scale});
		} break;

		default: {} break;
	}

	for (ui_element* child = e->children; child != e->children+e->num_children; child++) {
		scale_ui_element(child, scale);
	}
}
