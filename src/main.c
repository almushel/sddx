#include <stdio.h>
#include "SDL2/SDL.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define MATH_PI 3.14159265
#define RAD_TO_DEG(rads) rads * (180/MATH_PI)
#define DEG_TO_RAD(degs) degs * (MATH_PI/180)

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define array_length(array) ( sizeof(array) / sizeof(array[0]) )

enum stbi_masks {
	STBI_MASK_R = 0x000000FF, 
	STBI_MASK_G = 0x0000FF00,
	STBI_MASK_B = 0x00FF0000,
	STBI_MASK_A = 0xFF000000,
} stbi_masks;

typedef struct game_button_state {
	SDL_Scancode scan_code;
	SDL_bool pressed, held, released;
} game_button_state;

typedef union game_controller_state {
	struct {
		game_button_state up;
		game_button_state down;
		game_button_state left;
		game_button_state right;
		game_button_state fire;
	};
	game_button_state list[5];
} game_controller_state;

static inline double sin_deg(double degrees) { return SDL_sin(DEG_TO_RAD(degrees)); }
static inline double cos_deg(double degrees) { return SDL_cos(DEG_TO_RAD(degrees)); }

SDL_Texture* load_texture(SDL_Renderer* renderer, const char* file) {
int image_width, image_height, image_components;
	SDL_Texture* result = 0;

	unsigned char* image = stbi_load(file, &image_width, &image_height, &image_components, 0);

	SDL_Surface* image_surface;
	if (image) {
		image_surface = SDL_CreateRGBSurfaceFrom(image, image_width, image_height, image_components * 8, image_components * image_width, 
							STBI_MASK_R, STBI_MASK_G, STBI_MASK_B, STBI_MASK_A);
	} else {
		printf("stbi failed to load %s", file);
	}

	if (image_surface) {
		result = SDL_CreateTextureFromSurface(renderer, image_surface);
	} else {
		printf("SDL failed to create surface from loaded image");
	}
	
	if (!result) {
		printf("SDL failed to create texture from surface");
	}

	if (image_surface) SDL_FreeSurface(image_surface);
	if (image) stbi_image_free(image);

	return result;
}

void draw_texture(SDL_Renderer* renderer, SDL_Texture* texture, int x, int y, double angle, SDL_bool centered) {
	if (texture) {
		SDL_Rect dest_rect;
		dest_rect.x = x;
		dest_rect.y = y;

		SDL_QueryTexture(texture, NULL, NULL, &dest_rect.w, &dest_rect.h);

		if (centered == SDL_TRUE) {
			dest_rect.x -= dest_rect.w/2;
			dest_rect.y -= dest_rect.h/2;
		}

		SDL_RenderCopyEx(renderer, texture, NULL, &dest_rect, angle, 0, SDL_FLIP_NONE);
	}
}

void process_key_event(SDL_Event* event, game_controller_state* input) {
	switch (event->key.keysym.scancode) {
		case SDL_SCANCODE_ESCAPE: {
			SDL_Event quit_event;
			quit_event.type = SDL_QUIT;
			SDL_PushEvent(&quit_event);
		} return;
	}

	game_button_state* button;
	for (int i = 0; i < array_length(input->list); i++) {
		button = input->list + i;
		if (button->scan_code == event->key.keysym.scancode) {
			button->pressed = event->key.state == SDL_PRESSED;
			button->released = event->key.state == SDL_RELEASED;
		}
	}
}

int main(int argc, char* argv[]) {

	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* window;
	SDL_Renderer* renderer;

	SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);

	game_controller_state player_controller = {0};

	player_controller.up.scan_code = SDL_SCANCODE_W;
	player_controller.down.scan_code = SDL_SCANCODE_S;
	player_controller.left.scan_code = SDL_SCANCODE_A;
	player_controller.right.scan_code = SDL_SCANCODE_D;
	player_controller.fire.scan_code = SDL_SCANCODE_SPACE;

	game_controller_state new_player_controller = {0};

	SDL_Texture* player_ship = load_texture(renderer, "assets/images/player.png");

	struct {
		float x, y;
		double angle;
	} player;

	player.x = SCREEN_WIDTH / 2;
	player.y = SCREEN_HEIGHT / 2;
	player.angle = 270;

	SDL_bool running = SDL_TRUE;
	while(running) {
		new_player_controller = (game_controller_state) {
			.up.scan_code = player_controller.up.scan_code,
			.down.scan_code = player_controller.down.scan_code,
			.left.scan_code = player_controller.left.scan_code,
			.right.scan_code = player_controller.right.scan_code,
			.fire.scan_code = player_controller.fire.scan_code,
		};
	
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYUP: {
					process_key_event(&event, &new_player_controller);
				} break;
				case SDL_KEYDOWN: {
					process_key_event(&event, &new_player_controller);
				} break;

				case SDL_QUIT: {
					running = SDL_FALSE;
				} break;
			}
		}

		game_button_state* current_button,* new_button;
		for (int i = 0; i < array_length(new_player_controller.list); i++) {
			new_button = new_player_controller.list + i;
			current_button = player_controller.list + i;

			current_button->pressed = (!current_button->held && new_button->pressed);
			current_button->released = (current_button->held && new_button->released);

			if (new_button->pressed) current_button->held = SDL_TRUE;
			else if (new_button->released) current_button->held = SDL_FALSE;
		}

		float dt = 0.01;
		if (player_controller.left.held) player.angle -= 1 * dt;
		if (player_controller.right.held) player.angle += 1 * dt;

		if (player_controller.up.held) {
			player.x += cos_deg(player.angle) * dt;
			player.y += sin_deg(player.angle) * dt;
		}

		if (player.x < 0) player.x = SCREEN_WIDTH + player.x;
		else if (player.x > SCREEN_WIDTH) player.x -= SCREEN_WIDTH;

		if (player.y < 0) player.y = SCREEN_HEIGHT + player.y;
		else if (player.y > SCREEN_HEIGHT) player.y -= SCREEN_HEIGHT;

		SDL_SetRenderDrawColor(renderer,0,0,0,255);	
		SDL_RenderClear(renderer);
		draw_texture(renderer, player_ship, (int)player.x, (int)player.y, player.angle, SDL_TRUE);
//		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
//		SDL_Rect center_rect = {player.x-5, player.y-5,10, 10};
//		SDL_RenderFillRect(renderer, &center_rect);
		SDL_RenderPresent(renderer);
	}

	SDL_Quit();	

	return 0;
}