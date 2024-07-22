:: %1: Path to SDL2 source directory
:: %2: Path to SDL2_mixer source directory
:: %3: Build all? Default: false
@echo off

set options=-Zi -I %1/include -I %2/include -DSDL_MAIN_HANDLED -DDEBUG
set link_options=-SUBSYSTEM:CONSOLE -LIBPATH:%1/VisualC/x64/Debug -LIBPATH:%2/VisualC/x64/Debug -OUT:sddx.exe
set src_files=../src/main.c ../src/engine/platform.c ../src/game/game.c
set libs=SDL2.lib SDL2main.lib SDL2_mixer.lib winmm.lib version.lib Imm32.lib Setupapi.lib

if not exist "bin" mkdir bin
if exist %1 (
	if not "%~3"=="" ( if "%~3"=="true" (
		pushd "%1/VisualC"
		msbuild -m -p:Configuration="Debug" -p:Platform="x64" || goto :Exit
		popd

		xcopy /f "%1/VisualC/x64/Debug/SDL2.dll" "bin" || goto :Exit
	))

	if exist %2 (
		if not "%~3"=="" ( if "%~3"=="true" (
			pushd "%2/VisualC"
			msbuild -m -p:Configuration="Debug" -p:Platform="x64" || goto :Exit
			popd

			xcopy /f "%2/VisualC/x64/Debug/SDL2_mixer.dll" "bin" || goto :Exit
		))
		
		pushd bin
		cl %options% %src_files% /link %link_options% %libs%
		popd
	)
)

:Exit
popd
