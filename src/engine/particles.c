#include "assets.h"
#include "graphics.h"
#include "math.h"
#include "types.h"

#define PARTICLE_LIFETIME 12
#define PARTICLE_MIN_SCALE 0.01
#define PARTICLE_MAX_START_RADIUS 7
#define PARTICLE_MIN_START_RADIUS 3
#define PARTICLE_SPEED 6
#define PARTICLE_DECAY 0.75f
#define EXPLOSION_STARTING_PARTICLES 12
#define DEFAULT_PARTICLE_COLOR (RGBA_Color){255, 255, 255, 255}
#define DEAD_PARTICLE_MAX 16

#define MAX_PARTICLES 512
#define MAX_PARTICLE_EMITTERS 128

struct Particle_System {
	Particle particles[MAX_PARTICLES];
	Uint32 particle_count;

	Particle_Emitter emitters[MAX_PARTICLE_EMITTERS];
	Uint32 dead_emitters[MAX_PARTICLE_EMITTERS];
	Uint32 emitter_count;
	Uint32 dead_emitter_count;
};

Particle_System* new_particle_system(void) {
	Particle_System* result = SDL_calloc(1, sizeof(Particle_System));
	result->emitter_count = 1;
	return result;
}

void reset_particle_system(Particle_System* ps) {
	ps->particle_count = 0;
	ps->dead_emitter_count = 0;
	ps->emitter_count = 1;
}

static void update_particle_emitters(Particle_System* ps, float dt);

void update_particles(Particle_System* ps, float dt) {
	update_particle_emitters(ps, dt);	

	Uint32 dead_particles[DEAD_PARTICLE_MAX];
	Uint32 dead_particle_count = 0;
	for (int p = 0; p < ps->particle_count; p++) {
		Particle* particle = ps->particles + p;
		
		if (randomf() * 100 > 50) {
			particle->timer -= PARTICLE_DECAY * dt;
		}
		
		if (particle->timer <= 0 && dead_particle_count < array_length(dead_particles)) {
			dead_particles[dead_particle_count] = p;
			dead_particle_count++;
		}

		particle->x += particle->vx * dt;
		particle->y += particle->vy * dt;
	}
	
	for (Uint32 d = 0; d < dead_particle_count; d++) {
		if (dead_particles[d] != ps->particle_count-1) {
			*(ps->particles + dead_particles[d]) = *(ps->particles + (ps->particle_count-1));
		}
		ps->particle_count--;
	}
}

void wrap_particles(Particle_System* ps, Rectangle bounds) {
	for (Particle* p = ps->particles; p != ps->particles+ps->particle_count; p++) {
		p->position = wrap_coords(p->x, p->y, bounds.x, bounds.y, bounds.x+bounds.w, bounds.y+bounds.h);
	}
}

void displace_particles(Particle_System* ps, Transform2D transform, Game_Shape shape) {
	for (Particle* p = ps->particles; p != ps->particles+ps->particle_count; p++) {
		Vector2 overlap = {0};
		Game_Shape particle_shape = scale_game_shape(p->shape, p->scale);

		if ( check_shape_collision(p->transform, particle_shape, transform, shape, &overlap)) {
			p->x -= overlap.x;
			p->y -= overlap.y;

			float magnitude = vector2_length(p->velocity);

			p->velocity = normalize_vector2(p->velocity);

			Vector2 new_v = {-overlap.x, -overlap.y};//subtract_vector2(p->position, entity->position);
					new_v = normalize_vector2(new_v);
					new_v = add_vector2(new_v, p->velocity);
					new_v = normalize_vector2(new_v);
			
			p->velocity = scale_vector2(new_v, magnitude / 2.0f);
		}
	}
}

void draw_particles(Particle_System* ps, Game_Assets* assets) {
	Particle* particle;
	for (int p = 0; p < ps->particle_count; p++) {
		particle = ps->particles + p;
		if (particle->timer <= 0) continue;

		float scale = SDL_clamp(particle->timer / (float)PARTICLE_LIFETIME, 0.0f, 1.0f);

		particle->sx = scale;
		particle->sy = scale;

		if (particle->sprite.texture_name) {
			render_draw_game_sprite(assets, &particle->sprite, particle->transform, 1);
		} else {
			Game_Shape 	shape = particle->shape;
			shape = scale_game_shape(shape, particle->scale);

			if (shape.type == SHAPE_TYPE_RECT) {
				shape.rectangle.x = -shape.rectangle.w/2.0f;
				shape.rectangle.y = -shape.rectangle.h/2.0f;
			}

			shape = rotate_game_shape(shape, particle->angle);

			render_fill_game_shape(particle->position, shape, particle->color);
		}
	}
}

