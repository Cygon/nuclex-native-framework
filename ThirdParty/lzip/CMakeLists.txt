#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.18)

# -------------------------------------------------------------------------------------------------

project(
	LZip
	VERSION 1.11.0
	DESCRIPTION "Cleaner implementation of LZMA compression algorithm"
)

include("../../BuildSystem/cmake/cplusplus.cmake")

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/downloads/lzlib-1.11.tar.gz)
	message(STATUS "Downloading LZip sources")
	file(
		DOWNLOAD http://download.savannah.gnu.org/releases/lzip/lzlib/lzlib-1.11.tar.gz
		${PROJECT_SOURCE_DIR}/downloads/lzlib-1.11.tar.gz
		SHOW_PROGRESS
		EXPECTED_HASH SHA256=6c5c5f8759d1ab7c4c3c53788ea2d9daad04aeddcf338226893f8ff134914d36
	)
endif()

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/build)
	message(STATUS "Extracting LZip sources")
	file(
		ARCHIVE_EXTRACT
		INPUT ${PROJECT_SOURCE_DIR}/downloads/lzlib-1.11.tar.gz
		DESTINATION ${CMAKE_BINARY_DIR}/extract
	)
	file(
		RENAME
		${CMAKE_BINARY_DIR}/extract/lzlib-1.11
		${PROJECT_SOURCE_DIR}/build
	)
endif()

# -------------------------------------------------------------------------------------------------

set(
	sourceFiles
	"build/lzlib.c"
)
set(
	headerFiles
	"build/lzlib.h"
)

# -------------------------------------------------------------------------------------------------

add_library(LZip STATIC)
add_library(LZip::Static ALIAS LZip)

if(NOT WIN32)
	target_compile_definitions(
		LZip
		PUBLIC OS_LINUX
	)
endif()

target_include_directories(
	LZip
	PUBLIC "build"
)

target_sources(
	LZip
	PUBLIC ${headerFiles}
	PRIVATE ${sourceFiles}
)

#set_target_properties(LZip PROPERTIES PREFIX "")
set_target_properties(LZip PROPERTIES OUTPUT_NAME "lzip")

# -------------------------------------------------------------------------------------------------

install(
	FILES ${headerFiles}
	DESTINATION ${PROJECT_SOURCE_DIR}/Include
)

install(
	TARGETS LZip
	ARCHIVE
	DESTINATION ${PROJECT_SOURCE_DIR}/bin/${CMAKE_COMPILER_TAG}
)

# -------------------------------------------------------------------------------------------------

file(
	WRITE "${PROJECT_SOURCE_DIR}/LZipConfig.cmake"
	"#!/usr/bin/cmake

# Configuration to include LZip in a CMake-based project. If you want to
# reference LZip as an externally compiled static library, do this:
#
#    set(LZip_DIR \"../ThirdParty/lzip\")
#    find_package(LZip REQUIRED CONFIG)
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE LZip::Static
#    )
#
# Alternatively, if you want to build LZip together with your project,
# use the normal CMakeLists.txt with CMake's add_subdirectory() command:
#
#    add_subdirectory(
#        \"\${PROJECT_SOURCE_DIR}/../ThirdParty/lzip\"
#        \"\${CMAKE_BINARY_DIR}/lzip\"
#    )
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE LZip
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

add_library(LZip::Static STATIC IMPORTED)

# This may cause warnings on recent GCC versions (10.0.0+?) with LTO because GCC detects
# that the headers used during build (residing in build/) are not the same used when
# linking the library (copies resising in Include/).
#
# CMake doesn't run the install step during build, so the only way to get the headers
# in place before building would be by copying them rather than installing them.
set_target_properties(
	LZip::Static PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES \"\${CMAKE_CURRENT_LIST_DIR}/Include\"
	IMPORTED_LINK_INTERFACE_LANGUAGES \"C\"
)

if(WIN32)
	set_target_properties(
		LZip::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/liblzip.lib\"
	)
else()
	set_target_properties(
		LZip::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/liblzip.a\"
	)
endif()

message(STATUS \"Imported LZip targets with binaries in '\${CMAKE_CURRENT_LIST_DIR}'\")"
)

# -------------------------------------------------------------------------------------------------
