::!%windir%\System32\cmd.exe
@ECHO OFF

SET processorCount=%NUMBER_OF_PROCESSORS%

FOR %%m IN (Debug Release) DO (

	REM Delete existing CMake intermediate directory
	REM
	REM CMake doesn't support anything but a full rebuild unless you manually
	REM enter each and every source file into your CMakeLists.txt. See the
	REM documentation on file(GLOB_RECURSE ...) for that turd.
	REM
	IF "%1"=="full" (
		IF EXIST obj\cmake-%%m rd /s /q obj\cmake-%%m
	)

	REM Let CMake build a Makefile (that's the default)
	REM
	REM Up until Visual Studio 2019, CMake used different generators to switch
	REM between 32-bit and 64-bit (and defaults to 32-bits). Unless we want to
	REM drop support for Visual Studio 2017, this variable is the only way
	REM where CMake still picks up *any* installed Visual Studio version and does
	REM a 64-bit build.
	REM
	IF "%1"=="ninja" (
		cmake ^
			-B obj\cmake-%%m ^
			-D CMAKE_BUILD_TYPE=%%m ^
			-D WANT_TIFF=ON ^
			-D WANT_JPG=ON ^
			-D WANT_PNG=ON ^
			-GNinja
	) ELSE (
		cmake ^
			-B obj\cmake-%%m ^
			-D CMAKE_BUILD_TYPE=%%m ^
			-D CMAKE_GENERATOR_PLATFORM=x64 ^
			-D WANT_TIFF=ON ^
			-D WANT_JPG=ON ^
			-D WANT_PNG=ON ^
			-D BUILD_UNIT_TESTS=ON ^
			-D BUILD_BENCHMARK=ON
	)

	REM Compile the binary
	cmake ^
		--build obj\cmake-%%m ^
		--config %%m ^
		--parallel %processorCount%

	REM Put build artifacts in ./bin/windows-msvc14.1-amd64-release/ or similar
	cmake ^
		--install obj\cmake-%%m ^
		--config %%m

)
