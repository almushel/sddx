#include "SDL.h"
#include "engine/platform.h"
#include "game/game.h"

int main(int argc, char* argv[]) {
	Platform_State platform = {
		.title = "Space Drifter DX",
		.screen = {1280, 700},
		.world = {800, 600},
		.target_fps = (double)TARGET_FPS,
		.target_frame_time = 1000.0/(double)TARGET_FPS,
	};
	Game_State* game = SDL_calloc(1, sizeof(Game_State));
	Game_Input input = {0};

	platform_init(&platform);
	init_game(game);

	platform.current_count = platform.last_count = SDL_GetPerformanceCounter();

	SDL_bool running;
	while ( (running = platform_update_and_render(&platform, game, &input)) );

	SDL_Quit();	

	return 0;
}
