#ifndef UI_H
#define UI_H

#include "defs.h"

typedef struct ui_element {
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
		struct {Rectangle dest; char* name; } texture;
		struct {int size; char* str; char* align; } text;
	};

	void (*draw)(struct ui_element* element);
	
	int num_children;
	struct ui_element* children;
} ui_element;


void draw_ui_element(Game_State* game, ui_element* e);
void scale_ui_element(ui_element* e, float scale);
#endif
