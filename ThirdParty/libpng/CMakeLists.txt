#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.18)

# -------------------------------------------------------------------------------------------------

project(
	LibPNG
	VERSION 1.6.37
	DESCRIPTION "Widely used bitmap storage format with lossless compression"
)

include("../../BuildSystem/cmake/cplusplus.cmake")

if(NOT (TARGET ZLib))
	add_subdirectory(${PROJECT_SOURCE_DIR}/../zlib ${CMAKE_BINARY_DIR}/zlib)
endif()

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/downloads/libpng-1.6.37.tar.gz)
	message(STATUS "Downloading LibPNG sources")
	file(
		DOWNLOAD https://download.sourceforge.net/libpng/libpng-1.6.37.tar.gz
		${PROJECT_SOURCE_DIR}/downloads/libpng-1.6.37.tar.gz
		SHOW_PROGRESS
		EXPECTED_HASH SHA256=daeb2620d829575513e35fecc83f0d3791a620b9b93d800b763542ece9390fb4
	)
endif()

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/build)
	message(STATUS "Extracting LibPNG sources")
	file(
		ARCHIVE_EXTRACT
		INPUT ${PROJECT_SOURCE_DIR}/downloads/libpng-1.6.37.tar.gz
		DESTINATION ${CMAKE_BINARY_DIR}/extract
	)
	file(
		RENAME
		${CMAKE_BINARY_DIR}/extract/libpng-1.6.37
		${PROJECT_SOURCE_DIR}/build
	)
endif()

file(
	COPY "${PROJECT_SOURCE_DIR}/pnglibconf.h"
	DESTINATION "${PROJECT_SOURCE_DIR}/build/"
)
file(
	COPY "${PROJECT_SOURCE_DIR}/config.h"
	DESTINATION "${PROJECT_SOURCE_DIR}/build/"
)

# -------------------------------------------------------------------------------------------------

set(
	sourceFiles
	"build/png.c"
	"build/pngerror.c"
	"build/pngget.c"
	"build/pngmem.c"
	"build/pngpread.c"
	"build/pngread.c"
	"build/pngrio.c"
	"build/pngrtran.c"
	"build/pngrutil.c"
	"build/pngset.c"
	"build/pngtrans.c"
	"build/pngwio.c"
	"build/pngwrite.c"
	"build/pngwtran.c"
	"build/pngwutil.c"
	"build/arm/arm_init.c"
	"build/arm/palette_neon_intrinsics.c"
	"build/arm/filter_neon_intrinsics.c"
)

set(
	headerFiles
	"build/pnginfo.h"
	"build/config.h"
	"build/png.h"
	"build/pngconf.h"
	"build/pngdebug.h"
	"build/pngpriv.h"
	"build/pngstruct.h"
	"build/pnglibconf.h"
)

# -------------------------------------------------------------------------------------------------

add_library(LibPNG STATIC)
add_library(LibPNG::Static ALIAS LibPNG)

if(${CMAKE_PROJECT_NAME} STREQUAL "LibPNG")
	enable_target_compiler_warnings(LibPNG)
else()
	disable_target_compiler_warnings(LibPNG)
endif()

target_link_libraries(LibPNG PRIVATE ZLib::Static)

if(NOT WIN32)
	target_compile_options(LibPNG PRIVATE -fexceptions)
endif()

target_include_directories(
	LibPNG
	PUBLIC "build"
)

target_sources(
	LibPNG
	PUBLIC ${headerFiles}
	PRIVATE ${sourceFiles}
)

#set_target_properties(LibPNG PROPERTIES PREFIX "")
set_target_properties(LibPNG PROPERTIES OUTPUT_NAME "png")

# -------------------------------------------------------------------------------------------------

install(
	FILES ${headerFiles}
	DESTINATION ${PROJECT_SOURCE_DIR}/Include
)

install(
	FILES ${headerFiles}
	DESTINATION ${PROJECT_SOURCE_DIR}/Include
)

install(
	TARGETS LibPNG
	ARCHIVE
	DESTINATION ${PROJECT_SOURCE_DIR}/bin/${CMAKE_COMPILER_TAG}
)

# -------------------------------------------------------------------------------------------------

file(
	WRITE "${PROJECT_SOURCE_DIR}/LibPNGConfig.cmake"
	"#!/usr/bin/cmake

# Configuration to include LibPNG in a CMake-based project. If you want to
# reference LibPNG as an externally compiled static library, do this:
#
#    set(LibPNG_DIR \"../ThirdParty/libpng\")
#    find_package(LibPNG REQUIRED CONFIG)
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE LibPNG::Static
#    )
#
# Alternatively, if you want to build LibPNG together with your project,
# use the normal CMakeLists.txt with CMake's add_subdirectory() command:
#
#    add_subdirectory(
#        \"\${PROJECT_SOURCE_DIR}/../ThirdParty/libpng\"
#        \"\${CMAKE_BINARY_DIR}/libpng\"
#    )
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE LibPNG
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

add_library(LibPNG::Static STATIC IMPORTED)

# This may cause warnings on recent GCC versions (10.0.0+?) with LTO because GCC detects
# that the headers used during build (residing in build/) are not the same used when
# linking the library (copies resising in Include/).
#
# CMake doesn't run the install step during build, so the only way to get the headers
# in place before building would be by copying them rather than installing them.
set_target_properties(
	LibPNG::Static PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES \"\${CMAKE_CURRENT_LIST_DIR}/Include\"
	IMPORTED_LINK_INTERFACE_LANGUAGES \"C\"
)

if(WIN32)
	set_target_properties(
		LibPNG::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/libpng.lib\"
		COMPILE_DEFINITIONS OS_LINUX
	)
else()
	set_target_properties(
		LibPNG::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/libpng.a\"
	)
endif()

message(STATUS \"Imported LibPNG targets with binaries in '\${CMAKE_CURRENT_LIST_DIR}'\")"
)

# -------------------------------------------------------------------------------------------------
