#include "SDL2/SDL.h"
#include "stdlib.h"
#include "defs.h"

#define PARTICLE_LIFETIME 15
#define PARTICLE_MAX_START_RADIUS 7
#define PARTICLE_MIN_START_RADIUS 3
#define PARTICLE_SPEED 6
#define PARTICLE_DECAY 0.75f
#define EXPLOSION_STARTING_PARTICLES 12
#define DEFAULT_PARTICLE_COLOR (rgb_color){255, 255, 255}

// Returns a pseudo-random value between 0 and 1
float random(void) {
	static SDL_bool seeded = 0;
	if (!seeded) {
		srand(42);
		seeded = 1;
	}

	float result = (float)(rand() % 1000) / 1000.0f;

	return result;
}

void update_particles(Particle_System* ps, float dt) {
	Uint32* dead_particles = SDL_malloc(sizeof(Uint32) * ps->particle_count);
	SDL_memset(dead_particles, 0, sizeof(Uint32) * ps->particle_count);
	Uint32 dead_particle_count = 0;

	for (int p = 0; p < ps->particle_count; p++) {
		Particle* particle = ps->particles + p;
		
		if (random() * 100 > 50){
			particle->life_left -= PARTICLE_DECAY * dt;
			particle->collision_radius -= PARTICLE_DECAY * dt;
			if (particle->collision_radius < 0) particle->collision_radius = 0;
		}
		
		if (particle->life_left <= 0) {
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

	SDL_free(dead_particles);
}

void draw_particles(Game_State* game, SDL_Renderer* renderer) {
	Particle_System* ps = &game->particle_system;
	for (int p = 0; p < ps->particle_count; p++) {
		Particle particle = ps->particles[p];
		if (particle.sprite.texture) {
			// TO-DO: Scale particles based on lifetime elaspsed (particle.life_left / PARTICLE_LIFETIME)
			draw_game_sprite(game, &particle.sprite, (Transform2D){particle.x, particle.y, particle.angle}, 1);
		} else {
			switch (particle.shape) {
//				case PARTICLE_SHAPE_RECT:
				default: {
					SDL_SetRenderDrawColor(renderer, particle.color.r, particle.color.g, particle.color.b, 255);
					
					SDL_FRect p_rect;
					p_rect.x = particle.x - particle.collision_radius;
					p_rect.y = particle.y - particle.collision_radius;
					p_rect.w = particle.collision_radius*2.0f;
					p_rect.h = p_rect.w;

					SDL_RenderFillRectF(renderer, &p_rect);
				} break;
			}
		}
	}
}

void init_particle(Particle* p) {
	*p = (Particle) {
		.collision_radius = PARTICLE_MAX_START_RADIUS,
		.life_left = PARTICLE_LIFETIME,
		.color = DEFAULT_PARTICLE_COLOR,
	};
}

Particle* get_particle(Particle_System* ps) {
	Particle* result = 0;
	if (ps->particle_count < MAX_PARTICLES) {
		result = ps->particles + ps->particle_count++;
	} else {
		SDL_Log("get_particle(): max Particle count reached");
	}

	return result;
}

Particle* instantiate_particle(Particle_System* ps, Game_Sprite* sprite, Particle_Shape shape) {
	Particle* result = get_particle(ps);
	if (result) {
		init_particle(result);
		if (sprite) result->sprite = *sprite;
		result->shape = shape;
	}

	return result;
}
	
static float random_particle_radius() {
	float result = PARTICLE_MIN_START_RADIUS + SDL_floorf(random() * (PARTICLE_MAX_START_RADIUS - PARTICLE_MIN_START_RADIUS));
		
	return result;
}

static rgb_color random_color(rgb_color* colors, Uint32 color_count) {
	rgb_color result = {0};
	if (colors == 0 || color_count == 0) {
		rgb_color default_color = {
			.r = 255,
			.g = 255,
			.b = 255,
		};
		color_count = 1;
		colors = &default_color;
	}

	result = colors[ (Uint32)(random() * (color_count-1)) ]; 
	return result;
}

void randomize_particle(Particle* p, rgb_color* colors, Uint32 color_count) {
	float angle = random() * 360.0f;
	p->collision_radius = random_particle_radius();
	p->color = random_color(colors, color_count); 
	p->vx = cos_deg(angle) * (float)PARTICLE_SPEED;
	p->vy = sin_deg(angle) * (float)PARTICLE_SPEED;
}

void explode_at_point(Particle_System* ps, float x, float y, float force, rgb_color* colors, Uint32 num_colors, Game_Sprite* sprite, Particle_Shape shape) {
//	if (force != 0) {force_circle(x, x, 120, force); }
	for (int p = 0; p < EXPLOSION_STARTING_PARTICLES; p++) {
		Particle* p = instantiate_particle(ps, sprite, shape);
		randomize_particle(p, colors, num_colors);
		p->x = x;
		p->y = y;
	}
}

Particle_Emitter* get_particle_emitter(Particle_System* ps) {
	Particle_Emitter* result = 0;
	if (ps->emitter_count < array_length(ps->emitters)) {
		result = ps->emitters + ps->emitter_count++;
	} else {
		SDL_Log("get_particle_emitter(): max Particle emitter maximum reached");
	}

	return result;
}

void update_particle_emitters(Particle_System* ps, float dt) {
	for (int emitter_index = 0; emitter_index < ps->emitter_count; emitter_index++) {
		Particle_Emitter* emitter = ps->emitters + emitter_index;
		if (!emitter->active) continue;
		
		emitter->counter += emitter->density * dt;

		if (emitter->counter >= 1.0f) {
			float rel_x = emitter->x;
			float rel_y = emitter->y;
			float rel_angle = emitter->angle;
			if (emitter->parent) {
				rel_x += emitter->parent->x;
				rel_y += emitter->parent->y;
				rel_angle += emitter->parent->angle;
			}

			int particles_to_emit = (int)emitter->counter;
			while (particles_to_emit) {
				Particle* p = instantiate_particle(ps, 0, 0);
				if (p) {
					randomize_particle(p, emitter->colors, emitter->color_count);
					//p->collision_radius *= emitter->scale;

					p->x = rel_x;
					p->y = rel_y;

					p->vx = cos_deg(rel_angle) * PARTICLE_SPEED;
					p->vy = sin_deg(rel_angle) * PARTICLE_SPEED;
				}
				particles_to_emit--;
			}

			emitter->counter -= (int)emitter->counter;
		}
	}
}

void explode_sprite(Particle_System* ps, Game_Sprite* sprite, float x, float y, float angle, int pieces) {
	float angle_division = 360.0f / (float)pieces;
	float random_deviation = 0;//angle_division * 1.8;

	Game_Sprite* chunks = divide_sprite(sprite, pieces);

	float radius = (chunks[0].rect.w + chunks[0].rect.h) / 2;
	int cHalf = pieces / 2;

	// Create explosion using chunks as particle sprites
	for (int chunk_index = 0; chunk_index < pieces; chunk_index++) {
		Particle* particle = instantiate_particle(ps, chunks + chunk_index, 0);
		const float random_deviation = 30.0f;

		float chunk_angle = angle + 180.0f + (chunk_index * angle_division);
			  chunk_angle += random_deviation/2.0f - (random() * random_deviation);
		float vx = cos_deg(chunk_angle) * (float)PARTICLE_SPEED;
		float vy = sin_deg(chunk_angle) * (float)PARTICLE_SPEED;

		randomize_particle(particle, 0, 0);
		particle->collision_radius = radius;
		particle->life_left = PARTICLE_LIFETIME;
		particle->angle = angle;
		particle->x = x;
		particle->y = y;
		particle->vx = vx;
		particle->vy = vy;
	}

	SDL_free(chunks);
}
