#include "platform.h"
#include "math.h"
#include "assets.h"

float measure_text(STBTTF_Font* font, float size, const char* text) {
	float result = 0; // width
	float scale = size / font->size;
	for (int i = 0; text[i]; i++) {
		if (text[i] >= 32 && text[i] < 128) {
			stbtt_packedchar* info = &font->chars[text[i] - 32];
			result += info->xadvance * scale;
		}
	}

	return result;
}

void render_text(STBTTF_Font* font, float size, float x, float y, const char* text) {
	RGBA_Color color = platform_get_render_draw_color();
	SDL_SetTextureColorMod(font->atlas, color.r, color.g, color.b);
	SDL_SetTextureAlphaMod(font->atlas, color.a);

	float scale = size / font->size;

	for (int i = 0; text[i]; i++) {
		if (text[i] >= 32 && text[i] < 128) {

			stbtt_packedchar* info = &font->chars[text[i] - 32];
			Rectangle src_rect = {info->x0, info->y0, info->x1 - info->x0, info->y1 - info->y0};

			Rectangle dst_rect = {
				x + info->xoff * scale, 
				y + info->yoff * scale, 
				(info->x1 - info->x0) * scale, 
				(info->y1 - info->y0) * scale 
			};

			platform_render_copy(font->atlas, &src_rect, &dst_rect, 0, 0, 0);
			x += info->xadvance * scale;
		}
	}

}

void render_text_aligned(STBTTF_Font* font, float size, float x, float y, const char* text, const char* alignment) {
	float offset_x = x;
	float offset_y = y;

	if (alignment) {
		float text_width = measure_text(font, size, text);
		if (SDL_strcmp(alignment, "center") == 0) {
			offset_x -= text_width / 2.0f;
		}
		else if (SDL_strcmp(alignment, "right") == 0) {
			offset_x -= text_width;	
		}
	}

	render_text(font, size, offset_x, offset_y, text);
}

void render_draw_circle(int cx, int cy, int r) {
	if (r <= 0) return;
	Vector2 points[4];
	float r_squared = (float)r * (float)r;

	uint8_t* y_used = SDL_malloc(sizeof(uint8_t) * r);
	uint8_t* x_used = SDL_malloc(sizeof(uint8_t) * r);
	SDL_memset(y_used, 0, sizeof(uint8_t) * r);
	SDL_memset(x_used, 0, sizeof(uint8_t) * r);

	for (int x = 0; x <= r; x++) {
		int y = (int)SDL_roundf(SDL_sqrtf(r_squared - (float)x*(float)x));

		points[0] = (Vector2){cx + x, cy + y};
		points[1] = (Vector2){cx + x, cy - y};
		points[2] = (Vector2){cx - x, cy + y};
		points[3] = (Vector2){cx - x, cy - y};
	
		platform_render_draw_points(points, 4);

		x_used[x] = 1;
		y_used[y] = 1;
	}

	for (int y = 0; y <= r; y++) {
		if (y_used[y]) continue;
		int x = (int)SDL_roundf(SDL_sqrtf(r_squared - (float)y*(float)y));

		points[0] = (Vector2){cx + x, cy + y};
		points[1] = (Vector2){cx + x, cy - y};
		points[2] = (Vector2){cx - x, cy + y};
		points[3] = (Vector2){cx - x, cy - y};
	
		platform_render_draw_points(points, 4);
	}

	SDL_free(x_used);
	SDL_free(y_used);
}

// TO-DO: Identify crash related to this. This function is bad.
void render_draw_circlef(float cx, float cy, float r) {
	if (r <= 0) return;
	Vector2 points[4];
	float r_squared = r * r;
	int ri = (int)SDL_ceilf(r);
	
	SDL_bool* y_used = SDL_malloc(sizeof(SDL_bool) * ri);
	SDL_bool* x_used = SDL_malloc(sizeof(SDL_bool) * ri);
	SDL_memset(y_used, 0, sizeof(SDL_bool) * ri);
	SDL_memset(x_used, 0, sizeof(SDL_bool) * ri);

	for (int x = 0; x <= ri; x++) {
		float fx = (float)x;
		float fy = (x == ri) ? 0 : SDL_sqrtf(r_squared - fx*fx);
		fy = SDL_clamp(fy, 0, ri);

		points[0] = (Vector2){cx + fx, cy + fy};
		points[1] = (Vector2){cx + fx, cy - fy};
		points[2] = (Vector2){cx - fx, cy + fy};
		points[3] = (Vector2){cx - fx, cy - fy};
	
		platform_render_draw_points(points, 4);

		x_used[x] = 1;
		y_used[(int)fy] = 1;
	}

	for (int y = 0; y <= ri; y++) {
		if (y_used[y]) continue;
		float fy = (float)y;
		float fx = (y == ri) ? 0 : SDL_sqrtf(r_squared - fy*fy);
		fy = SDL_clamp(fy, 0, ri);
		
		points[0] = (Vector2){cx + fx, cy + fy};
		points[1] = (Vector2){cx + fx, cy - fy};
		points[2] = (Vector2){cx - fx, cy + fy};
		points[3] = (Vector2){cx - fx, cy - fy};
	
		platform_render_draw_points(points, 4);
	}

	SDL_free(x_used);
	SDL_free(y_used);
}

