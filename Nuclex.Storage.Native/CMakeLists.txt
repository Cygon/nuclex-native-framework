#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.8)

# -------------------------------------------------------------------------------------------------

project(
	NuclexStorageNative
	VERSION 1.0.0
	DESCRIPTION "Virtual file system, serialization and compression utlities"
)

include("../build-system/cmake/cplusplus.cmake")

set(BUILD_DOCS OFF CACHE BOOL "Whether to generate documentation via Doxygen")
set(ENABLE_COVERAGE OFF CACHE BOOL "Whether to instrument binaries for code coverage metrics")

set(WANT_BROTLI ON CACHE BOOL "Whether to integrate Brotli compression")
set(WANT_CSC OFF CACHE BOOL "Whether to integrate CSC compression")
set(WANT_LZIP ON CACHE BOOL "Whether to integrate LZip (LZMA) compression")
set(WANT_ZLIB ON CACHE BOOL "Whether to integrate ZLib (deflate) compression")

# -------------------------------------------------------------------------------------------------

find_package(Threads REQUIRED)

if(BUILD_DOCS)
	find_package(Doxygen)
endif()

#set(NuclexSupportNative_DIR "../Nuclex.Support.Native")
#find_package(NuclexSupportNative REQUIRED CONFIG)

add_subdirectory(${PROJECT_SOURCE_DIR}/../Nuclex.Support.Native ${CMAKE_BINARY_DIR}/Nuclex.Support.Native)

# Already included by Nuclex.Support.Native, CMake doesn't like a second inclusion...
#add_subdirectory(${PROJECT_SOURCE_DIR}/../third-party/gtest ${CMAKE_BINARY_DIR}/gtest)

add_subdirectory(${PROJECT_SOURCE_DIR}/../third-party/expat ${CMAKE_BINARY_DIR}/expat)

if(WANT_BROTLI)
	add_subdirectory(${PROJECT_SOURCE_DIR}/../third-party/brotli ${CMAKE_BINARY_DIR}/brotli)
endif()
if(WANT_CSC)
	add_subdirectory(${PROJECT_SOURCE_DIR}/../third-party/csc ${CMAKE_BINARY_DIR}/csc)
endif()
if(WANT_LZIP)
	add_subdirectory(${PROJECT_SOURCE_DIR}/../third-party/lzip ${CMAKE_BINARY_DIR}/lzip)
endif()
if(WANT_ZLIB)
	add_subdirectory(${PROJECT_SOURCE_DIR}/../third-party/zlib ${CMAKE_BINARY_DIR}/zlib)
endif()

# -------------------------------------------------------------------------------------------------

if(ENABLE_COVERAGE)

	# Function from cplusplus.cmake that adds the required flags to
	# the CMAKE_C_FLAGS and CMAKE_CXX_FLAGS globally-applied build flags
	set_coverage_c_cxx_flags()

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
	"Include/Nuclex/Storage/*.h"
)

file(
	GLOB_RECURSE unittestFiles
	CONFIGURE_DEPENDS
	"Tests/*.cpp"
)

# -------------------------------------------------------------------------------------------------

add_library(NuclexStorageNative SHARED)

target_include_directories(
	NuclexStorageNative
	PUBLIC "Include"
)

target_sources(
	NuclexStorageNative
	PUBLIC ${headerFiles}
	PRIVATE ${sourceFiles}
)

target_link_libraries(
	NuclexStorageNative
	PRIVATE Expat::Static
	PUBLIC NuclexSupportNative
)

if(WANT_BROTLI)
	target_compile_definitions(NuclexStorageNative PRIVATE NUCLEX_STORAGE_HAVE_BROTLI)
	target_link_libraries(NuclexStorageNative PRIVATE Brotli::Static)
endif()
if(WANT_CSC)
	target_compile_definitions(NuclexStorageNative PRIVATE NUCLEX_STORAGE_HAVE_CSC)
	target_link_libraries(NuclexStorageNative PRIVATE Csc::Static)
endif()
if(WANT_LZIP)
	target_compile_definitions(NuclexStorageNative PRIVATE NUCLEX_STORAGE_HAVE_LZIP)
	target_link_libraries(NuclexStorageNative PRIVATE LZip::Static)
endif()
if(WANT_ZLIB)
	target_compile_definitions(NuclexStorageNative PRIVATE NUCLEX_STORAGE_HAVE_ZLIB)
	target_link_libraries(NuclexStorageNative PRIVATE ZLib::SoloStatic)
endif()

if(WIN32)
	set_target_properties(NuclexStorageNative PROPERTIES OUTPUT_NAME "Nuclex.Storage.Native")
endif()

# On Unix/Linux systems, look for shared libraries in the same directory
# (this is where libNuclexSupportNative.so is searched first)
set_target_properties(
	NuclexStorageNative PROPERTIES
	BUILD_RPATH_USE_ORIGIN ON
	BUILD_WITH_INSTALL_RPATH ON
	INSTALL_RPATH_USE_LINK_PATH OFF
	INSTALL_RPATH "\${ORIGIN}"
)

# -------------------------------------------------------------------------------------------------

add_executable(NuclexStorageNativeTests)

