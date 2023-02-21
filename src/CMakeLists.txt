cmake_minimum_required(VERSION 3.22.2)

project(SPACE_DRIFTER_DX C)

set(SDL-DIR "w:/external/SDL2/")

add_executable(sddx main.c)

target_include_directories(sddx PUBLIC "w:/external/include")
target_link_directories(sddx PUBLIC "${SDL-DIR}/bin/")
target_link_libraries(sddx SDL2d.lib SDL2maind.lib)

