# sddx

A C/SDL port of my original vanilla JS game [Space Drifter](https://github.com/almushel/space-drifter).

## Building

Because SDL_Mixer requires an existing build of SDL (and maybe also because I don't know what I'm doing), I've written a batch script that sets up and builds the static libraries for SDL2 and SDL_Mixer in sequence.

```cmd
./build_sdl.bat
```

The library source and build directories in this script as well as `EXTERNAL_DIR` in [CMakeLists](src/CMakeLists.txt) should be updated to point to local copies of SDL2 and SDL_Mixer. Alternatively, this build script can be ignored, and release binaries of both used instead.