:: %1: Path to SDL2 source directory
:: %2: Path to SDL2_mixer source directory
:: %3: Build all? Default: false

if exist %1 (
	if not "%~3"=="" ( if "%~3"=="true" (
		pushd %1\VisualC
		msbuild -m -p:Configuration="Debug" -p:Platform="x64"
		popd
		if not errorlevel == 0 exit

		echo Copying SDL2.dll...
		copy %1\VisualC\x64\Debug\SDL2.dll SDL2.dll
	))

	set INCLUDE="%INCLUDE%;%1\include;
	set LIB="%LIB%;%1\VisualC\x64\Debug;
	set UseEnv=true

	if exist %2 (
		if not "%~3"=="" ( if "%~3"=="true" (
			pushd %1\VisualC
			msbuild -m -p:Configuration="Debug" -p:Platform="x64"
			popd
			if not errorlevel == 0 exit

			echo Copying SDL2_mixer.dll...
			copy %2\VisualC\x64\Debug\SDL2_mixer.dll SDL2_mixer.dll
		))

		set INCLUDE="%INCLUDE%;%2\include"
		set LIB="%LIB%;%2\VisualC\x64\Debug"
		
		pushd bin
		msbuild sddx.sln -p:Configuration="Debug" -p:Platform="x64"
		popd
	)
)

