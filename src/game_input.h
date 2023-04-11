#ifndef GAME_INPUT_H
#define GAME_INPUT_H

#include <stdbool.h>
#include "SDL2/SDL_scancode.h"
#include "SDL2/SDL_gamecontroller.h"
#include "SDL2/SDL_events.h"

typedef enum Game_Input_State {
	GAME_INPUT_NULL,
	GAME_INPUT_PRESSED,
	GAME_INPUT_HELD,
	GAME_INPUT_RELEASED,
} Game_Input_State;

typedef struct Game_Controller {
	Game_Input_State buttons[SDL_CONTROLLER_BUTTON_MAX];
	float axes[SDL_CONTROLLER_AXIS_MAX];
} Game_Controller;

typedef union Game_Player_Controller {
	struct {
		SDL_Scancode thrust;
		SDL_Scancode turn_left;
		SDL_Scancode turn_right;
		SDL_Scancode thrust_left;
		SDL_Scancode thrust_right;
		SDL_Scancode fire;
	};
	SDL_Scancode list[6];
} Game_Player_Controller;

#define GAME_MAX_CONTROLLERS 4
typedef struct Game_Input {
	Game_Input_State keys[SDL_NUM_SCANCODES];
	Game_Controller controllers[GAME_MAX_CONTROLLERS];
} Game_Input;

void poll_input(Game_Input* input);

void process_key_event(Game_Input* input, SDL_KeyboardEvent* event);
bool is_key_pressed(Game_Input* input, SDL_Scancode key);
bool is_key_held(Game_Input* input, SDL_Scancode key);
bool is_key_released(Game_Input* input,  SDL_Scancode key);

void process_controller_button_event(Game_Input* input, SDL_ControllerButtonEvent* event);
void process_controller_axis_event(Game_Input* input, SDL_ControllerAxisEvent* event);
bool is_controller_button_pressed(Game_Input* input, SDL_GameControllerButton button);
bool is_controller_button_held(Game_Input* input, SDL_GameControllerButton button);
bool is_controller_button_released(Game_Input* input, SDL_GameControllerButton button);

#endif