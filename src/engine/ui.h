#ifndef UI_H
#define UI_H

#include "types.h"

typedef struct ui_element ui_element;
typedef void (ui_proc) (ui_element* element);

struct ui_element {
	Vec2_Union(pos, x, y);
	float angle;
	RGBA_Color color;
	union {
		int* i;
		float* f;
	} val;

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
		struct {Rectangle dest; SDL_Texture* texture; } texture;
		struct {int size; char* str; char* align; } text;
	};

	ui_proc* draw;
	
	int num_children;
	ui_element* children;
};

ui_element new_ui_text(Vector2 pos, int* val, RGBA_Color color, char* str, int size, const char* align);
ui_element new_ui_text_proc(Vector2 pos, int* val, RGBA_Color color, ui_proc proc, char* str, int size, const char* align);
ui_element new_ui_rect(Vector2 pos, float* val, RGBA_Color color, Rectangle rect);
ui_element new_ui_rect_proc(Vector2 pos, float* val, ui_proc proc, RGBA_Color color, Rectangle rect);
ui_element new_ui_poly(Vector2 pos, RGBA_Color color, Poly2D polygon);
ui_element new_ui_poly_proc(Vector2 pos, RGBA_Color color, ui_proc proc, Poly2D polygon);

void draw_ui_element(ui_element* e, STBTTF_Font* font);
void scale_ui_element(ui_element* e, float scale);
#endif
