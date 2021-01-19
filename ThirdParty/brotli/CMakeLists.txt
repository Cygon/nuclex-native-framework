#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.18)

# -------------------------------------------------------------------------------------------------

project(
	Brotli
	VERSION 1.0.7
	DESCRIPTION "Modern compression algorithm achieving high compression ratios"
)

include("../../BuildSystem/cmake/cplusplus.cmake")

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/downloads/v1.0.7.tar.gz)
	message(STATUS "Downloading Brotli sources")
	file(
		DOWNLOAD https://github.com/google/brotli/archive/v1.0.7.tar.gz
		${PROJECT_SOURCE_DIR}/downloads/v1.0.7.tar.gz
		SHOW_PROGRESS
		EXPECTED_HASH SHA256=4c61bfb0faca87219ea587326c467b95acb25555b53d1a421ffa3c8a9296ee2c
	)
endif()

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/build)
	message(STATUS "Extracting Brotli sources")
	file(
		ARCHIVE_EXTRACT
		INPUT ${PROJECT_SOURCE_DIR}/downloads/v1.0.7.tar.gz
		DESTINATION ${CMAKE_BINARY_DIR}/extract
	)
	file(
		RENAME
		${CMAKE_BINARY_DIR}/extract/brotli-1.0.7
		${PROJECT_SOURCE_DIR}/build
	)
endif()

# -------------------------------------------------------------------------------------------------

set(
	sourceFiles
	"build/c/common/dictionary.c"
	"build/c/common/transform.c"
	"build/c/dec/bit_reader.c"
	"build/c/dec/decode.c"
	"build/c/dec/huffman.c"
	"build/c/dec/state.c"
	"build/c/enc/backward_references.c"
	"build/c/enc/backward_references_hq.c"
	"build/c/enc/bit_cost.c"
	"build/c/enc/block_splitter.c"
	"build/c/enc/brotli_bit_stream.c"
	"build/c/enc/cluster.c"
	"build/c/enc/compress_fragment.c"
	"build/c/enc/compress_fragment_two_pass.c"
	"build/c/enc/dictionary_hash.c"
	"build/c/enc/encode.c"
	"build/c/enc/encoder_dict.c"
	"build/c/enc/entropy_encode.c"
	"build/c/enc/histogram.c"
	"build/c/enc/literal_cost.c"
	"build/c/enc/memory.c"
	"build/c/enc/metablock.c"
	"build/c/enc/static_dict.c"
	"build/c/enc/utf8_util.c"
	"build/c/fuzz/decode_fuzzer.c"
)
set(
	headerFiles
	"build/c/include/brotli/decode.h"
	"build/c/include/brotli/encode.h"
	"build/c/include/brotli/port.h"
	"build/c/include/brotli/types.h"
)

# -------------------------------------------------------------------------------------------------

# The original build created 3 libraries:
#   brotlienc-static.lib
#   brotlidec-static.lib
#   brotlicommon-static.lib
# That's pointless for out purposes...

add_library(Brotli STATIC)
add_library(Brotli::Static ALIAS Brotli)

if(${CMAKE_PROJECT_NAME} STREQUAL "Brotli")
	enable_target_compiler_warnings(Brotli)
else()
	disable_target_compiler_warnings(Brotli)
endif()

if(NOT WIN32)
	target_compile_definitions(
		Brotli
		PUBLIC OS_LINUX
	)
endif()

target_include_directories(
	Brotli
	PUBLIC "build/c/include"
)

target_sources(
	Brotli
	PUBLIC ${headerFiles}
	PRIVATE ${sourceFiles}
)

#set_target_properties(Brotli PROPERTIES PREFIX "")
set_target_properties(Brotli PROPERTIES OUTPUT_NAME "brotli")

# -------------------------------------------------------------------------------------------------

install(
	FILES ${headerFiles}
	DESTINATION ${PROJECT_SOURCE_DIR}/Include
)

install(
	TARGETS Brotli
	ARCHIVE
	DESTINATION ${PROJECT_SOURCE_DIR}/bin/${CMAKE_COMPILER_TAG}
)

# -------------------------------------------------------------------------------------------------

file(
	WRITE "${PROJECT_SOURCE_DIR}/BrotliConfig.cmake"
	"#!/usr/bin/cmake

# Configuration to include Brotli in a CMake-based project. If you want to
# reference Brotli as an externally compiled static library, do this:
#
#    set(Brotli_DIR \"../ThirdParty/brotli\")
#    find_package(Brotli REQUIRED CONFIG)
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE Brotli::Static
#    )
#
# Alternatively, if you want to build Brotli together with your project,
# use the normal CMakeLists.txt with CMake's add_subdirectory() command:
#
#    add_subdirectory(
#        \"\${PROJECT_SOURCE_DIR}/../ThirdParty/brotli\"
#        \"\${CMAKE_BINARY_DIR}/brotli\"
#    )
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE Brotli
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

add_library(Brotli::Static STATIC IMPORTED)

# This may cause warnings on recent GCC versions (10.0.0+?) with LTO because GCC detects
# that the headers used during build (residing in build/) are not the same used when
# linking the library (copies resising in Include/).
#
# CMake doesn't run the install step during build, so the only way to get the headers
# in place before building would be by copying them rather than installing them.
set_target_properties(
	Brotli::Static PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES \"\${CMAKE_CURRENT_LIST_DIR}/Include\"
	IMPORTED_LINK_INTERFACE_LANGUAGES \"C\"
)

if(WIN32)
	set_target_properties(
		Brotli::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/brotli.lib\"
		COMPILE_DEFINITIONS OS_LINUX
	)
else()
	set_target_properties(
		Brotli::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/libbrotli.a\"
	)
endif()

message(STATUS \"Imported Brotli targets with binaries in '\${CMAKE_CURRENT_LIST_DIR}'\")"
)

# -------------------------------------------------------------------------------------------------