void render_fill_circle(int cx, int cy, int r) {
	if (r <= 0) return;
	Vector2 points[4];
	int radius = (int)r;
	int r_squared = radius*radius;

	for (int y = 0; y <= radius; y++) {
		for (int x = 0; x <= radius; x++) {
			if(x*x + y*y <= r_squared + r) {
				points[0] = (Vector2){cx + x, cy + y};
				points[1] = (Vector2){cx + x, cy - y};
				points[2] = (Vector2){cx - x, cy + y};
				points[3] = (Vector2){cx - x, cy - y};

				platform_render_draw_points(points, 4);
			}
		}
	}
}

// TO-DO: Figure out why sub-pixel rendering doesn't seem to be working here. Very clear pixel jitter on moving objects.
void render_fill_circlef(float cx, float cy, float r) {
	if (r <= 0) return;
	Vector2 points[4];
	int radius = (int)SDL_roundf(r);
	float r_squared = r*r;

	for (int y = 0; y <= radius; y++) {
		for (int x = 0; x <= radius; x++) {
			float fx = (float)x;
			float fy = (float)y;

			if(fx*fx + fy*fy <= r_squared + r) {
				points[0] = (Vector2){cx + fx, cy + fy};
				points[1] = (Vector2){cx + fx, cy - fy};
				points[2] = (Vector2){cx - fx, cy + fy};
				points[3] = (Vector2){cx - fx, cy - fy};

				platform_render_draw_points(points, 4);
			}
		}
	}
}

void render_fill_circlef_linear_gradient(float cx, float cy, float r, RGBA_Color start_color, RGBA_Color end_color) {
	if (r <= 0) return;
	Vector2 points[4];
	int radius = (int)SDL_roundf(r);
	float r_squared = r*r;

	for (int y = 0; y <= radius; y++) {
		for (int x = 0; x <= radius; x++) {
			float fx = (float)x;
			float fy = (float)y;

			float t = (fx*fx + fy*fy) / (r_squared + r);

			if (t <= 1.0f) {
				platform_set_render_draw_color( 
					(RGBA_Color) {
						lerp(start_color.r, end_color.r, t),
						lerp(start_color.g, end_color.g, t),
						lerp(start_color.b, end_color.b, t),
						lerp(start_color.a, end_color.a, t),
					}
				);

				points[0] = (Vector2){cx + fx, cy + fy};
				points[1] = (Vector2){cx + fx, cy - fy};
				points[2] = (Vector2){cx - fx, cy + fy};
				points[3] = (Vector2){cx - fx, cy - fy};

				platform_render_draw_points(points, 4);
			}
		}
	}
}

void render_draw_texture(SDL_Texture* texture, float x, float y, float angle, SDL_bool centered) {
	if (texture) {
		Vector2 dim = platform_get_texture_dimensions(texture);

		Rectangle dest_rect;
		dest_rect.x = x;
		dest_rect.y = y;
		dest_rect.w = dim.x;
		dest_rect.h = dim.y;

		if (centered == SDL_TRUE) {
			dest_rect.x -= dest_rect.w/2.0f;
			dest_rect.y -= dest_rect.h/2.0f;
		}

		if (platform_render_copy(texture, NULL, &dest_rect, angle, 0, SDL_FLIP_NONE) == -1) {
			SDL_Log("render_draw_texture: RenderCopy failed. %s", SDL_GetError());
		};
	}
}

void render_draw_game_sprite(Game_Assets* assets, Game_Sprite* sprite, Transform2D transform, SDL_bool centered) {
	SDL_Texture* texture =  assets_get_texture(assets, sprite->texture_name);

	if (texture) {
		Rectangle sprite_rect = get_sprite_rect(assets, sprite);

		Rectangle dest_rect;
		dest_rect.x = transform.x;
		dest_rect.y = transform.y;
		dest_rect.w = (float)sprite_rect.w;
		dest_rect.h = (float)sprite_rect.h;

		if (transform.sx > 0.0f && transform.sy > 0.0f) {
			float angle = transform.angle * (float)(int)sprite->rotation_enabled;
			Vector2 offset = rotate_vector2(sprite->offset, angle);

			if (offset.x) dest_rect.x += offset.x * transform.sx;
			if (offset.y) dest_rect.y += offset.y * transform.sy;

			dest_rect.w *= transform.sx;
			dest_rect.h *= transform.sy;

			if (centered == SDL_TRUE) {
				dest_rect.x -= dest_rect.w/2.0f;
				dest_rect.y -= dest_rect.h/2.0f;
			}

			platform_render_copy(texture, &sprite_rect, &dest_rect, angle, 0, 0);
		}
	}
}

