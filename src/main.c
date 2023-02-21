#include <stdio.h>
#include "SDL2/SDL.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

int main(int argc, char* argv[]) {

	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* window;
	SDL_Renderer* renderer;

	SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer);

	SDL_Rect dest_rect;
	dest_rect.x = 640;
	dest_rect.y = 360;

	int image_width, image_height, image_components;

	unsigned char* image = stbi_load("assets/images/player.png", &image_width, &image_height, &image_components, 0);

	SDL_Surface* image_surface;
	if (image) {
		Uint32 r_mask = 0x000000FF; 
		Uint32 g_mask = 0x0000FF00;
		Uint32 b_mask = 0x00FF0000;
		Uint32 a_mask = 0xFF000000;

		image_surface = SDL_CreateRGBSurfaceFrom(image, image_width, image_height, image_components * 8, image_components * image_width, 
							r_mask, g_mask, b_mask, a_mask);
	} else {
		printf("stbi failed to load player.png");
	}

	SDL_Texture* image_texture;
	if (image_surface) {
		image_texture = SDL_CreateTextureFromSurface(renderer, image_surface);
		dest_rect.w = image_width;
		dest_rect.h = image_height;
	} else {
		printf("SDL failed to create surface from loaded player image");
	}

	if (!image_texture) {
		printf("SDL failed to create texture from player image surface");
	}

	SDL_bool running = SDL_TRUE;
	while(running) {
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYUP: {
					switch(event.key.keysym.sym) {
						case SDLK_ESCAPE: {
							SDL_Event quit_event;
							quit_event.type = SDL_QUIT;
							SDL_PushEvent(&quit_event);
						}
					}
				} break;
				case SDL_QUIT: {
					running = SDL_FALSE;
				} break;
			}
		}

		SDL_SetRenderDrawColor(renderer,0,0,0,255);	
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 255, 255, 100, 255);
		if (image_texture) {
			double angle = 270;
			SDL_Point* center = 0;
			
			SDL_RenderCopyEx(renderer, image_texture, NULL, &dest_rect, angle, center, SDL_FLIP_NONE);
		}
		//SDL_RenderDrawRect(renderer, &rect);
		SDL_RenderPresent(renderer);
	}

	SDL_Quit();	

	return 0;
}