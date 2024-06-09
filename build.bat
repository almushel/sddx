:: %1: Path to SDL2 source directory
:: %2: Path to SDL2_mixer source directory
:: %3: Build all? Default: false
@echo off

set options=-I %1\include -I %2\include -DSDL_MAIN_HANDLED -DDEBUG
set link_options=-SUBSYSTEM:CONSOLE -LIBPATH:%1\VisualC\x64\Debug -LIBPATH:%2\VisualC\x64\Debug -OUT:bin\sddx.exe
set src_files=src\main.c src\game.c src\assets.c src\game_math.c
set libs=SDL2.lib SDL2main.lib SDL2_mixer.lib winmm.lib version.lib Imm32.lib Setupapi.lib

if exist %1 (
	if not "%~3"=="" ( if "%~3"=="true" (
		pushd %1\VisualC
		msbuild -m -p:Configuration="Debug" -p:Platform="x64"
		popd
		if not errorlevel == 0 exit

		echo Copying SDL2.dll...
		copy %1\VisualC\x64\Debug\SDL2.dll SDL2.dll
	))

	if exist %2 (
		if not "%~3"=="" ( if "%~3"=="true" (
			pushd %2\VisualC
			msbuild -m -p:Configuration="Debug" -p:Platform="x64"
			popd
			if not errorlevel == 0 exit

			echo Copying SDL2_mixer.dll...
			copy %2\VisualC\x64\Debug\SDL2_mixer.dll SDL2_mixer.dll
		))
		
		if not exist "bin" mkdir bin
		cl %options% %src_files% /link %link_options% %libs%
	)
)

