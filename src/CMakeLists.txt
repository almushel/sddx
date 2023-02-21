cmake_minimum_required(VERSION 3.22.2)

project(sddx C)

set(SDL-DIR "w:/external/SDL2/")

add_executable(sddx main.c)

target_include_directories(sddx PUBLIC "w:/external/include")
target_link_directories(sddx PUBLIC "${SDL-DIR}/bin/")
target_link_libraries(sddx SDL2-staticd SDL2maind winmm version Imm32 Setupapi)