SDL_Vertex* pack_sdl_vertices(Vector2* positions, Vector2* tex_coords, RGBA_Color color, int vert_count) {
	SDL_Vertex* result = SDL_malloc(sizeof(SDL_Vertex) * vert_count);

	for (int i = 0; i < vert_count; i++) {
		if (positions)
			result[i].position = (SDL_FPoint){positions[i].x, positions[i].y};

		if (tex_coords)
			result[i].tex_coord = (SDL_FPoint){tex_coords[i].x, tex_coords[i].y};
		
		result[i].color = (SDL_Color){color.r,color.g,color.b,color.a};
	}

	return result;
}

void render_draw_polygon(Vector2* points, int num_points) {
	Vector2 connection[2] = {
		{points[num_points-1].x, points[num_points-1].y}, 
		{points[0].x, points[0].y},
	};
	
	platform_render_draw_lines(points, num_points);
	platform_render_draw_lines(connection, 2);
}

void render_fill_polygon(Vector2* points, int num_points, RGBA_Color color) {
	int num_verts = num_points;
	SDL_Vertex* vertices = SDL_malloc(sizeof(SDL_Vertex) * num_points);
	
	for (int i = 0; i < num_verts; i++) {
		vertices[i].position  	= (SDL_FPoint) {points[i].x, points[i].y};
		vertices[i].tex_coord 	= (SDL_FPoint) {1.0f,1.0f};
		vertices[i].color 		= (SDL_Color){color.r, color.g, color.b, color.a};
	}

	// Number of triangles in a polygon = number of verticles - 2;
	int num_triangles = (num_verts-2);
	int num_indices = num_triangles * 3;
	int * indices = SDL_malloc(sizeof(int) * num_indices);
	
	int next_index = 1;
	for (int triangle = 0; triangle < num_triangles; triangle++) {
		int i = triangle*3;
		indices[i	 ] 	= 0;
		indices[i + 1] 	= next_index++;
		indices[i + 2] 	= next_index;
	}

	platform_render_geometry(NULL, vertices, num_verts, indices, num_indices);

	SDL_free(indices);
	SDL_free(vertices);
}

void render_draw_triangle(Vector2 v1, Vector2 v2, Vector2 v3) {
	Vector2 points[4] = {v1, v2, v3, v1};
	platform_render_draw_lines(points, 4);
}

void render_fill_triangle(Vector2 v1, Vector2 v2, Vector2 v3, RGBA_Color color) {
	SDL_Vertex vertex_1 = { {v1.x, v1.y}, {color.r, color.g, color.b, color.a}, {1, 1}};
	SDL_Vertex vertex_2 = { {v2.x, v2.y}, {color.r, color.g, color.b, color.a}, {1, 1}};
	SDL_Vertex vertex_3 = { {v3.x, v3.y}, {color.r, color.g, color.b, color.a}, {1, 1}};

	// Put them into array

	SDL_Vertex vertices[] = { vertex_1, vertex_2, vertex_3 };

	platform_render_geometry(NULL, vertices, 3, 0, 0);
}

void render_draw_game_shape(Vector2 position, Game_Shape shape, RGBA_Color color) {
	platform_set_render_draw_color(color);
	switch(shape.type) {
		case SHAPE_TYPE_CIRCLE: {
			render_draw_circle(position.x, position.y, shape.radius);
		} break;
		case SHAPE_TYPE_RECT: {
			Rectangle rect = translate_rect(shape.rectangle, position);
			platform_render_draw_rect(rect);
		} break;
		case SHAPE_TYPE_POLY2D: {
			Poly2D polygon = translate_poly2d(shape.polygon, position);
			render_draw_polygon(polygon.vertices, shape.polygon.vert_count);
		} break;
	}
}

void render_fill_game_shape(Vector2 position, Game_Shape shape, RGBA_Color color) {
	switch(shape.type) {
		case SHAPE_TYPE_CIRCLE: {
			platform_set_render_draw_color(color);
			render_fill_circlef(position.x, position.y, shape.radius);
		} break;

		case SHAPE_TYPE_RECT: {
			platform_set_render_draw_color(color);
			Rectangle rect = translate_rect(shape.rectangle, position);
			platform_render_fill_rect(rect);
		} break;

		case SHAPE_TYPE_POLY2D: {
			Poly2D polygon = translate_poly2d(shape.polygon, position);
			render_fill_polygon(polygon.vertices, shape.polygon.vert_count, color);
		} break;
	}
}
