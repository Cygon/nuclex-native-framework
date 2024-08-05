#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.18)

# -------------------------------------------------------------------------------------------------

project(
	LibTIFF
	VERSION 4.2.0
	DESCRIPTION "Older file format still used in place of PNG by some applications"
)

include("../../build-system/cmake/cplusplus.cmake")

if(NOT (TARGET LibJPEG))
	add_subdirectory(${PROJECT_SOURCE_DIR}/../libjpeg ${CMAKE_BINARY_DIR}/libjpeg)
endif()

if(NOT (TARGET ZLib))
	add_subdirectory(${PROJECT_SOURCE_DIR}/../zlib ${CMAKE_BINARY_DIR}/zlib)
endif()

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/downloads/tiff-4.2.0.tar.gz)
	message(STATUS "Downloading LibTIFF sources")
	file(
		DOWNLOAD https://download.osgeo.org/libtiff/tiff-4.2.0.tar.gz
		${PROJECT_SOURCE_DIR}/downloads/tiff-4.2.0.tar.gz
		SHOW_PROGRESS
		EXPECTED_HASH SHA256=eb0484e568ead8fa23b513e9b0041df7e327f4ee2d22db5a533929dfc19633cb
	)
endif()

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/build)
	message(STATUS "Extracting LibTIFF sources")
	file(
		ARCHIVE_EXTRACT
		INPUT ${PROJECT_SOURCE_DIR}/downloads/tiff-4.2.0.tar.gz
		DESTINATION ${CMAKE_BINARY_DIR}/extract
	)
	file(
		RENAME
		${CMAKE_BINARY_DIR}/extract/tiff-4.2.0
		${PROJECT_SOURCE_DIR}/build
	)
endif()

file(
	COPY "${PROJECT_SOURCE_DIR}/tif_config.h"
	DESTINATION "${PROJECT_SOURCE_DIR}/build/libtiff"
)
file(
	COPY "${PROJECT_SOURCE_DIR}/tiffconf.h"
	DESTINATION "${PROJECT_SOURCE_DIR}/build/libtiff"
)

# -------------------------------------------------------------------------------------------------

set(
	sourceFiles
	"build/libtiff/tif_aux.c"
	"build/libtiff/tif_close.c"
	"build/libtiff/tif_codec.c"
	"build/libtiff/tif_color.c"
	"build/libtiff/tif_compress.c"
	"build/libtiff/tif_dir.c"
	"build/libtiff/tif_dirinfo.c"
	"build/libtiff/tif_dirread.c"
	"build/libtiff/tif_dirwrite.c"
	"build/libtiff/tif_dumpmode.c"
	"build/libtiff/tif_error.c"
	"build/libtiff/tif_extension.c"
	"build/libtiff/tif_fax3.c"
	"build/libtiff/tif_fax3sm.c"
	"build/libtiff/tif_flush.c"
	"build/libtiff/tif_getimage.c"
	#"build/libtiff/tif_jbig.c"
	#"build/libtiff/tif_jpeg_12.c"
	"build/libtiff/tif_jpeg.c"
	"build/libtiff/tif_luv.c"
	#"build/libtiff/tif_lzma.c"
	"build/libtiff/tif_lzw.c"
	"build/libtiff/tif_next.c"
	#"build/libtiff/tif_ojpeg.c"
	"build/libtiff/tif_open.c"
	"build/libtiff/tif_packbits.c"
	"build/libtiff/tif_pixarlog.c"
	"build/libtiff/tif_predict.c"
	"build/libtiff/tif_print.c"
	"build/libtiff/tif_read.c"
	#"build/libtiff/tif_stream.cxx"
	"build/libtiff/tif_strip.c"
	"build/libtiff/tif_swab.c"
	"build/libtiff/tif_thunder.c"
	"build/libtiff/tif_tile.c"
	"build/libtiff/tif_version.c"
	"build/libtiff/tif_warning.c"
	#"build/libtiff/tif_webp.c"
	"build/libtiff/tif_write.c"
	"build/libtiff/tif_zip.c"
	#"build/libtiff/tif_zstd.c"
)

