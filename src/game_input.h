#ifndef GAME_INPUT_H
#define GAME_INPUT_H

#include <stdbool.h>
#include "SDL2/SDL_scancode.h"
#include "SDL2/SDL_events.h"

typedef enum Game_Input_State {
	GAME_INPUT_NULL,
	GAME_INPUT_PRESSED,
	GAME_INPUT_HELD,
	GAME_INPUT_RELEASED,
} Game_Input_State;

typedef union Game_Controller {
	struct {
		SDL_Scancode thrust;
		SDL_Scancode turn_left;
		SDL_Scancode turn_right;
		SDL_Scancode thrust_left;
		SDL_Scancode thrust_right;
		SDL_Scancode fire;
	};
	SDL_Scancode list[6];
} Game_Controller;

typedef struct Game_Input {
	Game_Input_State keys[SDL_NUM_SCANCODES];
} Game_Input;

void process_key_event(SDL_KeyboardEvent* event, Game_Input* input);
void poll_input(Game_Input* input);

bool is_key_pressed(Game_Input* input, SDL_Scancode key);
bool is_key_held(Game_Input* input, SDL_Scancode key);
bool is_key_released(Game_Input* input,  SDL_Scancode key);

#endif