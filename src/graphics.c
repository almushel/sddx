#include "SDL2/SDL.h"
#include "assets.h"

void render_draw_circle(SDL_Renderer* renderer, int cx, int cy, int r) {
	if (r <= 0) return;
	SDL_Point points[4];
	float r_squared = (float)r * (float)r;

	uint8_t* y_used = SDL_malloc(sizeof(uint8_t) * r);
	uint8_t* x_used = SDL_malloc(sizeof(uint8_t) * r);
	SDL_memset(y_used, 0, sizeof(uint8_t) * r);
	SDL_memset(x_used, 0, sizeof(uint8_t) * r);

	for (int x = 0; x <= r; x++) {
		int y = (int)SDL_roundf(SDL_sqrtf(r_squared - (float)x*(float)x));

		points[0] = (SDL_Point){cx + x, cy + y};
		points[1] = (SDL_Point){cx + x, cy - y};
		points[2] = (SDL_Point){cx - x, cy + y};
		points[3] = (SDL_Point){cx - x, cy - y};
	
		SDL_RenderDrawPoints(renderer, points, 4);

		x_used[x] = 1;
		y_used[y] = 1;
	}

	for (int y = 0; y <= r; y++) {
		if (y_used[y]) continue;
		int x = (int)SDL_roundf(SDL_sqrtf(r_squared - (float)y*(float)y));

		points[0] = (SDL_Point){cx + x, cy + y};
		points[1] = (SDL_Point){cx + x, cy - y};
		points[2] = (SDL_Point){cx - x, cy + y};
		points[3] = (SDL_Point){cx - x, cy - y};
	
		SDL_RenderDrawPoints(renderer, points, 4);
	}

	SDL_free(x_used);
	SDL_free(y_used);
}

// TO-DO: Identify crash related to this.
void render_draw_circlef(SDL_Renderer* renderer, float cx, float cy, float r) {
	if (r <= 0) return;
	SDL_FPoint points[4];
	float r_squared = r * r;
	int ri = (int)SDL_ceilf(r);
	
	SDL_bool* y_used = SDL_malloc(sizeof(SDL_bool) * ri);
	SDL_bool* x_used = SDL_malloc(sizeof(SDL_bool) * ri);
	SDL_memset(y_used, 0, sizeof(SDL_bool) * ri);
	SDL_memset(x_used, 0, sizeof(SDL_bool) * ri);

	for (int x = 0; x <= ri; x++) {
		float fx = (float)x;
		float fy = SDL_sqrtf(r_squared - fx*fx);

		points[0] = (SDL_FPoint){cx + fx, cy + fy};
		points[1] = (SDL_FPoint){cx + fx, cy - fy};
		points[2] = (SDL_FPoint){cx - fx, cy + fy};
		points[3] = (SDL_FPoint){cx - fx, cy - fy};
	
		SDL_RenderDrawPointsF(renderer, points, 4);

		x_used[x] = 1;
		y_used[(int)fy] = 1;
	}

	for (int y = 0; y <= ri; y++) {
		if (y_used[y]) continue;
		float fy = (float)y;
		float fx = SDL_sqrtf(r_squared - fy*fy);

		points[0] = (SDL_FPoint){cx + fx, cy + fy};
		points[1] = (SDL_FPoint){cx + fx, cy - fy};
		points[2] = (SDL_FPoint){cx - fx, cy + fy};
		points[3] = (SDL_FPoint){cx - fx, cy - fy};
	
		SDL_RenderDrawPointsF(renderer, points, 4);
	}

	SDL_free(x_used);
	SDL_free(y_used);
}

