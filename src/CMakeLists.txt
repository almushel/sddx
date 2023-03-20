cmake_minimum_required(VERSION 3.22.2)

set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/../../external")
message(${EXTERNAL_DIR})

project(sddx C)
add_executable(sddx main.c game_math.c)

target_link_directories(sddx 
	PRIVATE "${EXTERNAL_DIR}/bin/SDL_mixer/"
	PRIVATE "${EXTERNAL_DIR}/bin/SDL2/"
)
target_link_libraries(sddx SDL2-static SDL2main SDL2_mixer-static winmm version Imm32 Setupapi)

target_include_directories(sddx 
	PRIVATE "${EXTERNAL_DIR}/include/"
)