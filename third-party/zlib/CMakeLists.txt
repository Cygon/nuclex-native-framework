#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.18)

# -------------------------------------------------------------------------------------------------

project(
	ZLib
	VERSION 1.2.11
	DESCRIPTION "Compression algorithm used in zip and gz archives"
)

include("../../build-system/cmake/cplusplus.cmake")

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/downloads/v1.2.11.tar.gz)
	message(STATUS "Downloading ZLib sources")
	file(
		DOWNLOAD https://github.com/madler/zlib/archive/v1.2.11.tar.gz
		${PROJECT_SOURCE_DIR}/downloads/v1.2.11.tar.gz
		SHOW_PROGRESS
		EXPECTED_HASH SHA256=629380c90a77b964d896ed37163f5c3a34f6e6d897311f1df2a7016355c45eff
	)
endif()

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/build)
	message(STATUS "Extracting ZLib sources")
	file(
		ARCHIVE_EXTRACT
		INPUT ${PROJECT_SOURCE_DIR}/downloads/v1.2.11.tar.gz
		DESTINATION ${CMAKE_BINARY_DIR}/extract
	)
	file(
		RENAME
		${CMAKE_BINARY_DIR}/extract/zlib-1.2.11
		${PROJECT_SOURCE_DIR}/build
	)
endif()

# -------------------------------------------------------------------------------------------------

# Needed unless NO_GZIP defined
#	"build/gzclose.c"
#	"build/gzlib.c"
#	"build/gzread.c"
#	"build/gzwrite.c"

# Callback version of inflate.c
#	"build/infback.c"

set(
	sourceFiles
	"build/adler32.c"
	"build/compress.c"
	"build/crc32.c"
	"build/deflate.c"
	"build/inffast.c"
	"build/inflate.c"
	"build/inftrees.c"
	"build/trees.c"
	"build/uncompr.c"
	"build/zutil.c"
)
set(
	headerFiles
	"build/zconf.h"
	"build/zlib.h"
	"build/zutil.h"
)

# -------------------------------------------------------------------------------------------------

add_library(ZLibSolo STATIC)
add_library(ZLib::SoloStatic ALIAS ZLibSolo)

target_compile_definitions(
	ZLibSolo
	PUBLIC ZLIB_CONST
	PUBLIC NO_GZIP
	PUBLIC Z_SOLO
)

target_include_directories(
	ZLibSolo
	PUBLIC "build"
)

target_sources(
	ZLibSolo
	PUBLIC ${headerFiles}
	PRIVATE ${sourceFiles}
)

set_target_properties(ZLibSolo PROPERTIES PREFIX "")
set_target_properties(ZLibSolo PROPERTIES OUTPUT_NAME "zlibsolo")

# -------------------------------------------------------------------------------------------------

add_library(ZLib STATIC)
add_library(ZLib::Static ALIAS ZLib)

target_compile_definitions(
	ZLib
	PUBLIC ZLIB_CONST
	PUBLIC NO_GZIP
)

target_include_directories(
	ZLib
	PUBLIC "build"
)

target_sources(
	ZLib
	PUBLIC ${headerFiles}
	PRIVATE ${sourceFiles}
)

set_target_properties(ZLib PROPERTIES PREFIX "")
set_target_properties(ZLib PROPERTIES OUTPUT_NAME "zlib")

# -------------------------------------------------------------------------------------------------

install(
	FILES ${headerFiles}
	DESTINATION ${PROJECT_SOURCE_DIR}/Include
)

install(
	TARGETS ZLibSolo
	ARCHIVE	DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)
install(
	TARGETS ZLib
	ARCHIVE	DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

# -------------------------------------------------------------------------------------------------

file(
	WRITE "${PROJECT_SOURCE_DIR}/ZLibConfig.cmake"
	"#!/usr/bin/cmake

# Configuration to include ZLib in a CMake-based project. If you want to
# reference ZLib as an externally compiled static library, do this:
#
#    set(ZLib_DIR \"../ThirdParty/zlib\")
#    find_package(ZLib REQUIRED CONFIG)
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE ZLib::Static
#    )
#
# Alternatively, if you want to build ZLib together with your project,
# use the normal CMakeLists.txt with CMake's add_subdirectory() command:
#
#    add_subdirectory(
#        \"\${PROJECT_SOURCE_DIR}/../ThirdParty/zlib\"
#        \"\${CMAKE_BINARY_DIR}/zlib\"
#    )
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE ZLib
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

add_library(ZLib::Static STATIC IMPORTED)

# This may cause warnings on recent GCC versions (10.0.0+?) with LTO because GCC detects
# that the headers used during build (residing in build/) are not the same used when
# linking the library (copies resising in Include/).
#
# CMake doesn't run the install step during build, so the only way to get the headers
# in place before building would be by copying them rather than installing them.
set_target_properties(
	ZLib::Static PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES \"\${CMAKE_CURRENT_LIST_DIR}/Include\"
	IMPORTED_LINK_INTERFACE_LANGUAGES \"C\"
)

if(WIN32)
	set_target_properties(
		ZLib::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/zlib.lib\"
	)
else()
	set_target_properties(
		ZLib::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/zlib.a\"
	)
endif()

message(STATUS \"Imported ZLib targets with binaries in '\${CMAKE_CURRENT_LIST_DIR}'\")"
)

# -------------------------------------------------------------------------------------------------
