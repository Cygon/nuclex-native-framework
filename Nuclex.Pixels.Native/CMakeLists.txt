#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.15)

# -------------------------------------------------------------------------------------------------

project(
	NuclexPixelsNative
	VERSION 1.0.0
	DESCRIPTION "Bitmap loading, pixel format conversion and scaling library"
)

option(
	BUILD_DOCS
	"Whether to generate documentation via Doxygen"
	OFF
)
option(
	BUILD_UNIT_TESTS
	"Whether to build the unit test executable. This will require an extra \
	compilation of the entire source tree as well as the GoogleTest library."
	OFF
)
option(
	BUILD_BENCHMARK
	"Whether to build the benchmark executable. This will require an extra \
	compilation of the entire source tree as well as the Celero library."
	OFF
)

# JPEG-XL depends on AVIF and WEBP.
set(WANT_JPEGXL OFF CACHE BOOL "Whether to integrate JPEG-XL image loading and saving")
set(WANT_AVIF OFF CACHE BOOL "Whether to integrate AV1 image loading and saving")
set(WANT_WEBP ON CACHE BOOL "Whether to integrate WebP image loading and saving")
# TIFF depends on LibJPEG and OpenEXR
set(WANT_TIFF ON CACHE BOOL "Whether to integrate TIFF image loading and saving")
set(WANT_JPG ON CACHE BOOL "Whether to integrate JPG image loading and saving")
set(WANT_EXR ON CACHE BOOL "Whether to integrate OpenEXR image loading and saving")
set(WANT_PNG ON CACHE BOOL "Whether to integrate PNG image loading and saving")

# -------------------------------------------------------------------------------------------------

# Contains compiler options, compiler tag for output directory, etc.
include("../BuildSystem/cmake/cplusplus.cmake")

# The Unix build pipeline doesn't automatically include threading, so search for
# the pthreads library in order to link against it later on.
#   https://en.wikipedia.org/wiki/Pthreads
find_package(Threads REQUIRED)

# Add Nuclex.Support.Native as a sub-project, we link it for utility methods.
if(NOT (TARGET NuclexSupportNative))
	add_subdirectory(
		${PROJECT_SOURCE_DIR}/../Nuclex.Support.Native
		${CMAKE_BINARY_DIR}/NuclexSupportNative
	)
endif()

# Already included by Nuclex.Support.Native, CMake doesn't like a second inclusion...
if(NOT (TARGET GoogleTest))
	add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/gtest ${CMAKE_BINARY_DIR}/gtest)
endif()

message(STATUS "Enabled options for Nuclex.Pixels.Native:")
message(STATUS "  ⚫ Build core library")

if(BUILD_UNIT_TESTS)
	message(STATUS "  ⚫ Build unit tests")

	# Add GoogleTest as a sub-project so we can link our unit test executable
	if(NOT (TARGET GoogleTest))
		add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/gtest ${CMAKE_BINARY_DIR}/gtest)
	endif()
endif()

if(BUILD_BENCHMARK)
	message(STATUS "  ⚫ Build benchmark")

	# Add Celero as a sub-project so we can link our benchmark executable
	if(NOT (TARGET Celero))
		add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/celero ${CMAKE_BINARY_DIR}/celero)
	endif()
endif()

# Libraries to load and save each image format
if(WANT_JPEGXL)
	message(STATUS "  ⚫ Support JPEG-XL")
	add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/jpeg-xl ${CMAKE_BINARY_DIR}/jpeg-xl)
endif()
if(WANT_AVIF)
	message(STATUS "  ⚫ Support AVIF")
	# LibAVIF may have already been referenced by JPEG-XL to borrow its compression code
	if(NOT (TARGET LibAVIF))
		add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/av1f ${CMAKE_BINARY_DIR}/av1f)
	endif()
endif()
if(WANT_WEBP)
	message(STATUS "  ⚫ Support WebP")
	# LibWebP may have already been referenced by JPEG-XL to borrow its compression code
	if(NOT (TARGET LibWebP))
		add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/libwebp ${CMAKE_BINARY_DIR}/libwebp)
	endif()
endif()
if(WANT_JPG)
	message(STATUS "  ⚫ Support JPEG")
	# LibJPEG may have already been referenced by LibTIFF to borrow its compression code
	if(NOT (TARGET LibJPEG))
		add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/libjpeg ${CMAKE_BINARY_DIR}/libjpeg)
	endif()
endif()
if(WANT_TIFF)
	message(STATUS "  ⚫ Support TIFF")
	add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/libtiff ${CMAKE_BINARY_DIR}/libtiff)
endif()
if(WANT_EXR)
	message(STATUS "  ⚫ Support EXR")
	# OpenEXR may have already been referenced by LibTIFF to borrow its compression code
	if(NOT (TARGET OpenEXR))
		add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/openexr ${CMAKE_BINARY_DIR}/openexr)
	endif()
