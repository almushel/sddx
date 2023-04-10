#include "game_input.h"

void process_key_event(SDL_KeyboardEvent* event, Game_Input* input) {
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