void init_particle(Particle* p, Game_Shape_Types shape) {
	*p = (Particle) {0};
	switch(shape) {
		case SHAPE_TYPE_RECT: 	{
			p->shape.rectangle.x = -PARTICLE_MAX_START_RADIUS;
			p->shape.rectangle.y = -PARTICLE_MAX_START_RADIUS;
			p->shape.rectangle.w = (float)PARTICLE_MAX_START_RADIUS*2.0f;
			p->shape.rectangle.h = (float)PARTICLE_MAX_START_RADIUS*2.0f;
		} break;
		
		case SHAPE_TYPE_CIRCLE: {
			p->shape.radius = PARTICLE_MAX_START_RADIUS;
		} break;

		case SHAPE_TYPE_POLY2D: {
			p->shape.type = SHAPE_TYPE_POLY2D;
			p->shape.polygon = generate_poly2D(5, PARTICLE_MAX_START_RADIUS, PARTICLE_MAX_START_RADIUS/2);
		} break;
	}
	p->shape.type = shape;
	p->timer = PARTICLE_LIFETIME,
	p->color = DEFAULT_PARTICLE_COLOR,
	p->sx = p->sy = 1.0f;
}

Uint32 get_new_particle(Particle_System* ps) {
	Uint32 result = 0;
	if (ps->particle_count == 0) ps->particle_count++;
	if (ps->particle_count < MAX_PARTICLES) {
		result = ps->particle_count++;
	} else {
		SDL_Log("get_new_particle(): max Particle count reached");
	}

	return result;
}

Uint32 spawn_particle(Particle_System* ps, Game_Sprite* sprite, Game_Shape_Types shape) {
	Uint32 result = get_new_particle(ps);
	if (result) {
		Particle* particle = ps->particles + result;
		init_particle(particle, shape);
		if (sprite) particle->sprite = *sprite;
	}

	return result;
}
	
static float random_particle_radius() {
	float result = PARTICLE_MIN_START_RADIUS + SDL_floorf(randomf() * (PARTICLE_MAX_START_RADIUS - PARTICLE_MIN_START_RADIUS));
		
	return result;
}

static RGBA_Color random_color(RGBA_Color* colors, Uint32 color_count) {
	RGBA_Color result = {0};
	if (colors == 0 || color_count == 0) {
		RGBA_Color default_color = DEFAULT_PARTICLE_COLOR;
		color_count = 1;
		colors = &default_color;
	}

	result = colors[ (Uint32)(randomf() * (color_count)) ]; 
	return result;
}

void randomize_particle(Particle* p, RGBA_Color* colors, Uint32 color_count) {
	float angle = randomf() * 360.0f;

	float radius = random_particle_radius();
	switch(p->shape.type) {
		case SHAPE_TYPE_CIRCLE: {
			p->shape.radius = radius;
		} break;
		
		case SHAPE_TYPE_RECT: {
			p->shape.rectangle.x = 0.0f;
			p->shape.rectangle.y = 0.0f;
			p->shape.rectangle.w =  radius*2.0f;
			p->shape.rectangle.h =  radius*2.0f;
		} break;

		case SHAPE_TYPE_POLY2D: {
			int vert_count = SDL_clamp(3 + (randomf() * 5.0f), 3, MAX_POLY2D_VERTS);
			float angle_increment = 360.0f / (float)vert_count;

			float angle = 0;
			for (int v = 0; v < vert_count; v++) {

				p->shape.polygon.vertices[v] = (Vector2) {
					cos_deg(angle) * radius,
					sin_deg(angle) * radius,
				};

				angle += angle_increment;
				radius = random_particle_radius();
			}
		} break;
		default: { break; }
	}
	p->color = random_color(colors, color_count); 
	p->vx = cos_deg(angle) * (float)PARTICLE_SPEED;
	p->vy = sin_deg(angle) * (float)PARTICLE_SPEED;
}