if(WIN32)
	list(APPEND sourceFiles "build/libtiff/tif_win32.c")
else()
	list(APPEND sourceFiles "build/libtiff/tif_unix.c")
endif()

set(
	headerFiles
	#"build/libtiff/t4.h"
	"build/libtiff/tif_config.h"
	"build/libtiff/tif_dir.h"
	#"build/libtiff/tif_fax3.h"
	"build/libtiff/tiffconf.h"
	"build/libtiff/tiff.h"
	"build/libtiff/tiffio.h"
	#"build/libtiff/tiffiop.h"
	"build/libtiff/tiffvers.h"
	#"build/libtiff/uvcode.h"
)

# -------------------------------------------------------------------------------------------------

add_library(LibTIFF STATIC)
add_library(LibTIFF::Static ALIAS LibTIFF)

if(${CMAKE_PROJECT_NAME} STREQUAL "LibTIFF")
	enable_target_compiler_warnings(LibTIFF)
else()
	disable_target_compiler_warnings(LibTIFF)
endif()

target_link_libraries(
	LibTIFF
	PRIVATE ZLib::Static
	PRIVATE LibJPEG::Static
)

target_include_directories(
	LibTIFF
	PUBLIC "build"
)

target_sources(
	LibTIFF
	PUBLIC ${headerFiles}
	PRIVATE ${sourceFiles}
)

#set_target_properties(LibTIFF PROPERTIES PREFIX "")
set_target_properties(LibTIFF PROPERTIES OUTPUT_NAME "tiff")

# -------------------------------------------------------------------------------------------------

install(
	DIRECTORY build/libtiff
	DESTINATION ${PROJECT_SOURCE_DIR}/Include
	FILES_MATCHING PATTERN "*.h"
)
install(
	TARGETS LibTIFF
	ARCHIVE
	DESTINATION ${PROJECT_SOURCE_DIR}/bin/${CMAKE_COMPILER_TAG}
)

# -------------------------------------------------------------------------------------------------

file(
	WRITE "${PROJECT_SOURCE_DIR}/LibTIFFConfig.cmake"
	"#!/usr/bin/cmake

# Configuration to include LibTIFF in a CMake-based project. If you want to
# reference LibTIFF as an externally compiled static library, do this:
#
#    set(LibTIFF_DIR \"../ThirdParty/libtiff\")
#    find_package(LibTIFF REQUIRED CONFIG)
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE LibTIFF::Static
#    )
#
# Alternatively, if you want to build LibTIFF together with your project,
# use the normal CMakeLists.txt with CMake's add_subdirectory() command:
#
#    add_subdirectory(
#        \"\${PROJECT_SOURCE_DIR}/../ThirdParty/libtiff\"
#        \"\${CMAKE_BINARY_DIR}/libtiff\"
#    )
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE LibTIFF
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

add_library(LibTIFF::Static STATIC IMPORTED)

# This may cause warnings on recent GCC versions (10.0.0+?) with LTO because GCC detects
# that the headers used during build (residing in build/) are not the same used when
# linking the library (copies resising in Include/).
#
# CMake doesn't run the install step during build, so the only way to get the headers
# in place before building would be by copying them rather than installing them.
set_target_properties(
	LibTIFF::Static PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES \"\${CMAKE_CURRENT_LIST_DIR}/Include\"
	IMPORTED_LINK_INTERFACE_LANGUAGES \"C\"
)

if(WIN32)
	set_target_properties(
		LibTIFF::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/libtiff.lib\"
		COMPILE_DEFINITIONS OS_LINUX
	)
else()
	set_target_properties(
		LibTIFF::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/libtiff.a\"
	)
endif()

message(STATUS \"Imported LibTIFF targets with binaries in '\${CMAKE_CURRENT_LIST_DIR}'\")"
)

# -------------------------------------------------------------------------------------------------
