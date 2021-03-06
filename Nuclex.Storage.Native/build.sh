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
	cmake -B obj/cmake-$buildMode -DCMAKE_BUILD_TYPE=$buildMode \
		-D WANT_BROTLI=ON \
		-D WANT_BSC=ON \
		-D WANT_CSC=ON \
		-D WANT_LZIP=ON \
		-D WANT_SEVENZIP=ON \
		-D WANT_TANGELO=ON \
		-D WANT_UNRAR=ON \
		-D WANT_ZLIB=ON \
		-D WANT_ZPAQ=ON

	# Compile the binary
	cmake --build obj/cmake-$buildMode --config $buildMode --parallel $processorCount

	# Put build artifacts in ./bin/linux-gcc9.2-amd64-release/ or similar
	cmake --install obj/cmake-$buildMode --config $buildMode

done