endif()
if(WANT_PNG)
	message(STATUS "  ⚫ Support PNG")
	add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/libpng ${CMAKE_BINARY_DIR}/libpng)
endif()

# Use CMake's own package for locating Doxygen on the system
if(BUILD_DOCS)
	find_package(Doxygen)
endif()

# -------------------------------------------------------------------------------------------------

# Project structure
#
#   ProjectName/
#     Source/                   All source files, using deeper directories as needed
#     Include/ProjectName/      All public headers, using deeper directories as needed
#     Tests/                    All unit tests, using deeper directories as needed
#     Benchmarks/               All benchmark files, using deeper directories as needed
#
# CMake documentation:
#   |  Note: We do not recommend using GLOB to collect a list of
#   |  source files from your source tree. If no CMakeLists.txt file
#   |  changes when a source is added or removed then the generated
#   |  build system cannot know when to ask CMake to regenerate.
#
# As so very often, CMake becomes a hurdle rather than helping.
# I'm not going to manually maintain a list of source files. Rebuilds
# where files are added, removed or renamed need to be from scratch.
#
file(
	GLOB_RECURSE sourceFiles
	CONFIGURE_DEPENDS
	"Source/*.cpp"
	"Source/*.c"
)
file(
	GLOB_RECURSE headerFiles
	CONFIGURE_DEPENDS
	"Include/Nuclex/Pixels/*.h"
)
file(
	GLOB_RECURSE unittestFiles
	CONFIGURE_DEPENDS
	"Tests/*.cpp"
)
file(
	GLOB_RECURSE benchmarkFiles
	CONFIGURE_DEPENDS
	"Benchmarks/*.cpp"
)

# -------------------------------------------------------------------------------------------------

function(add_third_party_libraries target_name)

	if(WANT_JPEGXL)
		target_compile_definitions(${target_name} PRIVATE NUCLEX_PIXELS_HAVE_JPEGXL)
		target_link_libraries(${target_name} PRIVATE JPEG-XL::Static)
	endif()
	if(WANT_AV1F)
		target_compile_definitions(${target_name} PRIVATE NUCLEX_PIXELS_HAVE_AV1F)
		target_link_libraries(${target_name} PRIVATE AV1F::Static)
	endif()
	if(WANT_WEBP)
		target_compile_definitions(${target_name} PRIVATE NUCLEX_PIXELS_HAVE_WEBP)
		message(STATUS "Enabled LibWebP for ${target_name}")
		target_link_libraries(${target_name} PRIVATE LibWebP::Static)
	endif()
	if(WANT_TIFF)
		target_compile_definitions(${target_name} PRIVATE NUCLEX_PIXELS_HAVE_LIBTIFF)
		target_link_libraries(${target_name} PRIVATE LibTIFF::Static)
	endif()
	if(WANT_JPG)
		target_compile_definitions(${target_name} PRIVATE NUCLEX_PIXELS_HAVE_LIBJPEG)
		target_link_libraries(${target_name} PRIVATE LibJPEG::Static)
	endif()
	if(WANT_EXR)
		target_compile_definitions(${target_name} PRIVATE NUCLEX_PIXELS_HAVE_OPENEXR)
		target_link_libraries(${target_name} PRIVATE OpenEXR::Static)
	endif()
	if(WANT_PNG)
		target_compile_definitions(${target_name} PRIVATE NUCLEX_PIXELS_HAVE_LIBPNG)
		target_link_libraries(${target_name} PRIVATE ZLib::Static)
		target_link_libraries(${target_name} PRIVATE LibPNG::Static)
	endif()

	# On Unix systems, the library and unit test executable should look for
	# dependencies in its own directory first.
	set_target_properties(
		${target_name} PROPERTIES
		BUILD_RPATH_USE_ORIGIN ON
		BUILD_WITH_INSTALL_RPATH ON
		INSTALL_RPATH_USE_LINK_PATH OFF
		INSTALL_RPATH "\${ORIGIN}"
	)

endfunction()

# -------------------------------------------------------------------------------------------------

# Shared library that can be linked to other projects
add_library(NuclexPixelsNative SHARED)

# Enable compiler warnings only if this library is compiled on its own.
# If it's used as a sub-project, the including project's developers aren't
# interested in seeing warnings from a project they're not maintaining.
if(${CMAKE_PROJECT_NAME} STREQUAL "NuclexPixelsNative")
	enable_target_compiler_warnings(NuclexPixelsNative)
else()
	disable_target_compiler_warnings(NuclexPixelsNative)
endif()

# Add directory with public headers to include path
target_include_directories(
	NuclexPixelsNative
	PUBLIC "Include"
)

