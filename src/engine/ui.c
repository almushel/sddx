#include "SDL_render.h"
#include "assets.h"
#include "math.h"
#include "graphics.h"
#include "platform.h"
#include "types.h"
#include "ui.h"

ui_element new_ui_text(Vector2 pos, int* val, RGBA_Color color, char* str, int size, const char* align) {
	ui_element result = {
		.type = UI_TYPE_TEXT,
		.pos = pos,
		.color = color,
		.val.i = val,
		.text = {
			.str = str,
			.size = size,
			.align = (char*)align,
		}
	};

	return result;
}

inline ui_element new_ui_text_proc(Vector2 pos, int* val, RGBA_Color color, ui_proc proc, char* str, int size, const char* align) {
	ui_element result = new_ui_text(pos, val, color, str, size, align);
	result.draw = proc;

	return result;
}

ui_element new_ui_rect(Vector2 pos, float* val, RGBA_Color color, Rectangle rect) {
	ui_element result = {
		.type = UI_TYPE_RECT,
		.pos = pos,
		.rect = rect,
		.color = color,
		.val.f = val,
	};

	return result;
}

ui_element new_ui_rect_proc(Vector2 pos, float* val, ui_proc proc, RGBA_Color color, Rectangle rect) {
	ui_element result = new_ui_rect(pos, val, color, rect);
	result.draw = proc;

	return result;
}

ui_element new_ui_poly(Vector2 pos, RGBA_Color color, Poly2D polygon) {
	ui_element result = {
		.type = UI_TYPE_POLY,
		.pos = pos,
		.color = color,
		.polygon = polygon,
	};

	return result;
}

ui_element new_ui_poly_proc(Vector2 pos, RGBA_Color color, ui_proc proc, Poly2D polygon) {
	ui_element result = new_ui_poly(pos, color, polygon);
	result.draw = proc;

	return result;
}

void draw_ui_element(ui_element* e, STBTTF_Font* font) {
	if (e->draw != 0) {
		e->draw(e);
	}

	switch(e->type) {
		case UI_TYPE_TEXTURE: {
			SDL_Texture* texture = e->texture.texture;
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
			if (font == NULL) { break; }
			char buf[64];
			size_t len = 0;
			if (e->text.str != 0 && e->text.str[0] != '\0') {
				len = SDL_strlcpy(buf, e->text.str, array_length(buf));
			}
			if (e->val.i) {
				SDL_itoa(*(e->val.i), buf+len, 10);
			}
			platform_set_render_draw_color(e->color);
			render_text_aligned(font, e->text.size, e->pos.x, e->pos.y, buf, e->text.align);
		} break;

		case UI_TYPE_POLY: {
			Poly2D p = translate_poly2d(e->polygon, e->pos);
			render_fill_polygon(p.vertices, p.vert_count, e->color);
		} break;

		case UI_TYPE_RECT: {
			Rectangle rect = translate_rect(e->rect, e->pos);
			platform_set_render_draw_color(e->color);
			platform_render_fill_rect(rect);
		} break;

		default: break;
	}

	for (int i = 0; i < e->num_children; i++) {
		(e->children+i)->pos = add_vector2(e->pos, (e->children+i)->pos);
		draw_ui_element(e->children+i, font);
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
			Rectangle* dest = &(e->texture.dest);
			if ( (dest->x+dest->y+dest->w+dest->h) == 0 ) {
				int width, height;
				SDL_QueryTexture(e->texture.texture, 0, 0, &width, &height);

				e->texture.dest.w = (float)width;
				e->texture.dest.h = (float)height;
			}

			e->texture.dest = scale_rect(e->texture.dest, (Vector2){scale, scale});
		} break;

		default: {} break;
	}

	for (ui_element* child = e->children; child != e->children+e->num_children; child++) {
		scale_ui_element(child, scale);
	}
}
