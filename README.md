# Space Drifter Native

A C/SDL port of my original JavaScript game [Space Drifter](https://github.com/almushel/space-drifter).

## Building Dependencies

Space Drifter Native has two external dependencies: SDL2 and SDL2_mixer.
Download or clone [SDL2](https://github.com/libsdl-org/SDL)(2.30.3) and [SDL2_mixer](https://github.com/libsdl-org/SDL_mixer)(2.8.0).

### Windows (MSVC)

The build script takes three positional paramters.
1. The path to the SDL2 source directory.
2. The path to the SDL2_mixer source directory
3. If set to `true`, will build SDL2 and SDL2_mixer in order before building the game.

```sh
./build.bat path\to\SDL2 path\to\SDL_mixer true
```

### Linux 

```bash
cd SDL2
./configure && make && sudo make install

cd ../SDL2_mixer
./external/download.sh
./configure && make && sudo make install
```

## Build Space Drifter

The project includes a very straightforward `CMakeLists.txt` file for convenience. 
See `add_executable` and `target_link_libraries` for the source files and libraries required to build.

```sh
cmake -B bin -S src

#Windows MSVC
pushd bin && msbuild sddx.sln && popd

# Linux
pushd bin && make && popd
    
```