# Add public headers and sources to compilation list
# (headers, too, in case CMake is used to generate an IDE project)
target_sources(
	NuclexPixelsNative
	PUBLIC ${headerFiles}
	PRIVATE ${sourceFiles}
)

# Link against PThreads and Nuclex.Support.Native
target_link_libraries(
	NuclexPixelsNative
	PRIVATE Threads::Threads
	PUBLIC NuclexSupportNative
)

# Add include directories and static libraries of all enabled image formats
add_third_party_libraries(NuclexPixelsNative)

# On Windows, we want the shared library to be named Nuclex.Pixels.Native.dll
if(WIN32)
	set_target_properties(
		NuclexPixelsNative
		PROPERTIES OUTPUT_NAME "Nuclex.Pixels.Native"
	)
endif()

# -------------------------------------------------------------------------------------------------

if(BUILD_UNIT_TESTS)

	# Executable that runs the unit tests (main() supplied by GoogleTest)
	add_executable(NuclexPixelsNativeTests)

	# Enable compiler warnings only if this library is compiles on its own.
	# If it's used as a sub-project, the including project's developers aren't
	# interested in seeing warnings from a project they're not maintaining.
	if(${CMAKE_PROJECT_NAME} STREQUAL "NuclexPixelsNative")
		enable_target_compiler_warnings(NuclexPixelsNativeTests)
	else()
		disable_target_compiler_warnings(NuclexPixelsNativeTests)
	endif()

	# Let the code know it's not being compiled into a shared library
	# (this disables visibility/exports, thus allowing the compiler detect
	# additional unused code and warn about it)
	target_compile_definitions(
		NuclexPixelsNativeTests
		PRIVATE NUCLEX_PIXELS_EXECUTABLE
	)

	# Add directory with public headers to include path
	target_include_directories(
		NuclexPixelsNativeTests
		PUBLIC "Include"
	)

	# Add public headers and sources (normal + unit tests) to compilation list
	# (headers, too, in case CMake is used to generate an IDE project)
	target_sources(
		NuclexPixelsNativeTests
		PRIVATE ${headerFiles}
		PRIVATE ${sourceFiles}
		PRIVATE ${unittestFiles}
	)

	# Link GoogleTest and the main() function supplied by GoogleTest
	# Also link against PThreads
	target_link_libraries(
		NuclexPixelsNativeTests
		PRIVATE GoogleTest::Static
		PRIVATE GoogleTest::Main
		PRIVATE Threads::Threads
		PUBLIC NuclexSupportNative
	)

	add_third_party_libraries(NuclexPixelsNativeTests)

	# On Windows, we want the executable to be named Nuclex.Pixels.Native.Tests.exe
	if(WIN32)
		set_target_properties(
			NuclexPixelsNativeTests
			PROPERTIES OUTPUT_NAME "Nuclex.Pixels.Native.Tests"
		)
	endif()

endif() # if BUILD_UNIT_TESTS enabled

# -------------------------------------------------------------------------------------------------

if(BUILD_BENCHMARK)

	# Executable that runs the benchmark (main() supplied by Celero)
	add_executable(NuclexPixelsNativeBenchmark)

	# Enable compiler warnings only if this library is compiled on its own.
	# If it's used as a sub-project, the including project's developers aren't
	# interested in seeing warnings from a project they're not maintaining.
	if(${CMAKE_PROJECT_NAME} STREQUAL "NuclexPixelsNative")
		enable_target_compiler_warnings(NuclexPixelsNativeBenchmark)
	else()
		disable_target_compiler_warnings(NuclexPixelsNativeBenchmark)
	endif()

	# Let the code know it's not being compiled into a shared library
	# (this disables visibility/exports, thus allowing the compiler detect
	# additional unused code and warn about it)
	target_compile_definitions(
		NuclexPixelsNativeBenchmark
		PRIVATE NUCLEX_PIXELS_EXECUTABLE
	)

	# Add directory with public headers to include path
	target_include_directories(
		NuclexPixelsNativeBenchmark
		PUBLIC "Include"
	)

	# Add public headers and sources (normal + benchmark) to compilation list
	# (headers, too, in case CMake is used to generate an IDE project)
	target_sources(
		NuclexPixelsNativeBenchmark
		PRIVATE ${headerFiles}
		PRIVATE ${sourceFiles}
		PRIVATE ${benchmarkFiles}
	)

	# Link Celero
	# Also link against PThreads
	target_link_libraries(
		NuclexPixelsNativeBenchmark
		PRIVATE Celero
		PRIVATE Threads::Threads
		PUBLIC NuclexSupportNative
	)

	add_third_party_libraries(NuclexPixelsNativeBenchmark)

	# On Windows, we want the executable to be named Nuclex.Pixels.Native.Benchmark.exe
	if(WIN32)
		set_target_properties(
			NuclexPixelsNativeBenchmark
			PROPERTIES OUTPUT_NAME "Nuclex.Pixels.Native.Benchmark"
		)
	endif()

