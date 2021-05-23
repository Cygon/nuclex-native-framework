#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.18)

# -------------------------------------------------------------------------------------------------

project(
	LibWebP
	VERSION 1.2.0
	DESCRIPTION "Modern lossy image format with higher compression than JPG"
)

include("../../BuildSystem/cmake/cplusplus.cmake")

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/downloads/v1.2.0.tar.gz)
	message(STATUS "Downloading LibWebP sources")
	file(
		DOWNLOAD https://github.com/webmproject/libwebp/archive/refs/tags/v1.2.0.tar.gz
		${PROJECT_SOURCE_DIR}/downloads/v1.2.0.tar.gz
		SHOW_PROGRESS
		EXPECTED_HASH SHA256=d60608c45682fa1e5d41c3c26c199be5d0184084cd8a971a6fc54035f76487d3
	)
endif()

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/build)
	message(STATUS "Extracting LibWebP sources")
	file(
		ARCHIVE_EXTRACT
		INPUT ${PROJECT_SOURCE_DIR}/downloads/v1.2.0.tar.gz
		DESTINATION ${CMAKE_BINARY_DIR}/extract
	)
	file(
		RENAME
		${CMAKE_BINARY_DIR}/extract/libwebp-1.2.0
		${PROJECT_SOURCE_DIR}/build
	)
endif()

# -------------------------------------------------------------------------------------------------

set(
	sourceFiles
	"build/src/enc/webp_enc.c"
	"build/src/dec/io_dec.c"
	"build/src/dec/tree_dec.c"
	"build/src/dec/alpha_dec.c"
	"build/src/dec/frame_dec.c"
	"build/src/dec/quant_dec.c"
	"build/src/dec/webp_dec.c"
	"build/src/dec/vp8_dec.c"
	"build/src/dec/vp8l_dec.c"
	"build/src/dec/buffer_dec.c"
	"build/src/dsp/yuv.c"
	"build/src/dsp/yuv_sse2.c"
	"build/src/dsp/yuv_sse41.c"
	"build/src/dsp/cpu.c"
	"build/src/dsp/filters.c"
	"build/src/dsp/lossless.c"
	"build/src/dsp/rescaler.c"
	"build/src/dsp/dec.c"
	"build/src/dsp/alpha_processing.c"
	"build/src/dsp/alpha_processing_sse2.c"
	"build/src/dsp/alpha_processing_sse41.c"
	"build/src/dsp/lossless_sse2.c"
	"build/src/dsp/dec_clip_tables.c"
	"build/src/dsp/dec_sse2.c"
	"build/src/dsp/dec_sse41.c"
	"build/src/dsp/filters_sse2.c"
	"build/src/dsp/rescaler_sse2.c"
	"build/src/dsp/upsampling.c"
	"build/src/dsp/upsampling_sse2.c"
	"build/src/dsp/upsampling_sse41.c"
	"build/src/utils/random_utils.c"
	"build/src/utils/quant_levels_dec_utils.c"
	"build/src/utils/utils.c"
	"build/src/utils/color_cache_utils.c"
	"build/src/utils/bit_reader_utils.c"
	"build/src/utils/huffman_utils.c"
	"build/src/utils/rescaler_utils.c"
	"build/src/utils/thread_utils.c"
)
set(
	headerFiles
	"build/src/webp/decode.h"
	"build/src/webp/demux.h"
	"build/src/webp/encode.h"
	"build/src/webp/format_constants.h"
	"build/src/webp/mux.h"
	"build/src/webp/mux_types.h"
	"build/src/webp/types.h"
)

# -------------------------------------------------------------------------------------------------

add_library(LibWebP STATIC)
add_library(LibWebP::Static ALIAS LibWebP)

#target_compile_definitions(
#	LibWebP
#	PUBLIC XML_POOR_ENTROPY
#	PUBLIC XML_STATIC
#)

target_include_directories(
	LibWebP
	PUBLIC "build"
)

target_sources(
	LibWebP
	PUBLIC ${headerFiles}
	PRIVATE ${sourceFiles}
)

#set_target_properties(LibWebP PROPERTIES PREFIX "")
set_target_properties(LibWebP PROPERTIES OUTPUT_NAME "webp")

# -------------------------------------------------------------------------------------------------

#install(
#	FILES ${headerFiles}
#	DESTINATION ${PROJECT_SOURCE_DIR}/Include
#)
install(
	DIRECTORY build/src/webp
	DESTINATION ${PROJECT_SOURCE_DIR}/Include
	FILES_MATCHING PATTERN "*.h"
)


install(
	TARGETS LibWebP
	ARCHIVE	DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

install_debug_symbols(LibWebP)

# -------------------------------------------------------------------------------------------------

file(
	WRITE "${PROJECT_SOURCE_DIR}/LibWebPConfig.cmake"
	"#!/usr/bin/cmake

# Configuration to include LibWebP in a CMake-based project. If you want to
# reference LibWebP as an externally compiled static library, do this:
#
#    set(LibWebP_DIR \"../ThirdParty/libwebp\")
#    find_package(LibWebP REQUIRED CONFIG)
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE LibWebP::Static
#    )
#
# Alternatively, if you want to build LibWebP together with your project,
# use the normal CMakeLists.txt with CMake's add_subdirectory() command:
#
#    add_subdirectory(
#        \"\${PROJECT_SOURCE_DIR}/../ThirdParty/libwebp\"
#        \"\${CMAKE_BINARY_DIR}/libwebp\"
#    )
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE LibWebP
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

add_library(LibWebP::Static STATIC IMPORTED)

# This may cause warnings on recent GCC versions (10.0.0+?) with LTO because GCC detects
# that the headers used during build (residing in build/) are not the same used when
# linking the library (copies resising in Include/).
#
# CMake doesn't run the install step during build, so the only way to get the headers
# in place before building would be by copying them rather than installing them.
set_target_properties(
	LibWebP::Static PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES \"\${CMAKE_CURRENT_LIST_DIR}/Include\"
	IMPORTED_LINK_INTERFACE_LANGUAGES \"C\"
)

if(WIN32)
	set_target_properties(
		LibWebP::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/webp.lib\"
	)
else()
	set_target_properties(
		LibWebP::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/libwebp.a\"
	)
endif()

message(STATUS \"Imported LibWebP targets with binaries in '\${CMAKE_CURRENT_LIST_DIR}'\")"
)

# -------------------------------------------------------------------------------------------------
