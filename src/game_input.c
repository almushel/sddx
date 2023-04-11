#include "game_input.h"

void process_key_event(Game_Input* input, SDL_KeyboardEvent* event) {
	if (event->repeat == 0) {
		switch (event->state) {
			case SDL_PRESSED: {
				input->keys[event->keysym.scancode] = GAME_INPUT_PRESSED;
			} break;

			case SDL_RELEASED: {
				input->keys[event->keysym.scancode] = GAME_INPUT_RELEASED;
			} break;
		}
	}
}

void poll_input(Game_Input* input) {
	for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
		switch (input->keys[i]) {
			case GAME_INPUT_PRESSED: {
				input->keys[i] = GAME_INPUT_HELD;
			} break;

			case GAME_INPUT_RELEASED: {
				input->keys[i] = GAME_INPUT_NULL;
			} break;
		}
	}

	for (int c = 0; c < GAME_MAX_CONTROLLERS; c++) {
		for (int b = 0; b < SDL_CONTROLLER_BUTTON_MAX; b++) {
			switch(input->controllers[c].buttons[b]) {
				case GAME_INPUT_PRESSED: {
					input->controllers[c].buttons[b] = GAME_INPUT_HELD;
				} break;

				case GAME_INPUT_RELEASED: {
					input->controllers[c].buttons[b] = GAME_INPUT_NULL;
				} break;
			}
		}
	}
}

bool is_key_pressed(Game_Input* input, SDL_Scancode key) {
	bool result = false;

	result = (input->keys[key] == GAME_INPUT_PRESSED);

	return result;
}

bool is_key_held(Game_Input* input, SDL_Scancode key) {
	bool result = false;

	result = (input->keys[key] == GAME_INPUT_PRESSED) || (input->keys[key] == GAME_INPUT_HELD);

	return result;
}

bool is_key_released(Game_Input* input,  SDL_Scancode key) {
	bool result = false;

	result = (input->keys[key] == GAME_INPUT_RELEASED);

	return result;
}

void process_controller_button_event(Game_Input* input, SDL_ControllerButtonEvent* event) {
	// NOTE: Unique id is created every time a controller is connected and they are never reused.
	// Static array of controllers is probably not idea here.
	SDL_JoystickID id = event->which;
	switch(event->type) {
		case SDL_CONTROLLERBUTTONDOWN: 	{
			input->controllers[id].buttons[event->button] = GAME_INPUT_PRESSED;
		} break;
		
		case SDL_CONTROLLERBUTTONUP:	{
			input->controllers[id].buttons[event->button] = GAME_INPUT_RELEASED;
		} break;
	}
}

void process_controller_axis_event(Game_Input* input, SDL_ControllerAxisEvent* event) {
	SDL_JoystickID id = event->which;
	if (id < GAME_MAX_CONTROLLERS) {
		// Convert axis value to floating point value between -1 and 1
		input->controllers[id].axes[event->axis] = 
			(event->value >= 0) 				? 
			(float)(event->value / INT16_MAX) 	:
			(float)(event->value / INT16_MIN)	;
	}
}

bool is_controller_button_pressed(Game_Input* input, SDL_GameControllerButton button) {
	bool result = false;

	result = (input->controllers[0].buttons[button] == GAME_INPUT_PRESSED);

	return result;
}

bool is_controller_button_held(Game_Input* input, SDL_GameControllerButton button) {
	bool result = false;

	result = (input->controllers[0].buttons[button] == GAME_INPUT_PRESSED) || (input->controllers[0].buttons[button] == GAME_INPUT_HELD);

	return result;
}

bool is_controller_button_released(Game_Input* input, SDL_GameControllerButton button) {
	bool result = false;

	result = (input->controllers[0].buttons[button] == GAME_INPUT_RELEASED);

	return result;
}