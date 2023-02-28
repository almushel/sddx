#include "SDL2/SDL.h"
#include "stdlib.h"
#include "defs.h"

#define PARTICLE_LIFETIME 15
#define PARTICLE_MAX_START_RADIUS 7
#define PARTICLE_MIN_START_RADIUS 3
#define PARTICLE_SPEED 6
#define PARTICLE_DECAY 0.75f
#define EXPLOSION_STARTING_PARTICLES 12
#define MAX_PARTICLES 512
#define DEFAULT_PARTICLE_COLOR (rgb_color){255, 255, 255}

typedef enum Particle_Shape {
	PARTICLE_SHAPE_UNDEFINED,
	PARTICLE_SHAPE_RECT,
	PARTICLE_SHAPE_COUNT,
} Particle_Shape;

typedef struct Particle {
	float x, y;
	float vx, vy;
	float mass;
	float collision_radius;
	float life_left;
	Uint32 sprite;	

	Particle_Shape shape;
	rgb_color color;
} Particle;

static Particle particles[MAX_PARTICLES];
static Uint32 particle_count = 0;

// Returns a pseudo-random value between 0 and 1
float random() {
	static SDL_bool seeded = 0;
	if (!seeded) {
		srand(42);
		seeded = 1;
	}

	float result = (float)(rand() % 1000) / 1000.0f;

	return result;
}

void update_particles(float dt) {
	Uint32* dead_particles = SDL_malloc(sizeof(Uint32) * particle_count);
	SDL_memset(dead_particles, 0, sizeof(Uint32) * particle_count);
	Uint32 dead_particle_count = 0;

	for (int p = 0; p < particle_count; p++) {
		Particle* particle = particles + p;
		
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
		if (dead_particles[d] != particle_count-1) {
			*(particles + dead_particles[d]) = *(particles + (particle_count-1));
		}
		particle_count--;
	}

	SDL_free(dead_particles);
}

void draw_particles(SDL_Renderer* renderer) {
	for (int p = 0; p < particle_count; p++) {
		Particle particle = particles[p];
		if (particle.sprite) {
			// draw sprite
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

Particle* get_particle(void) {
	Particle* result = 0;
	if (particle_count < MAX_PARTICLES) {
		result = particles + particle_count++;
		if (particle_count >= MAX_PARTICLES) particle_count = 0;
	} else {
		SDL_Log("get_particle(): max Particle count reached");
	}

	return result;
}

Particle* instantiate_particle(Uint32 sprite, Particle_Shape shape) {
	Particle* result = get_particle();
	
	init_particle(result);
	result->sprite = sprite;
	result->shape = shape;

	return result;
}
	
static float random_particle_radius() {
	float result = PARTICLE_MIN_START_RADIUS + SDL_floorf(random() * (PARTICLE_MAX_START_RADIUS - PARTICLE_MIN_START_RADIUS));
		
	return result;
}

void randomize_particle(Particle* p, rgb_color* colors, Uint32 num_colors) {
	if (colors == 0) {
		rgb_color default_color = {
			.r = 255,
			.g = 255,
			.b = 255,
		};
		num_colors = 1;
		colors = &default_color;
	}
	
	float angle = random() * 360.0f;
	p->collision_radius = random_particle_radius();
	p->color = colors[ (Uint32)(random() * (num_colors-1)) ]; 
	p->vx = cos_deg(angle) * (float)PARTICLE_SPEED;
	p->vy = sin_deg(angle) * (float)PARTICLE_SPEED;
}

void explode_at_point(float x, float y, float force, rgb_color* colors, Uint32 num_colors, Uint32 sprite, Particle_Shape shape) {
//	if (force != 0) {force_circle(x, x, 120, force); }
	for (int p = 0; p < EXPLOSION_STARTING_PARTICLES; p++) {
		Particle* p = instantiate_particle(sprite, shape);
		randomize_particle(p, colors, num_colors);
		p->x = x;
		p->y = y;
	}
}

// TO-DO: Particle emitter

// TO-DO: Sprite explosion