target_compile_definitions(
	NuclexStorageNativeTests
	PRIVATE NUCLEX_STORAGE_EXECUTABLE
)

target_include_directories(
	NuclexStorageNativeTests
	PUBLIC "Include"
)
#	PUBLIC NuclexSupportNative

target_sources(
	NuclexStorageNativeTests
	PRIVATE ${headerFiles}
	PRIVATE ${sourceFiles}
	PRIVATE ${unittestFiles}
)

target_link_libraries(
	NuclexStorageNativeTests
	PRIVATE GoogleTest::Static
	PRIVATE GoogleTest::Main
	PRIVATE Expat::Static
	PRIVATE Threads::Threads
	PUBLIC NuclexSupportNative
)

if(WANT_BROTLI)
	target_compile_definitions(NuclexStorageNativeTests PRIVATE NUCLEX_STORAGE_HAVE_BROTLI)
	target_link_libraries(NuclexStorageNativeTests PRIVATE Brotli::Static)
endif()
if(WANT_CSC)
	target_compile_definitions(NuclexStorageNativeTests PRIVATE NUCLEX_STORAGE_HAVE_CSC)
	target_link_libraries(NuclexStorageNativeTests PRIVATE Csc::Static)
endif()
if(WANT_LZIP)
	target_compile_definitions(NuclexStorageNativeTests PRIVATE NUCLEX_STORAGE_HAVE_LZIP)
	target_link_libraries(NuclexStorageNativeTests PRIVATE LZip::Static)
endif()
if(WANT_ZLIB)
	target_compile_definitions(NuclexStorageNativeTests PRIVATE NUCLEX_STORAGE_HAVE_ZLIB)
	target_link_libraries(NuclexStorageNativeTests PRIVATE ZLib::SoloStatic)
endif()

if(WIN32)
	set_target_properties(
		NuclexStorageNativeTests PROPERTIES OUTPUT_NAME "Nuclex.Storage.Native.Tests"
	)
endif()

# On Unix/Linux systems, look for shared libraries in the same directory
# (this is where libNuclexSupportNative.so is searched first)
set_target_properties(
	NuclexStorageNativeTests PROPERTIES
	BUILD_RPATH_USE_ORIGIN ON
	BUILD_WITH_INSTALL_RPATH ON
	INSTALL_RPATH_USE_LINK_PATH OFF
	INSTALL_RPATH "\${ORIGIN}"
)

# -------------------------------------------------------------------------------------------------

install(
	TARGETS NuclexStorageNative
	ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

install(
	TARGETS NuclexSupportNative
	ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

install(
	TARGETS NuclexStorageNativeTests
	RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

# -------------------------------------------------------------------------------------------------

if(BUILD_DOCS)

	if(NOT DOXYGEN_FOUND)
		message(FATAL_ERROR "Can't build documentation because Doxygen was not found")
	endif()

	add_custom_target(
		NuclexStorageNativeDocs ALL
		COMMAND ${DOXYGEN_EXECUTABLE} "Nuclex.Storage.Native.doxygen.cfg"
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	)

endif()

# -------------------------------------------------------------------------------------------------

file(
	WRITE "${PROJECT_SOURCE_DIR}/NuclexStorageNativeConfig.cmake"
	"#!/usr/bin/cmake

# Configuration to include Nuclex.Storage.Native in a CMake-based project. If you want to
# reference Nuclex.Storage.Native as an externally compiled static library, do this:
#
#    set(NuclexStorageNative_DIR \"../Nuclex.Storage.Native\")
#    find_package(NuclexStorageNative REQUIRED CONFIG)
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE NuclexStorageNative::Dynamic
#    )
#
# Alternatively, if you want to build Nuclex.Storage.Native together with your project,
# use the normal CMakeLists.txt with CMake's add_subdirectory() command:
#
#    add_subdirectory(
#        \"\${PROJECT_SOURCE_DIR}/../Nuclex.Storage.Native\"
#        \"\${CMAKE_BINARY_DIR}/nuclex.storage.native\"
#    )
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE NuclexStorageNative
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

add_library(NuclexStorageNative::Dynamic SHARED IMPORTED)

# This may cause warnings on recent GCC versions (10.0.0+?) with LTO because GCC detects
# that the headers used during build (residing in build/) are not the same used when
# linking the library (copies resising in Include/).
#
# CMake doesn't run the install step during build, so the only way to get the headers
# in place before building would be by copying them rather than installing them.
set_target_properties(
	NuclexStorageNative::Dynamic PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES \"\${CMAKE_CURRENT_LIST_DIR}/Include\"
	IMPORTED_LINK_INTERFACE_LANGUAGES \"C\"
)

if(WIN32)
	set_target_properties(
		NuclexStorageNative::Dynamic PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/Nuclex.Storage.Native.lib\"
	)
else()
	set_target_properties(
		NuclexStorageNative::Dynamic PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/libNuclexStorageNative.so\"
	)
endif()

message(STATUS \"Imported Nuclex.Storage.Native targets with binaries in '\${CMAKE_CURRENT_LIST_DIR}'\")"
)

# -------------------------------------------------------------------------------------------------