void render_fill_circle(SDL_Renderer* renderer, int cx, int cy, int r) {
	if (r <= 0) return;
	SDL_Point points[4];
	int radius = (int)r;
	int r_squared = radius*radius;

	for (int y = 0; y <= radius; y++) {
		for (int x = 0; x <= radius; x++) {
			if(x*x + y*y <= r_squared + r) {
				points[0] = (SDL_Point){cx + x, cy + y};
				points[1] = (SDL_Point){cx + x, cy - y};
				points[2] = (SDL_Point){cx - x, cy + y};
				points[3] = (SDL_Point){cx - x, cy - y};

				SDL_RenderDrawPoints(renderer, points, 4);
			}
		}
	}
}

// TO-DO: Figure out why sub-pixel rendering doesn't seem to be working here. Very clear pixel jitter on moving objects.
void render_fill_circlef(SDL_Renderer* renderer, float cx, float cy, float r) {
	if (r <= 0) return;
	SDL_FPoint points[4];
	int radius = (int)SDL_roundf(r);
	float r_squared = r*r;

	for (int y = 0; y <= radius; y++) {
		for (int x = 0; x <= radius; x++) {
			float fx = (float)x;
			float fy = (float)y;

			if(fx*fx + fy*fy <= r_squared + r) {
				points[0] = (SDL_FPoint){cx + fx, cy + fy};
				points[1] = (SDL_FPoint){cx + fx, cy - fy};
				points[2] = (SDL_FPoint){cx - fx, cy + fy};
				points[3] = (SDL_FPoint){cx - fx, cy - fy};

				SDL_RenderDrawPointsF(renderer, points, 4);
			}
		}
	}
}

static inline float lerp(float start, float end, float t) {
	t = SDL_clamp(t, 0.0f, 1.0f);

	return (1.0f - t) * start + (t * end);
}

void render_fill_circlef_linear_gradient(SDL_Renderer* renderer, float cx, float cy, float r, RGB_Color start_color, RGB_Color end_color) {
	if (r <= 0) return;
	SDL_FPoint points[4];
	int radius = (int)SDL_roundf(r);
	float r_squared = r*r;

	for (int y = 0; y <= radius; y++) {
		for (int x = 0; x <= radius; x++) {
			float fx = (float)x;
			float fy = (float)y;

			float t = (fx*fx + fy*fy) / (r_squared + r);

			if (t <= 1.0f) {
				SDL_SetRenderDrawColor(renderer, 
					(Uint8)lerp(start_color.r, end_color.r, t),
					(Uint8)lerp(start_color.g, end_color.g, t),
					(Uint8)lerp(start_color.b, end_color.b, t),
					255
				);

				points[0] = (SDL_FPoint){cx + fx, cy + fy};
				points[1] = (SDL_FPoint){cx + fx, cy - fy};
				points[2] = (SDL_FPoint){cx - fx, cy + fy};
				points[3] = (SDL_FPoint){cx - fx, cy - fy};

				SDL_RenderDrawPointsF(renderer, points, 4);
			}
		}
	}
}

void render_draw_texture(SDL_Renderer* renderer, SDL_Texture* texture, float x, float y, float angle, SDL_bool centered) {
	if (texture) {
		int dest_w, dest_h;

		if (SDL_QueryTexture(texture, NULL, NULL, &dest_w, &dest_h) != 0) {
			SDL_Log("render_draw_texture: QueryTexture failed.");
		}

		SDL_FRect dest_rect;
		dest_rect.x = x;
		dest_rect.y = y;
		dest_rect.w = (float)dest_w;
		dest_rect.h = (float)dest_h;

		if (centered == SDL_TRUE) {
			dest_rect.x -= dest_rect.w/2.0f;
			dest_rect.y -= dest_rect.h/2.0f;
		}

		if (SDL_RenderCopyExF(renderer, texture, NULL, &dest_rect, angle, 0, SDL_FLIP_NONE) == -1) {
			SDL_Log("render_draw_texture: RenderCopy failed. %s", SDL_GetError());
		};
	}
}

