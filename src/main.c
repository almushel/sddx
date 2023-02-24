#include "SDL2/SDL.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define MATH_PI 3.14159265
#define RAD_TO_DEG(rads) rads * (180/MATH_PI)
#define DEG_TO_RAD(degs) degs * (MATH_PI/180)

typedef struct rgb_color {uint8_t r, g, b;} rgb_color;
#define CLEAR_COLOR (rgb_color){0,10,48}

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define array_length(array) ( sizeof(array) / sizeof(array[0]) )

#define TARGET_FRAME_TIME 16.6667 // Target frame time in ms

#define PHYSICS_FRICTION 0.02f
#define PLAYER_FORWARD_THRUST 0.15f
#define PLAYER_LATERAL_THRUST 0.2f
#define PLAYER_TURN_SPEED 3.14f

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
		game_button_state thrust;
		game_button_state turn_left;
		game_button_state turn_right;
		game_button_state thrust_left;
		game_button_state thrust_right;
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
		SDL_Log("stbi failed to load %s", file);
	}

	if (image_surface) {
		result = SDL_CreateTextureFromSurface(renderer, image_surface);
	} else {
		SDL_Log("SDL failed to create surface from loaded image");
	}
	
	if (!result) {
		SDL_Log("SDL failed to create texture from surface");
	}

	if (image_surface) SDL_FreeSurface(image_surface);
	if (image) stbi_image_free(image);

	return result;
}

void draw_texture(SDL_Renderer* renderer, SDL_Texture* texture, float x, float y, double angle, SDL_bool centered) {
	if (texture) {
		int dest_w, dest_h;

		SDL_QueryTexture(texture, NULL, NULL, &dest_w, &dest_h);

		SDL_FRect dest_rect;
		dest_rect.x = x;
		dest_rect.y = y;
		dest_rect.w = (float)dest_w;
		dest_rect.h = (float)dest_h;

		if (centered == SDL_TRUE) {
			dest_rect.x -= dest_rect.w/2.0f;
			dest_rect.y -= dest_rect.h/2.0f;
		}

		SDL_RenderCopyExF(renderer, texture, NULL, &dest_rect, angle, 0, SDL_FLIP_NONE);
	}
}

void process_key_event(SDL_KeyboardEvent* event, game_controller_state* input) {
	if (event->repeat == 0) {
		switch (event->keysym.scancode) {
			case SDL_SCANCODE_ESCAPE: {
				SDL_Event quit_event;
				quit_event.type = SDL_QUIT;
				SDL_PushEvent(&quit_event);
			} return;
		}

		game_button_state* button;
		for (int i = 0; i < array_length(input->list); i++) {
			button = input->list + i;
			if (button->scan_code == event->keysym.scancode) {
				button->pressed = event->state == SDL_PRESSED;
				button->released = event->state == SDL_RELEASED;
			}
		}
	}
}

