#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.18)

# -------------------------------------------------------------------------------------------------

project(
	LibJPEG
	VERSION 0.9.13
	DESCRIPTION "Standard JPEG loading and saving library"
)

include("../../BuildSystem/cmake/cplusplus.cmake")

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/downloads/jpegsrc.v9c.tar.gz)
	message(STATUS "Downloading LibJPEG sources")
	file(
		DOWNLOAD http://ijg.org/files/jpegsrc.v9c.tar.gz
		${PROJECT_SOURCE_DIR}/downloads/jpegsrc.v9c.tar.gz
		SHOW_PROGRESS
		EXPECTED_HASH SHA256=1e9793e1c6ba66e7e0b6e5fe7fd0f9e935cc697854d5737adec54d93e5b3f730
	)
endif()

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/build)
	message(STATUS "Extracting LibJPEG sources")
	file(
		ARCHIVE_EXTRACT
		INPUT ${PROJECT_SOURCE_DIR}/downloads/jpegsrc.v9c.tar.gz
		DESTINATION ${CMAKE_BINARY_DIR}/extract
	)
	file(
		RENAME
		${CMAKE_BINARY_DIR}/extract/jpeg-9c
		${PROJECT_SOURCE_DIR}/build
	)
endif()

file(
	COPY "${PROJECT_SOURCE_DIR}/jconfig.h"
	DESTINATION "${PROJECT_SOURCE_DIR}/build/"
)

# -------------------------------------------------------------------------------------------------

set(
	sourceFiles
	"build/jquant2.c"
	"build/jdpostct.c"
	"build/jcapistd.c"
	"build/jddctmgr.c"
	"build/wrbmp.c"
	"build/jcarith.c"
	"build/wrgif.c"
	"build/jdinput.c"
	"build/jcmaster.c"
	"build/jcmarker.c"
	"build/jquant1.c"
	"build/rdgif.c"
	"build/jdtrans.c"
	"build/jfdctflt.c"
	"build/jdcoefct.c"
	"build/jchuff.c"
	"build/jidctflt.c"
	"build/wrppm.c"
	"build/jdarith.c"
	"build/rdbmp.c"
	"build/jcinit.c"
	"build/jcomapi.c"
	"build/rdppm.c"
	"build/jdmarker.c"
	"build/jmemmgr.c"
	"build/jdsample.c"
	"build/rdcolmap.c"
	"build/jdapistd.c"
	"build/jfdctfst.c"
	"build/jccolor.c"
	"build/jcparam.c"
	"build/transupp.c"
	"build/jidctint.c"
	"build/jfdctint.c"
	"build/jctrans.c"
	"build/jidctfst.c"
	"build/jutils.c"
	"build/jcprepct.c"
	"build/rdtarga.c"
	"build/wrtarga.c"
	"build/jdmainct.c"
	"build/jdmaster.c"
	"build/jmemnobs.c"
	"build/jdatadst.c"
	"build/jaricom.c"
	"build/rdswitch.c"
	"build/rdrle.c"
	"build/jccoefct.c"
	"build/jcdctmgr.c"
	"build/jcsample.c"
	"build/jdhuff.c"
	"build/jdapimin.c"
	"build/jdatasrc.c"
	"build/wrrle.c"
	"build/jdmerge.c"
	"build/jcapimin.c"
	"build/jdcolor.c"
	"build/jerror.c"
	"build/jcmainct.c"
)
set(
	headerFiles
	"build/transupp.h"
	"build/jconfig.h"
	"build/jmorecfg.h"
	"build/cderror.h"
	"build/jpegint.h"
	"build/jinclude.h"
	"build/jdct.h"
	"build/jmemsys.h"
	"build/jpeglib.h"
	"build/jerror.h"
	"build/cdjpeg.h"
	"build/jversion.h"
)

# -------------------------------------------------------------------------------------------------

add_library(LibJPEG STATIC)
add_library(LibJPEG::Static ALIAS LibJPEG)

if(${CMAKE_PROJECT_NAME} STREQUAL "LibJPEG")
	enable_target_compiler_warnings(LibJPEG)
else()
	disable_target_compiler_warnings(LibJPEG)
endif()

if(NOT WIN32)
	target_compile_options(LibJPEG PRIVATE -fexceptions)
endif()

target_include_directories(
	LibJPEG
	PUBLIC "build"
)

target_sources(
	LibJPEG
	PUBLIC ${headerFiles}
	PRIVATE ${sourceFiles}
)

#set_target_properties(LibJPEG PROPERTIES PREFIX "")
set_target_properties(LibJPEG PROPERTIES OUTPUT_NAME "jpeg")

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
	TARGETS LibJPEG
	ARCHIVE
	DESTINATION ${PROJECT_SOURCE_DIR}/bin/${CMAKE_COMPILER_TAG}
)

# -------------------------------------------------------------------------------------------------

file(
	WRITE "${PROJECT_SOURCE_DIR}/LibJPEGConfig.cmake"
	"#!/usr/bin/cmake

# Configuration to include LibJPEG in a CMake-based project. If you want to
# reference LibJPEG as an externally compiled static library, do this:
#
#    set(LibJPEG_DIR \"../ThirdParty/libjpeg\")
#    find_package(LibJPEG REQUIRED CONFIG)
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE LibJPEG::Static
#    )
#
# Alternatively, if you want to build LibJPEG together with your project,
# use the normal CMakeLists.txt with CMake's add_subdirectory() command:
#
#    add_subdirectory(
#        \"\${PROJECT_SOURCE_DIR}/../ThirdParty/libjpeg\"
#        \"\${CMAKE_BINARY_DIR}/libjpeg\"
#    )
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE LibJPEG
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

add_library(LibJPEG::Static STATIC IMPORTED)

# This may cause warnings on recent GCC versions (10.0.0+?) with LTO because GCC detects
# that the headers used during build (residing in build/) are not the same used when
# linking the library (copies resising in Include/).
#
# CMake doesn't run the install step during build, so the only way to get the headers
# in place before building would be by copying them rather than installing them.
set_target_properties(
	LibJPEG::Static PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES \"\${CMAKE_CURRENT_LIST_DIR}/Include\"
	IMPORTED_LINK_INTERFACE_LANGUAGES \"C\"
)

if(WIN32)
	set_target_properties(
		LibJPEG::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/jpeg.lib\"
		COMPILE_DEFINITIONS OS_LINUX
	)
else()
	set_target_properties(
		LibJPEG::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/libjpeg.a\"
	)
endif()

message(STATUS \"Imported LibJPEG targets with binaries in '\${CMAKE_CURRENT_LIST_DIR}'\")"
)

# -------------------------------------------------------------------------------------------------
