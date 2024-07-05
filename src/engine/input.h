#ifndef GAME_INPUT_H
#define GAME_INPUT_H

#include "SDL_scancode.h"
#include "SDL_gamecontroller.h"
#include "SDL_events.h"

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

typedef struct Game_Input {
	Game_Input_State keys[SDL_NUM_SCANCODES];
	Game_Controller controller;
} Game_Input;

typedef struct Game_Control {
	union { int key; int axis_key_minus; };
	int axis_key_plus;

	union { int button; int axis_button_minus; }; // controller button
	int axis_button_plus;

	int axis_id;
} Game_Control;

#define valid_scancode(key) 	 	(key > SDL_SCANCODE_UNKNOWN && key < SDL_NUM_SCANCODES)
#define valid_controller_button(button) (button > SDL_CONTROLLER_BUTTON_INVALID && button < SDL_CONTROLLER_BUTTON_MAX)
#define valid_controller_axis(axis)   	(axis > SDL_CONTROLLER_AXIS_INVALID && axis < SDL_CONTROLLER_AXIS_MAX)

void poll_input				(Game_Input* input);

void process_key_event			(Game_Input* input, SDL_KeyboardEvent* event);
SDL_bool is_key_pressed			(Game_Input* input, SDL_Scancode key);
SDL_bool is_key_held			(Game_Input* input, SDL_Scancode key);
SDL_bool is_key_released		(Game_Input* input,  SDL_Scancode key);

void process_controller_event		(Game_Input* input, SDL_Event* event);
SDL_bool is_controller_button_pressed	(Game_Input* input, SDL_GameControllerButton button);
SDL_bool is_controller_button_held	(Game_Input* input, SDL_GameControllerButton button);
SDL_bool is_controller_button_released	(Game_Input* input, SDL_GameControllerButton button);

SDL_bool is_game_control_pressed	(Game_Input* input, Game_Control* control);
SDL_bool is_game_control_held		(Game_Input* input, Game_Control* control);
SDL_bool is_game_control_released	(Game_Input* input, Game_Control* control);
float get_game_control_axis		(Game_Input* input, Game_Control* control);

#endif
