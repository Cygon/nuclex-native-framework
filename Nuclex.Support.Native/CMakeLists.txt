#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.8)

# -------------------------------------------------------------------------------------------------

project(
	NuclexSupportNative
	VERSION 1.0.0
	DESCRIPTION "Utility functions and basic interface definitions"
)

include("../BuildSystem/cmake/cplusplus.cmake")

set(BUILD_DOCS OFF CACHE BOOL "Whether to generate documentation via Doxygen")

# -------------------------------------------------------------------------------------------------

add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/gtest ${CMAKE_BINARY_DIR}/gtest)

# Alternative for precompiled GoogleTest library would be
#set(GoogleTest_DIR "../ThirdParty/gtest")
#find_package(GoogleTest REQUIRED CONFIG)

find_package(Threads REQUIRED)

if(BUILD_DOCS)
	find_package(Doxygen)
endif()

# -------------------------------------------------------------------------------------------------

# "Note: We do not recommend using GLOB to collect a list of source files from your source tree.
# If no CMakeLists.txt file changes when a source is added or removed then the generated
# build system cannot know when to ask CMake to regenerate."
#
# What are you f-ing smoking over there?
# Why would I even use this turd of a Makefile generator if I have to list my source files by hand?

file(
	GLOB_RECURSE sourceFiles
	CONFIGURE_DEPENDS
	"Source/*.cpp"
	"Source/*.c"
)
file(
	GLOB_RECURSE headerFiles
	CONFIGURE_DEPENDS
	"Include/Nuclex/Support/*.h"
)

file(
	GLOB_RECURSE unittestFiles
	CONFIGURE_DEPENDS
	"Tests/*.cpp"
)

# -------------------------------------------------------------------------------------------------

add_library(NuclexSupportNative SHARED)

# Custom function in cplusplus.cmake that turns on all compiler warnings
enable_target_compiler_warnings(NuclexSupportNative)

target_include_directories(
	NuclexSupportNative
	PUBLIC "Include"
)

target_sources(
	NuclexSupportNative
	PUBLIC ${headerFiles}
	PRIVATE ${sourceFiles}
)

if(WIN32)
	set_target_properties(NuclexSupportNative PROPERTIES OUTPUT_NAME "Nuclex.Support.Native")
endif()

# -------------------------------------------------------------------------------------------------

add_executable(NuclexSupportNativeTests)

# Custom function in cplusplus.cmake that turns on all compiler warnings
enable_target_compiler_warnings(NuclexSupportNativeTests)

target_compile_definitions(
	NuclexSupportNativeTests
	PRIVATE NUCLEX_SUPPORT_EXECUTABLE
)

target_include_directories(
	NuclexSupportNativeTests
	PUBLIC "Include"
)

target_sources(
	NuclexSupportNativeTests
	PRIVATE ${headerFiles}
	PRIVATE ${sourceFiles}
	PRIVATE ${unittestFiles}
)

target_link_libraries(
	NuclexSupportNativeTests
	PRIVATE GoogleTest
	PRIVATE GoogleTestMain
	PRIVATE Threads::Threads
)

if(WIN32)
	set_target_properties(
		NuclexSupportNativeTests PROPERTIES OUTPUT_NAME "Nuclex.Support.Native.Tests"
	)
endif()

# -------------------------------------------------------------------------------------------------

install(
	TARGETS NuclexSupportNative
	ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

install(
	TARGETS NuclexSupportNativeTests
	RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

# -------------------------------------------------------------------------------------------------

if(BUILD_DOCS)

	if(NOT DOXYGEN_FOUND)
		message(FATAL_ERROR "Can't build documentation because Doxygen was not found")
	endif()

	add_custom_target(
		NuclexSupportNativeDocs ALL
		COMMAND ${DOXYGEN_EXECUTABLE} "Nuclex.Support.Native.cfg"
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	)

endif()

# -------------------------------------------------------------------------------------------------

file(
	WRITE "${PROJECT_SOURCE_DIR}/NuclexSupportNativeConfig.cmake"
	"#!/usr/bin/cmake

# Configuration to include Nuclex.Support.Native in a CMake-based project. If you want to
# reference Nuclex.Support.Native as an externally compiled static library, do this:
#
#    set(NuclexSupportNative_DIR \"../Nuclex.Support.Native\")
#    find_package(NuclexSupportNative REQUIRED CONFIG)
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE NuclexSupportNative::Dynamic
#    )
#
# Alternatively, if you want to build Nuclex.Support.Native together with your project,
# use the normal CMakeLists.txt with CMake's add_subdirectory() command:
#
#    add_subdirectory(
#        \"\${PROJECT_SOURCE_DIR}/../Nuclex.Support.Native\"
#        \"\${CMAKE_BINARY_DIR}/nuclex.support.native\"
#    )
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE NuclexSupportNative
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

add_library(NuclexSupportNative::Dynamic SHARED IMPORTED)

# This may cause warnings on recent GCC versions (10.0.0+?) with LTO because GCC detects
# that the headers used during build (residing in build/) are not the same used when
# linking the library (copies resising in Include/).
#
# CMake doesn't run the install step during build, so the only way to get the headers
# in place before building would be by copying them rather than installing them.
set_target_properties(
	NuclexSupportNative::Dynamic PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES \"\${CMAKE_CURRENT_LIST_DIR}/Include\"
	IMPORTED_LINK_INTERFACE_LANGUAGES \"C\"
)

if(WIN32)
	set_target_properties(
		NuclexSupportNative::Dynamic PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/Nuclex.Support.Native.lib\"
	)
else()
	set_target_properties(
		NuclexSupportNative::Dynamic PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/libNuclexSupportNative.so\"
	)
endif()

message(STATUS \"Imported Nuclex.Support.Native targets with binaries in '\${CMAKE_CURRENT_LIST_DIR}'\")"
)

# -------------------------------------------------------------------------------------------------
