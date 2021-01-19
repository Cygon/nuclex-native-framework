#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.18)

# -------------------------------------------------------------------------------------------------

project(
	GoogleTest
	VERSION 1.8.1
	DESCRIPTION "Framework to write unit tests with"
)

include("../../BuildSystem/cmake/cplusplus.cmake")

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/downloads/release-1.8.1.tar.gz)
	message(STATUS "Downloading GoogleTest sources")
	file(
		DOWNLOAD https://github.com/google/googletest/archive/release-1.8.1.tar.gz
		${PROJECT_SOURCE_DIR}/downloads/release-1.8.1.tar.gz
		SHOW_PROGRESS
		EXPECTED_HASH SHA256=9bf1fe5182a604b4135edc1a425ae356c9ad15e9b23f9f12a02e80184c3a249c
	)
endif()

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/build)
	message(STATUS "Extracting GoogleTest sources")
	file(
		ARCHIVE_EXTRACT
		INPUT ${PROJECT_SOURCE_DIR}/downloads/release-1.8.1.tar.gz
		DESTINATION ${CMAKE_BINARY_DIR}/extract
	)
	file(
		RENAME
		${CMAKE_BINARY_DIR}/extract/googletest-release-1.8.1
		${PROJECT_SOURCE_DIR}/build
	)
endif()

# -------------------------------------------------------------------------------------------------

set(
	sourceFiles
	"build/googletest/src/gtest.cc"
	"build/googletest/src/gtest-death-test.cc"
	"build/googletest/src/gtest-filepath.cc"
	"build/googletest/src/gtest-port.cc"
	"build/googletest/src/gtest-printers.cc"
	"build/googletest/src/gtest-test-part.cc"
	"build/googletest/src/gtest-typed-test.cc"
)
file(
	GLOB_RECURSE headerFiles
	CONFIGURE_DEPENDS
	"build/googletest/include/*.h"
)

# -------------------------------------------------------------------------------------------------

add_library(GoogleTest STATIC)
add_library(GoogleTest::Static ALIAS GoogleTest)

target_include_directories(
	GoogleTest
	PUBLIC "build/googletest/include"
	PRIVATE "build/googletest"
)

target_sources(
	GoogleTest
	PUBLIC ${headerFiles}
	PRIVATE ${sourceFiles}
)

#set_target_properties(GoogleTest PROPERTIES PREFIX "")
set_target_properties(GoogleTest PROPERTIES OUTPUT_NAME "gtest")

# -------------------------------------------------------------------------------------------------

add_library(GoogleTestMain STATIC)
add_library(GoogleTest::Main ALIAS GoogleTestMain)

target_include_directories(
	GoogleTestMain
	PUBLIC "build/googletest/include"
	PRIVATE "build/googletest"
)

target_sources(
	GoogleTestMain
	PRIVATE ${headerFiles}
	PRIVATE "build/googletest/src/gtest_main.cc"
)

#set_target_properties(GoogleTestMain PROPERTIES PREFIX "")
set_target_properties(GoogleTestMain PROPERTIES OUTPUT_NAME "gtest_main")

# -------------------------------------------------------------------------------------------------

install(
	FILES ${headerFiles}
	DESTINATION ${PROJECT_SOURCE_DIR}/Include
)

install(
	TARGETS GoogleTest
	ARCHIVE	DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

install(
	TARGETS GoogleTestMain
	ARCHIVE	DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

# -------------------------------------------------------------------------------------------------

file(
	WRITE "${PROJECT_SOURCE_DIR}/GoogleTestConfig.cmake"
	"#!/usr/bin/cmake

# Configuration to include GoogleTest in a CMake-based project. If you want to
# reference GoogleTest as an externally compiled static library, do this:
#
#    set(GoogleTest_DIR \"../ThirdParty/gtest\")
#    find_package(GoogleTest REQUIRED CONFIG)
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE GTest::Static
#        PRIVATE GTest::Main
#    )
#
# Alternatively, if you want to build GoogleTest together with your project,
# use the normal CMakeLists.txt with CMake's add_subdirectory() command:
#
#    add_subdirectory(
#        \"\${PROJECT_SOURCE_DIR}/../ThirdParty/gtest\"
#        \"\${CMAKE_BINARY_DIR}/gtest\"
#    )
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE GoogleTest
#        PRIVATE GoogleTestMain
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

add_library(GoogleTest::Static STATIC IMPORTED)
add_library(GoogleTest::Main STATIC IMPORTED)

# This may cause warnings on recent GCC versions (10.0.0+?) with LTO because GCC detects
# that the headers used during build (residing in build/) are not the same used when
# linking the library (copies resising in Include/).
#
# CMake doesn't run the install step during build, so the only way to get the headers
# in place before building would be by copying them rather than installing them.
set_target_properties(
	GoogleTest::Static PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES \"\${CMAKE_CURRENT_LIST_DIR}/Include\"
	IMPORTED_LINK_INTERFACE_LANGUAGES \"CXX\"
)
set_target_properties(
	GoogleTest::Main PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES \"\${CMAKE_CURRENT_LIST_DIR}/Include\"
	IMPORTED_LINK_INTERFACE_LANGUAGES \"CXX\"
)

if(WIN32)
	set_target_properties(
		GoogleTest::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/gtest.lib\"
	)
	set_target_properties(
		GoogleTest::Main PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/gtest_main.lib\"
	)
else()
	set_target_properties(
		GoogleTest::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/libgtest.a\"
	)
	set_target_properties(
		GoogleTest::Main PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/libgtest_main.a\"
	)
endif()

message(STATUS \"Imported GoogleTest targets with binaries in '\${CMAKE_CURRENT_LIST_DIR}'\")"
)

# -------------------------------------------------------------------------------------------------
