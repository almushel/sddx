#include "types.h"

float lerp(float start, float end, float t) {
	t = SDL_clamp(t, 0.0f, 1.0f);

	return (1.0f - t) * start + (t * end);
}

float smooth_start(float t, int magnitude) {
	t = SDL_clamp(t, 0.0f, 1.0f);
	float result = t;
	for (int i = 0; i < magnitude; i++) {
		result *= t;
	}

	return result;
}

float smooth_stop(float t, int magnitude) {
	t = SDL_clamp(t, 0.0f, 1.0f);
	float result = 1.0f-t;
	for (int i = 0; i < magnitude; i++) {
		result *= (1.0f-t);
	}

	return 1.0f-result;
}

void lerp_timer_start(Lerp_Timer* lt, float min, float max, float dir) {
	*lt = (Lerp_Timer) {
		.min = min,
		.max = max,
		.dir = dir
	};
	
	lt->time = lt->dir > 0 ? min : max;
}


void lerp_timer_update(Lerp_Timer* lt, float dt) {
	lt->time += (lt->dir * dt);
}

void lerp_timer_set_t(Lerp_Timer* lt, float t) {
	lt->time = lerp(lt->min, lt->max, SDL_clamp(t, 0.0f, 1.0f));
}

// Convert lerp timer to float value between 0.0 and 1.0
float lerp_timer_get_t(Lerp_Timer* lt) {
	return SDL_clamp( 
		(lt->time - lt->min) / (lt->max-lt->min),
		0.0f, 1.0f
	);
}
