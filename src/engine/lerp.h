#ifndef LERP_H
#define LERP_H

#include "types.h"

float lerp			(float start, float end, float t);
float smooth_start		(float t, int magnitude);
float smooth_stop		(float t, int magnitude);

void lerp_timer_start		(Lerp_Timer* lt, float min, float max, float dir);
void lerp_timer_update		(Lerp_Timer* lt, float dt);
void lerp_timer_set_t		(Lerp_Timer* lt, float t);
float lerp_timer_get_t		(Lerp_Timer* lt);

#endif
