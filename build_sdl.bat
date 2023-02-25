REM This script takes the source directories of the SDL and SDL_Mixer libraries and builds them in sequentially,
REM because SDL_mixer requires built SDL2 static libraries at configuration time.
REM Example programs will not compile with SDL_mixer-static
REM Assumes CMake is outputting Visual Studio project files

@echo off
set sdl_dir="../SDL-release-2.26.3"
set sdlmixer_dir="../SDL_mixer-release-2.6.3"
set sdl_lib=%sdl_dir%/bin/Release/SDL2.lib
set sdl_main=%sdl_dir%/bin/Release/SDL2main.lib

pushd %sdl_dir%
cmake -S ./ -B bin

pushd bin
msbuild SDL2.sln -p:Configuration=Release -clp:ErrorsOnly
popd
popd

pushd %sdlmixer_dir%
cmake -S ./ -B bin -DSDL2_LIBRARY=../%sdl_lib% -DSDL2_MAIN_LIBRARY=../%sdl_main% -DSDL2_INCLUDE_DIR=../%sdl_dir%/include/ -DBUILD_SHARED_LIBS=0

pushd bin
msbuild SDL2_mixer.vcxproj -p:Configuration=Release -clp:ErrorsOnly
popd

popd