void render_draw_game_sprite(Game_State* game, Game_Sprite* sprite, Transform2D transform, SDL_bool centered) {
	SDL_Texture* texture =  game_get_texture(game, sprite->texture_name);

	if (texture) {
		SDL_Rect sprite_rect = get_sprite_rect(game, sprite);

		SDL_FRect dest_rect;
		dest_rect.x = transform.x;
		dest_rect.y = transform.y;
		dest_rect.w = (float)sprite_rect.w;
		dest_rect.h = (float)sprite_rect.h;

		if (transform.sx > 0.0f && transform.sy > 0.0f) {
			dest_rect.w *= transform.sx;
			dest_rect.h *= transform.sy;

			if (centered == SDL_TRUE) {
				dest_rect.x -= dest_rect.w/2.0f;
				dest_rect.y -= dest_rect.h/2.0f;
			}

			SDL_RenderCopyExF(game->renderer, texture, &sprite_rect, &dest_rect, transform.angle * (float)(int)sprite->rotation, 0, SDL_FLIP_NONE);
		}
	}
}

SDL_Vertex* pack_sdl_vertices(Vector2* positions, Vector2* tex_coords, RGB_Color color, int vert_count) {
	SDL_Vertex* result = SDL_malloc(sizeof(SDL_Vertex) * vert_count);

	for (int i = 0; i < vert_count; i++) {
		if (positions)
			result[i].position = (SDL_FPoint){positions[i].x, positions[i].y};

		if (tex_coords)
			result[i].tex_coord = (SDL_FPoint){tex_coords[i].x, tex_coords[i].y};
		
		result[i].color = (SDL_Color){color.r, color.g, color.b, 255};
	}

	return result;
}

void render_draw_polygon(SDL_Renderer* renderer, SDL_FPoint* points, int num_points) {
	SDL_RenderDrawLinesF(renderer, points, num_points);
	SDL_RenderDrawLineF(renderer, points[num_points-1].x, points[num_points-1].y, points[0].x, points[0].y);
}

void render_fill_polygon(SDL_Renderer* renderer, SDL_FPoint* points, int num_points, RGB_Color color) {
	int num_verts = num_points;
	SDL_Vertex* vertices = SDL_malloc(sizeof(SDL_Vertex) * num_points);
	
	for (int i = 0; i < num_verts; i++) {
		vertices[i].position  	= points[i];
		vertices[i].tex_coord 	= (SDL_FPoint) {1.0f,1.0f};
		vertices[i].color 		= (SDL_Color){color.r, color.g, color.b, 255};
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

	SDL_RenderGeometry(renderer, NULL, vertices, num_verts, indices, num_indices);

	SDL_free(indices);
	SDL_free(vertices);
}

void render_draw_triangle(SDL_Renderer* renderer, Vector2 v1, Vector2 v2, Vector2 v3) {
	SDL_FPoint points[4];
	points[0] = (SDL_FPoint){v1.x, v1.y};
	points[1] = (SDL_FPoint){v2.x, v2.y};
	points[2] = (SDL_FPoint){v3.x, v3.y};
	points[3] = (SDL_FPoint){v1.x, v1.y};

	SDL_RenderDrawLinesF(renderer, points, 4);
}

void render_fill_triangle(SDL_Renderer* renderer, Vector2 v1, Vector2 v2, Vector2 v3, RGB_Color color) {
	SDL_Vertex vertex_1 = { {v1.x, v1.y}, {color.r, color.g, color.b, 255}, {1, 1}};
	SDL_Vertex vertex_2 = { {v2.x, v2.y}, {color.r, color.g, color.b, 255}, {1, 1}};
	SDL_Vertex vertex_3 = { {v3.x, v3.y}, {color.r, color.g, color.b, 255}, {1, 1}};

	// Put them into array

	SDL_Vertex vertices[] = {
		vertex_1,
		vertex_2,
		vertex_3
	};

	SDL_RenderGeometry(renderer, NULL, vertices, 3, 0, 0);
}