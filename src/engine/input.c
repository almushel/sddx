#include "input.h"

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

			default: {} break;
		}
	}

	for (int b = 0; b < SDL_CONTROLLER_BUTTON_MAX; b++) {
		switch(input->controller.buttons[b]) {
			case GAME_INPUT_PRESSED: {
				input->controller.buttons[b] = GAME_INPUT_HELD;
			} break;

			case GAME_INPUT_RELEASED: {
				input->controller.buttons[b] = GAME_INPUT_NULL;
			} break;

			default: {} break;
		}
	}
}

SDL_bool is_key_pressed(Game_Input* input, SDL_Scancode key) {
	SDL_bool result = ( 
		valid_scancode(key) &&
		input->keys[key] == GAME_INPUT_PRESSED
	);

	return result;
}

SDL_bool is_key_held(Game_Input* input, SDL_Scancode key) {
	SDL_bool result = (
		valid_scancode(key) &&
		input->keys[key] == GAME_INPUT_PRESSED) || (input->keys[key] == GAME_INPUT_HELD
	);

	return result;
}

SDL_bool is_key_released(Game_Input* input,  SDL_Scancode key) {
	SDL_bool result = (
		valid_scancode(key) &&
		input->keys[key] == GAME_INPUT_RELEASED
	);

	return result;
}

// NOTE: Currently all connect controllers will update the same input.controller state, because this is a single-player game

void process_controller_event(Game_Input* input, SDL_Event* event) {
	switch(event->type) {
		case SDL_CONTROLLERDEVICEADDED: {
			int controller_count = SDL_NumJoysticks();
			for (int i = 0; i < controller_count; i++) {
				SDL_Log("Controller connected: %i", i);
				SDL_GameController* controller = SDL_GameControllerOpen(i);
				if (controller == NULL) {
					SDL_Log("%s", SDL_GetError());
				}
			}
		} break;

		case SDL_CONTROLLERDEVICEREMOVED: {
			SDL_Log("Controller disconnected: %i", event->cdevice.which);
			SDL_GameController* controller = SDL_GameControllerFromInstanceID(event->cdevice.which);
			SDL_GameControllerClose(controller);
		} break;
		
		case SDL_CONTROLLERBUTTONDOWN: 	{
			input->controller.buttons[event->cbutton.button] = GAME_INPUT_PRESSED;
		} break;
		
		case SDL_CONTROLLERBUTTONUP:	{
			input->controller.buttons[event->cbutton.button] = GAME_INPUT_RELEASED;
		} break;

		case SDL_CONTROLLERAXISMOTION:	{
			input->controller.axes[event->caxis.axis] = 
				(event->caxis.value >= 0) 				? 
				(float)(event->caxis.value / INT16_MAX) 	:
				(float)(event->caxis.value / INT16_MIN)	;
		}
	}
}

SDL_bool is_controller_button_pressed(Game_Input* input, SDL_GameControllerButton button) {
	SDL_bool result = (
		valid_controller_button(button) &&
		(input->controller.buttons[button] == GAME_INPUT_PRESSED)
	);

	return result;
}

SDL_bool is_controller_button_held(Game_Input* input, SDL_GameControllerButton button) {
	SDL_bool result = (	
		valid_controller_button(button) &&
		(input->controller.buttons[button] == GAME_INPUT_PRESSED) || (input->controller.buttons[button] == GAME_INPUT_HELD)
	);

	return result;
}

SDL_bool is_controller_button_released(Game_Input* input, SDL_GameControllerButton button) {
	SDL_bool result = (	
		valid_controller_button(button) &&
		input->controller.buttons[button] == GAME_INPUT_RELEASED
	);

	return result;
}

float get_controller_axis(Game_Input* input, int axis) {
	float result = 0;

	if (valid_controller_axis(axis)) {
		result = input->controller.axes[axis];
	}

	return result;
}

SDL_bool is_game_control_pressed(Game_Input* input, Game_Control* control){
	SDL_bool result = (
		is_key_pressed(input, control->key) ||
		is_controller_button_pressed(input, control->button)
	);

	return result;
}

SDL_bool is_game_control_held(Game_Input* input, Game_Control* control) {
	SDL_bool result = (
		is_key_held(input, control->key) ||
		is_controller_button_held(input, control->button)
	);

	return result;
}

SDL_bool is_game_control_released(Game_Input* input, Game_Control* control) {
	SDL_bool result = (
		is_key_released(input, control->key) ||
		is_controller_button_released(input, control->button)
	);

	return result;
}

float get_game_control_axis(Game_Input* input, Game_Control* control) {
	float result = 0;

	if (valid_scancode(control->axis_key_minus) && valid_scancode(control->axis_key_plus)) {
		result = (float)( (int)is_key_held(input, control->axis_key_plus) - (int)is_key_held(input, control->axis_key_minus) );
	}
	else if (valid_controller_button(control->axis_button_minus) && valid_controller_button(control->axis_button_plus)) {
		result = (float)( (int)is_controller_button_held(input, control->axis_button_plus) - (int)is_controller_button_held(input, control->axis_button_minus) );
	} 
	else {
		result = get_controller_axis(input, control->axis_id);
	}

	return result;

}