int main(int argc, char* argv[]) {

	SDL_Init(SDL_INIT_EVERYTHING);

	// TO-DO: Figure out why this stops responding when moved to a different monitor
	SDL_Window* window = SDL_CreateWindow("Space Drifter DX", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	if (window == NULL) {
		SDL_LogError(0, SDL_GetError());
		exit(1);
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		SDL_LogError(0, SDL_GetError());
		exit(1);
	}
	SDL_RenderSetVSync(renderer, -1);

	game_controller_state player_controller = {0};

	player_controller.thrust.scan_code = SDL_SCANCODE_W;
	player_controller.thrust_left.scan_code = SDL_SCANCODE_E;
	player_controller.thrust_right.scan_code = SDL_SCANCODE_Q;
	player_controller.turn_left.scan_code = SDL_SCANCODE_A;
	player_controller.turn_right.scan_code = SDL_SCANCODE_D;
	player_controller.fire.scan_code = SDL_SCANCODE_SPACE;

	game_controller_state new_player_controller = {0};

	SDL_Texture* player_ship = load_texture(renderer, "assets/images/player.png");

	struct {
		float x, y, dx, dy;
		double angle;
	} player = {0};

	player.x = SCREEN_WIDTH / 2;
	player.y = SCREEN_HEIGHT / 2;
	player.angle = 270;

	Uint64 last_count = SDL_GetPerformanceCounter();
	double delay_remainder = 0;

	SDL_bool running = SDL_TRUE;
	while(running) {
		Uint64 current_count = SDL_GetPerformanceCounter();
		double dt = (double)(current_count - last_count)*1000 / (double)SDL_GetPerformanceFrequency();
		dt /= TARGET_FRAME_TIME;
		last_count = current_count;
//		SDL_Log("Frame Time: %f", dt);

		new_player_controller = (game_controller_state) {
			.thrust.scan_code 		= 	player_controller.thrust.scan_code,
			.thrust_left.scan_code 	= 	player_controller.thrust_left.scan_code,
			.thrust_right.scan_code = 	player_controller.thrust_right.scan_code,
			.turn_left.scan_code 	= 	player_controller.turn_left.scan_code,
			.turn_right.scan_code 	= 	player_controller.turn_right.scan_code,
			.fire.scan_code 		= 	player_controller.fire.scan_code,
		};
	
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYUP: {
					process_key_event(&event.key, &new_player_controller);
				} break;
				case SDL_KEYDOWN: {
					process_key_event(&event.key, &new_player_controller);
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

		if (player_controller.turn_left.held) player.angle -= PLAYER_TURN_SPEED * dt;
		if (player_controller.turn_right.held) player.angle += PLAYER_TURN_SPEED * dt;

		if (player_controller.thrust.held) {
			player.dx += cos_deg(player.angle) * PLAYER_FORWARD_THRUST * dt;
			player.dy += sin_deg(player.angle) * PLAYER_FORWARD_THRUST * dt;
		}

		if (player_controller.thrust_left.held) {
			player.dx += cos_deg(player.angle + 90) * PLAYER_LATERAL_THRUST * dt;
			player.dy += sin_deg(player.angle + 90) * PLAYER_LATERAL_THRUST * dt;
		}
		if (player_controller.thrust_right.held) {
			player.dx += cos_deg(player.angle - 90) * PLAYER_LATERAL_THRUST * dt;
			player.dy += sin_deg(player.angle - 90) * PLAYER_LATERAL_THRUST * dt;
		}

		player.x += player.dx * dt;
		player.y += player.dy * dt;

		player.dx *= (1.0 - PHYSICS_FRICTION);
		player.dy *= (1.0 - PHYSICS_FRICTION);

		if (player.x < 0) player.x = SCREEN_WIDTH + player.x;
		else if (player.x > SCREEN_WIDTH) player.x -= SCREEN_WIDTH;

		if (player.y < 0) player.y = SCREEN_HEIGHT + player.y;
		else if (player.y > SCREEN_HEIGHT) player.y -= SCREEN_HEIGHT;

		SDL_SetRenderDrawColor(renderer,CLEAR_COLOR.r,CLEAR_COLOR.g,CLEAR_COLOR.b,255);	
		SDL_RenderClear(renderer);
		draw_texture(renderer, player_ship, (int)player.x, (int)player.y, player.angle, SDL_TRUE);
//		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
//		SDL_Rect center_rect = {player.x-5, player.y-5,10, 10};
//		SDL_RenderFillRect(renderer, &center_rect);
		SDL_RenderPresent(renderer);
		
		// Calculate the time_elapsed (the time taken to update and render the current frame)
		// Subtract from target frame time
		// Add remainder from previous frames (because SDL_Delay only excepts integer MS values)
		// Calculate new remainder from this sum

		double time_elapsed = (double)(SDL_GetPerformanceCounter() - current_count) / (double)SDL_GetPerformanceFrequency() * 1000;
		double delay = TARGET_FRAME_TIME - time_elapsed;
		if (delay_remainder >= 1.0) {
			delay += delay_remainder;
		}
		delay_remainder = (double)(delay - (Uint32)delay);
		
		SDL_Delay((Uint32)delay);
	}

	SDL_Quit();	

	return 0;
}