endif() # if BUILD_BENCHMARK enabled

# -------------------------------------------------------------------------------------------------

# Install the shared library into a subdirectory of this CMakeLists.txt file
# under ./bin/linux-gcc9.3-amd64-debug/ (the second-level directory is called
# "compiler tag" and dynamically formed -- it ensures that when linking
# a pre-compiled shared library, the correct library is used).
install(
	TARGETS NuclexPixelsNative
	ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

# Do the same for Nuclex.Support.Native. Since we depend on this library
# and have set the rpath accordingly, it needs to be in the same directory
install(
	TARGETS NuclexSupportNative
	ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

# Install unit tests in same location as shared library.
if(BUILD_UNIT_TESTS)
	install(
		TARGETS NuclexPixelsNativeTests
		RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	)

	# Install .pdb files on Windows platforms for the unit tests, too.
	install_debug_symbols(NuclexPixelsNativeTests)
endif()

# Install benchmarks in same location as shared library.
if(BUILD_BENCHMARK)
	install(
		TARGETS NuclexPixelsNativeBenchmark
		RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	)

	# Install .pdb files on Windows platforms for the benchmark, too.
	install_debug_symbols(NuclexPixelsNativeBenchmark)
endif()

# -------------------------------------------------------------------------------------------------

if(BUILD_DOCS)

	if(NOT DOXYGEN_FOUND)
		message(FATAL_ERROR "Can't build documentation because Doxygen was not found")
	endif()

	add_custom_target(
		NuclexPixelsNativeDocs ALL
		COMMAND ${DOXYGEN_EXECUTABLE} "Nuclex.Pixels.Native.doxygen.cfg"
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	)

endif()

# -------------------------------------------------------------------------------------------------

file(
	WRITE "${PROJECT_SOURCE_DIR}/NuclexPixelsNativeConfig.cmake"
	"#!/usr/bin/cmake

# Configuration to include Nuclex.Pixels.Native in a CMake-based project. If you want to
# reference Nuclex.Pixels.Native as an externally compiled static library, do this:
#
#    set(NuclexPixelsNative_DIR \"../Nuclex.Pixels.Native\")
#    find_package(NuclexPixelsNative REQUIRED CONFIG)
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE NuclexPixelsNative::Dynamic
#    )
#
# Alternatively, if you want to build Nuclex.Pixels.Native together with your project,
# use the normal CMakeLists.txt with CMake's add_subdirectory() command:
#
#    add_subdirectory(
#        \"\${PROJECT_SOURCE_DIR}/../Nuclex.Pixels.Native\"
#        \"\${CMAKE_BINARY_DIR}/nuclex.pixels.native\"
#    )
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE NuclexPixelsNative
#    )
#

# -------------------------------------------------------------------------------------------------

if(NOT DEFINED NUCLEX_COMPILER_TAG)
	message(
		FATAL_ERROR
		\"NUCLEX_COMPILER_TAG not defined! Include cplusplus.cmake before importing this package \\
		in order to generate a tag identifying the platform/compiler/architecture/variant!\"
	)
endif()

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}\")
	# TODO: Warn and link release build when compiling in debug mode
	# TODO: Warn and link build for older compiler version if found
	message(
		FATAL_ERROR
		\"Directory '\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}' not found. \\
		Please either build and install this project before importing it via \\
		find_package() or use this project's main CMakeFiles.txt via add_subdirectory()!\"
	)
endif()

# -------------------------------------------------------------------------------------------------

add_library(NuclexPixelsNative::Dynamic SHARED IMPORTED)

# This may cause warnings on recent GCC versions (10.0.0+?) with LTO because GCC detects
# that the headers used during build (residing in build/) are not the same used when
# linking the library (copies resising in Include/).
#
# CMake doesn't run the install step during build, so the only way to get the headers
# in place before building would be by copying them rather than installing them.
set_target_properties(
	NuclexPixelsNative::Dynamic PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES \"\${CMAKE_CURRENT_LIST_DIR}/Include\"
	IMPORTED_LINK_INTERFACE_LANGUAGES \"C\"
)

if(WIN32)
	set_target_properties(
		NuclexPixelsNative::Dynamic PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/Nuclex.Pixels.Native.lib\"
	)
else()
	set_target_properties(
		NuclexPixelsNative::Dynamic PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/libNuclexPixelsNative.so\"
	)
endif()

message(STATUS \"Imported Nuclex.Pixels.Native targets with binaries in '\${CMAKE_CURRENT_LIST_DIR}'\")"
)

# -------------------------------------------------------------------------------------------------
