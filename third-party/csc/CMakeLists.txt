#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.18)

# -------------------------------------------------------------------------------------------------

project(
	Csc
	VERSION 2016.10.13
	DESCRIPTION "Experimental compression algorithm achieving high compression ratios"
)

include("../../build-system/cmake/cplusplus.cmake")

set(BUILD_CMD OFF CACHE BOOL "Whether to build a command-line csc-cmd tool")

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/downloads/csc-2016-10-13.tar.gz)
	message(STATUS "Downloading Csc sources")
	file(
		DOWNLOAD https://github.com/fusiyuan2010/CSC/archive/c5dbe0944d07acbc97d2c04ec9f99a139c6f3931.zip
		${PROJECT_SOURCE_DIR}/downloads/csc-2016-10-13.tar.gz
		SHOW_PROGRESS
		EXPECTED_HASH SHA256=9c8fda0316b3c95cfdd24f7834b86bcc17c99ca2a25233d2db5516fe170becaa
	)
endif()

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/build)
	message(STATUS "Extracting Csc sources")
	file(
		ARCHIVE_EXTRACT
		INPUT ${PROJECT_SOURCE_DIR}/downloads/csc-2016-10-13.tar.gz
		DESTINATION ${CMAKE_BINARY_DIR}/extract
	)
	file(
		RENAME
		${CMAKE_BINARY_DIR}/extract/CSC-c5dbe0944d07acbc97d2c04ec9f99a139c6f3931
		${PROJECT_SOURCE_DIR}/build
	)
endif()

# -------------------------------------------------------------------------------------------------

set(
	sourceFiles
	"build/src/libcsc/csc_analyzer.cpp"
	"build/src/libcsc/csc_coder.cpp"
	"build/src/libcsc/csc_dec.cpp"
	"build/src/libcsc/csc_default_alloc.cpp"
	"build/src/libcsc/csc_enc.cpp"
	"build/src/libcsc/csc_encoder_main.cpp"
	"build/src/libcsc/csc_filters.cpp"
	"build/src/libcsc/csc_lz.cpp"
	"build/src/libcsc/csc_memio.cpp"
	"build/src/libcsc/csc_mf.cpp"
	"build/src/libcsc/csc_model.cpp"
	"build/src/libcsc/csc_profiler.cpp"
)
set(
	headerFiles
	"build/src/archiver/csa_adler32.h"
	"build/src/archiver/csa_common.h"
	"build/src/archiver/csa_file.h"
	"build/src/archiver/csa_indexpack.h"
	"build/src/archiver/csa_io.h"
	"build/src/archiver/csa_progress.h"
	"build/src/archiver/csa_thread.h"
	"build/src/archiver/csa_typedef.h"
	"build/src/archiver/csa_worker.h"
	"build/src/libcsc/csc_analyzer.h"
	"build/src/libcsc/csc_coder.h"
	"build/src/libcsc/csc_common.h"
	"build/src/libcsc/csc_dec.h"
	"build/src/libcsc/csc_default_alloc.h"
	"build/src/libcsc/csc_enc.h"
	"build/src/libcsc/csc_encoder_main.h"
	"build/src/libcsc/csc_filters.h"
	"build/src/libcsc/csc_lz.h"
	"build/src/libcsc/csc_memio.h"
	"build/src/libcsc/csc_mf.h"
	"build/src/libcsc/csc_model.h"
	"build/src/libcsc/csc_profiler.h"
	"build/src/libcsc/csc_typedef.h"
	"build/src/libcsc/Types.h"
)
set(
	executableFiles
	"build/src/archiver/csarc.cpp"
	"build/src/archiver/csa_adler32.cpp"
	"build/src/archiver/csa_common.cpp"
	"build/src/archiver/csa_file.cpp"
	"build/src/archiver/csa_indexpack.cpp"
	"build/src/archiver/csa_progress.cpp"
	"build/src/archiver/csa_worker.cpp"
)

#	"build/src/archiver/csa_adler32.cpp"
#	"build/src/archiver/csa_indexpack.cpp"

# -------------------------------------------------------------------------------------------------

add_library(Csc STATIC)
add_library(Csc::Static ALIAS Csc)

target_compile_definitions(
	Csc
	PUBLIC _7Z_TYPES_
)

target_include_directories(
	Csc
	PUBLIC "build/src/libcsc"
	PUBLIC "build/src/archiver"
)

target_sources(
	Csc
	PUBLIC ${headerFiles}
	PRIVATE ${sourceFiles}
)

#set_target_properties(Csc PROPERTIES PREFIX "")
set_target_properties(Csc PROPERTIES OUTPUT_NAME "csc")

# -------------------------------------------------------------------------------------------------

if(BUILD_CMD)

	add_executable(Csc-cmd)

	target_compile_definitions(
		Csc-cmd
		PUBLIC _7Z_TYPES_
	)

	target_include_directories(
		Csc-cmd
		PUBLIC "build/src/libcsc"
		PUBLIC "build/src/archiver"
	)

	target_sources(
		Csc-cmd
		PUBLIC ${headerFiles}
		PRIVATE ${sourceFiles}
		#PRIVATE ${executableFiles}
		PRIVATE "build/src/libcsc/csc.cpp"
	)

	#set_target_properties(Csc-cmd PROPERTIES PREFIX "")
	set_target_properties(Csc-cmd PROPERTIES OUTPUT_NAME "csc-cmd")

endif()

# -------------------------------------------------------------------------------------------------

install(
	FILES ${headerFiles}
	DESTINATION ${PROJECT_SOURCE_DIR}/Include
)

install(
	TARGETS Csc
	ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/bin/${CMAKE_COMPILER_TAG}
)

if(BUILD_CMD)

	install(
		TARGETS Csc-cmd
		RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${CMAKE_COMPILER_TAG}
	)

endif()

# -------------------------------------------------------------------------------------------------

file(
	WRITE "${PROJECT_SOURCE_DIR}/CscConfig.cmake"
	"#!/usr/bin/cmake

# Configuration to include Csc in a CMake-based project. If you want to
# reference Csc as an externally compiled static library, do this:
#
#    set(Csc_DIR \"../ThirdParty/csc\")
#    find_package(Csc REQUIRED CONFIG)
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE Csc::Static
#    )
#
# Alternatively, if you want to build Csc together with your project,
# use the normal CMakeLists.txt with CMake's add_subdirectory() command:
#
#    add_subdirectory(
#        \"\${PROJECT_SOURCE_DIR}/../ThirdParty/csc\"
#        \"\${CMAKE_BINARY_DIR}/csc\"
#    )
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE Csc
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

add_library(Csc::Static STATIC IMPORTED)

# This may cause warnings on recent GCC versions (10.0.0+?) with LTO because GCC detects
# that the headers used during build (residing in build/) are not the same used when
# linking the library (copies resising in Include/).
#
# CMake doesn't run the install step during build, so the only way to get the headers
# in place before building would be by copying them rather than installing them.
set_target_properties(
	Csc::Static PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES \"\${CMAKE_CURRENT_LIST_DIR}/Include\"
	IMPORTED_LINK_INTERFACE_LANGUAGES \"C\"
)

if(WIN32)
	set_target_properties(
		Csc::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/csc.lib\"
	)
else()
	set_target_properties(
		Csc::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/libcsc.a\"
	)
endif()

message(STATUS \"Imported Csc targets with binaries in '\${CMAKE_CURRENT_LIST_DIR}'\")"
)

# -------------------------------------------------------------------------------------------------
