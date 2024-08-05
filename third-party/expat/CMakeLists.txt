#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.18)

# -------------------------------------------------------------------------------------------------

project(
	Expat
	VERSION 2.2.9
	DESCRIPTION "Stream-oriented (SAX-like) XML push parser"
)

include("../../build-system/cmake/cplusplus.cmake")

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/downloads/expat-2.2.9.tar.gz)
	message(STATUS "Downloading Expat sources")
	file(
		DOWNLOAD https://github.com/libexpat/libexpat/releases/download/R_2_2_9/expat-2.2.9.tar.gz
		${PROJECT_SOURCE_DIR}/downloads/expat-2.2.9.tar.gz
		SHOW_PROGRESS
		EXPECTED_HASH SHA256=4456e0aa72ecc7e1d4b3368cd545a5eec7f9de5133a8dc37fdb1efa6174c4947
	)
endif()

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/build)
	message(STATUS "Extracting Expat sources")
	file(
		ARCHIVE_EXTRACT
		INPUT ${PROJECT_SOURCE_DIR}/downloads/expat-2.2.9.tar.gz
		DESTINATION ${CMAKE_BINARY_DIR}/extract
	)
	file(
		RENAME
		${CMAKE_BINARY_DIR}/extract/expat-2.2.9
		${PROJECT_SOURCE_DIR}/build
	)
endif()

# -------------------------------------------------------------------------------------------------

set(
	sourceFiles
	"build/lib/xmlparse.c"
	"build/lib/xmlrole.c"
	"build/lib/xmltok.c"
	"build/lib/xmltok_impl.c"
	"build/lib/xmltok_ns.c"
)
set(
	headerFiles
	"build/lib/ascii.h"
	"build/lib/asciitab.h"
	"build/lib/expat.h"
	"build/lib/expat_external.h"
	"build/lib/iasciitab.h"
	"build/lib/internal.h"
	"build/lib/latin1tab.h"
	"build/lib/nametab.h"
	"build/lib/siphash.h"
	"build/lib/utf8tab.h"
	"build/lib/winconfig.h"
	"build/lib/xmlrole.h"
	"build/lib/xmltok.h"
	"build/lib/xmltok_impl.h"
)

# -------------------------------------------------------------------------------------------------

add_library(Expat STATIC)
add_library(Expat::Static ALIAS Expat)

target_compile_definitions(
	Expat
	PUBLIC XML_POOR_ENTROPY
	PUBLIC XML_STATIC
)

target_include_directories(
	Expat
	PUBLIC "build/lib"
)

target_sources(
	Expat
	PUBLIC ${headerFiles}
	PRIVATE ${sourceFiles}
)

#set_target_properties(Expat PROPERTIES PREFIX "")
#set_target_properties(Expat PROPERTIES OUTPUT_NAME "expat")

# -------------------------------------------------------------------------------------------------

install(
	FILES ${headerFiles}
	DESTINATION ${PROJECT_SOURCE_DIR}/Include
)

install(
	TARGETS Expat
	ARCHIVE	DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

install_debug_symbols(Expat)

# -------------------------------------------------------------------------------------------------

file(
	WRITE "${PROJECT_SOURCE_DIR}/ExpatConfig.cmake"
	"#!/usr/bin/cmake

# Configuration to include Expat in a CMake-based project. If you want to
# reference Expat as an externally compiled static library, do this:
#
#    set(Expat_DIR \"../ThirdParty/expat\")
#    find_package(Expat REQUIRED CONFIG)
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE Expat::Static
#    )
#
# Alternatively, if you want to build Expat together with your project,
# use the normal CMakeLists.txt with CMake's add_subdirectory() command:
#
#    add_subdirectory(
#        \"\${PROJECT_SOURCE_DIR}/../ThirdParty/expat\"
#        \"\${CMAKE_BINARY_DIR}/expat\"
#    )
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE Expat
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

add_library(Expat::Static STATIC IMPORTED)

# This may cause warnings on recent GCC versions (10.0.0+?) with LTO because GCC detects
# that the headers used during build (residing in build/) are not the same used when
# linking the library (copies resising in Include/).
#
# CMake doesn't run the install step during build, so the only way to get the headers
# in place before building would be by copying them rather than installing them.
set_target_properties(
	Expat::Static PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES \"\${CMAKE_CURRENT_LIST_DIR}/Include\"
	IMPORTED_LINK_INTERFACE_LANGUAGES \"C\"
)

if(WIN32)
	set_target_properties(
		Expat::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/expat.lib\"
	)
else()
	set_target_properties(
		Expat::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/libexpat.a\"
	)
endif()

message(STATUS \"Imported Expat targets with binaries in '\${CMAKE_CURRENT_LIST_DIR}'\")"
)

# -------------------------------------------------------------------------------------------------
