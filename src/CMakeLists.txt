cmake_minimum_required(VERSION 3.22.2)

set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/../../external")
message(${EXTERNAL_DIR})

project(sddx C)
add_executable(sddx main.c game.c assets.c game_math.c)

target_link_directories(sddx 
	PRIVATE "${EXTERNAL_DIR}/bin/SDL_mixer/"
	PRIVATE "${EXTERNAL_DIR}/bin/SDL2/"
)
target_link_libraries(sddx SDL2-staticd SDL2maind SDL2_mixer-staticd winmm version Imm32 Setupapi)

target_include_directories(sddx 
	PRIVATE "${EXTERNAL_DIR}/include/"
)