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

	# Let CMake build a Makefile or a Ninja build script
	if [ $# -ge 1 ] && [ $1 == "ninja" ]; then
		cmake \
			-B obj/cmake-$buildMode \
			-D CMAKE_BUILD_TYPE=$buildMode \
			-D BUILD_UNIT_TESTS=ON \
			-D BUILD_BENCHMARK=ON \
			-D BENCHMARK_THIRD_PARTY_LIBRARIES=OFF \
			-GNinja
	else
		cmake \
			-B obj/cmake-$buildMode \
			-D CMAKE_BUILD_TYPE=$buildMode \
			-D BUILD_UNIT_TESTS=ON \
			-D BUILD_BENCHMARK=ON \
			-D BENCHMARK_THIRD_PARTY_LIBRARIES=OFF
	fi

	# Compile the binary
	cmake \
		--build obj/cmake-$buildMode \
		--config $buildMode \
		--parallel $processorCount

	# Put build artifacts in ./bin/linux-gcc9.2-amd64-release/ or similar
	cmake \
		--install obj/cmake-$buildMode \
		--config $buildMode

done
