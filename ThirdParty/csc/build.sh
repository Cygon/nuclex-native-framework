#!/bin/sh

processorCount=`grep -c ^processor /proc/cpuinfo`

for buildMode in Debug Release; do

	# Delete existing CMake intermediate directory
	#
	# CMake doesn't support anything but a full rebuild unless you manually
	# enter each and every source file into your CMakeLists.txt. See the
	# documentation on file(GLOB_RECURSE ...) for that turd.
	#
	if [ -d obj ]; then
		if [ -d obj/cmake-$buildMode ]; then
			rm -rf obj/cmake-$buildMode
		fi
	fi

	# Let CMake build a Makefile (that's the default)
	cmake -B obj/cmake-$buildMode -DCMAKE_BUILD_TYPE=$buildMode -DBUILD_CMD=ON

	# Compile the binary
	cmake --build obj/cmake-$buildMode --config $buildMode --parallel $processorCount

	# Put build artifacts in ./bin/linux-gcc9.2-amd64-release/ or similar
	cmake --install obj/cmake-$buildMode --config $buildMode

done
