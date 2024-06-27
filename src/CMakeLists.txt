cmake_minimum_required(VERSION 3.22.1)

project(sddx C)
add_executable(sddx
	main.c
	engine/platform.c
	game/game.c 
)
if (DEBUG)
	target_compile_definitions(sddx PRIVATE DEBUG)
endif()

if (WIN32)
	target_link_libraries(sddx SDL2 SDL2main SDL2_mixer winmm version Imm32 Setupapi)
else()
	target_include_directories(sddx 
		PRIVATE /usr/include/SDL2/
	)
	target_link_libraries(sddx SDL2d SDL2maind SDL2_mixerd)
endif()