void explode_at_point(Particle_System* ps, float x, float y, RGBA_Color* colors, Uint32 num_colors, Game_Sprite* sprite, Game_Shape_Types shape) {
//	if (force != 0) {force_circle(x, x, 120, force); }
	for (int p = 0; p < EXPLOSION_STARTING_PARTICLES; p++) {
		Uint32 id = spawn_particle(ps, sprite, shape);
		if (id) {
			Particle* p = ps->particles + id;
			randomize_particle(p, colors, num_colors);
			p->x = x;
			p->y = y;
		}

	}
}

// TO-DO: Fix broken particle emitters
// Maybe need to reserve zero index?
Uint32 get_new_particle_emitter(Particle_System* ps) {
	Uint32 result = 0;
	if (ps->dead_emitter_count > 0) {
		ps->dead_emitter_count--;
		result = ps->dead_emitters[ps->dead_emitter_count];
	} else if (ps->emitter_count < array_length(ps->emitters)) {
		result = ps->emitter_count++;
	} else {
		SDL_Log("get_get_new_particle_emitter(): Particle emitter maximum reached");
	}

	if (result) {
		*(ps->emitters + result) = (Particle_Emitter) {
			.scale = {1.0f,1.0f},
		};
	}

	return result;
}

Particle_Emitter* get_particle_emitter(Particle_System* ps, Uint32 index) {
	Particle_Emitter* result = 0;

	if (index && index < ps->emitter_count) result = ps->emitters + index;

	return result;
}

void remove_particle_emitter(Particle_System* ps, Uint32 index) {
	if (index > ps->emitter_count) return;
	if (index == ps->emitter_count-1) {
		ps->emitter_count--;
	} else {
		ps->emitters[index].state = EMITTER_STATE_DEAD;
		ps->dead_emitters[ps->dead_emitter_count] = index;
		ps->dead_emitter_count++;
	}
}

static void update_particle_emitters(Particle_System* ps, float dt) {
	for (int emitter_index = 1; emitter_index < ps->emitter_count; emitter_index++) {
		Particle_Emitter* emitter = ps->emitters + emitter_index;
		if (emitter->state != EMITTER_STATE_ACTIVE) continue;
		
		emitter->counter += emitter->density * dt;

		if (emitter->counter >= 1.0f) {
			int particles_to_emit = (int)emitter->counter;
			while (particles_to_emit) {
				Uint32 id = spawn_particle(ps, 0, emitter->shape);
				if (id) {
					Particle* p = ps->particles + id;
					randomize_particle(p, emitter->colors, emitter->color_count);

					p->shape = scale_game_shape(p->shape, emitter->scale);
					
					p->x = emitter->x;
					p->y = emitter->y;

					p->vx = cos_deg(emitter->angle) * emitter->speed;
					p->vy = sin_deg(emitter->angle) * emitter->speed;
				}
				particles_to_emit--;
			}

			emitter->counter -= (int)emitter->counter;
		}
	}
}

void explode_sprite(Game_Assets* assets, Particle_System* ps, Game_Sprite* sprite, float x, float y, float angle, int pieces) {
	float angle_division = 360.0f / (float)pieces;
	float random_deviation = 0;//angle_division * 1.8;

	Vector2 sprite_offset = rotate_vector2(sprite->offset, angle);

	Game_Sprite* chunks = divide_sprite(assets, sprite, pieces);

	float radius = (chunks[0].src_rect.w + chunks[0].src_rect.h) / 2;
	int cHalf = pieces / 2;

	// Create explosion using chunks as particle sprites
	for (int chunk_index = 0; chunk_index < pieces; chunk_index++) {
		Uint32 particle_id = spawn_particle(ps, chunks + chunk_index, SHAPE_TYPE_CIRCLE);
		if (particle_id) {
			const float random_deviation = 30.0f;

			float chunk_angle = angle + 180.0f + (chunk_index * angle_division);
				chunk_angle += random_deviation/2.0f - (randomf() * random_deviation);
			float vx = cos_deg(chunk_angle) * (float)PARTICLE_SPEED;
			float vy = sin_deg(chunk_angle) * (float)PARTICLE_SPEED;

			Particle* particle = ps->particles + particle_id;
			randomize_particle(particle, 0, 0);
			particle->timer = PARTICLE_LIFETIME;
			particle->angle = angle;
			particle->x = x + sprite_offset.x;
			particle->y = y + sprite_offset.y;
			particle->vx = vx;
			particle->vy = vy;
		}
	}

	SDL_free(chunks);
}
