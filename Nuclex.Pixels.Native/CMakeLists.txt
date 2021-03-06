#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.8)

# -------------------------------------------------------------------------------------------------

project(
	NuclexPixelsNative
	VERSION 1.0.0
	DESCRIPTION "Bitmap loading, pixel format conversion and scaling library"
)

include("../BuildSystem/cmake/cplusplus.cmake")

set(BUILD_DOCS OFF CACHE BOOL "Whether to generate documentation via Doxygen")
set(ENABLE_COVERAGE OFF CACHE BOOL "Whether to instrument binaries for code coverage metrics")

set(WANT_PNG ON CACHE BOOL "Whether to integrate PNG image loading and saving")
set(WANT_JPG ON CACHE BOOL "Whether to integrate JPG image loading and saving")
set(WANT_EXR ON CACHE BOOL "Whether to integrate OpenEXR image loading and saving")
set(WANT_AV1F OFF CACHE BOOL "Whether to integrate AV1 image loading and saving")
set(WANT_WEBP OFF CACHE BOOL "Whether to integrate WebP image loading and saving")

# -------------------------------------------------------------------------------------------------

find_package(Threads REQUIRED)

if(BUILD_DOCS)
	find_package(Doxygen)
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/../Nuclex.Support.Native ${CMAKE_BINARY_DIR}/Nuclex.Support.Native)

# Already included by Nuclex.Support.Native, CMake doesn't like a second inclusion...
#add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/gtest ${CMAKE_BINARY_DIR}/gtest)

if(WANT_PNG)
	#add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/zlib ${CMAKE_BINARY_DIR}/zlib)
	add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/libpng ${CMAKE_BINARY_DIR}/libpng)
endif()
if(WANT_JPG)
	add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/libjpeg ${CMAKE_BINARY_DIR}/libjpeg)
endif()
if(WANT_EXR)
	#add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/ilmbase ${CMAKE_BINARY_DIR}/ilmbase)
	add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/openexr ${CMAKE_BINARY_DIR}/openexr)
endif()
if(WANT_AV1F)
	add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/av1f ${CMAKE_BINARY_DIR}/av1f)
endif()
if(WANT_WEBP)
	add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/web ${CMAKE_BINARY_DIR}/webp)
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
	"Include/Nuclex/Pixels/*.h"
)

file(
	GLOB_RECURSE unittestFiles
	CONFIGURE_DEPENDS
	"Tests/*.cpp"
)

# -------------------------------------------------------------------------------------------------

add_library(NuclexPixelsNative SHARED)

target_include_directories(
	NuclexPixelsNative
	PUBLIC "Include"
)

target_sources(
	NuclexPixelsNative
	PUBLIC ${headerFiles}
	PRIVATE ${sourceFiles}
)

target_link_libraries(
	NuclexPixelsNative
	PUBLIC NuclexSupportNative
)

if(WANT_PNG)
	target_compile_definitions(NuclexPixelsNative PRIVATE NUCLEX_PIXELS_HAVE_LIBPNG)
	target_link_libraries(NuclexPixelsNative PRIVATE ZLib::Static)
	target_link_libraries(NuclexPixelsNative PRIVATE LibPNG::Static)
endif()
if(WANT_JPG)
	target_compile_definitions(NuclexPixelsNative PRIVATE NUCLEX_PIXELS_HAVE_LIBJPEG)
	target_link_libraries(NuclexPixelsNative PRIVATE LibJPEG::Static)
endif()
if(WANT_EXR)
	target_compile_definitions(NuclexPixelsNative PRIVATE NUCLEX_PIXELS_HAVE_OPENEXR)
	target_link_libraries(NuclexPixelsNative PRIVATE OpenEXR::Static)
endif()
if(WANT_AV1F)
	target_compile_definitions(NuclexPixelsNative PRIVATE NUCLEX_PIXELS_HAVE_AV1F)
	target_link_libraries(NuclexPixelsNative PRIVATE AV1F::Static)
endif()
if(WANT_WEBP)
	target_compile_definitions(NuclexPixelsNative PRIVATE NUCLEX_PIXELS_HAVE_WEBP)
	target_link_libraries(NuclexPixelsNative PRIVATE WebP::Static)
endif()

if(WIN32)
	set_target_properties(NuclexPixelsNative PROPERTIES OUTPUT_NAME "Nuclex.Pixels.Native")
endif()

# On Unix/Linux systems, look for shared libraries in the same directory
# (this is where libNuclexSupportNative.so is searched first)
set_target_properties(
	NuclexPixelsNative PROPERTIES
	BUILD_RPATH_USE_ORIGIN ON
	BUILD_WITH_INSTALL_RPATH ON
	INSTALL_RPATH_USE_LINK_PATH OFF
	INSTALL_RPATH "\${ORIGIN}"
)

# -------------------------------------------------------------------------------------------------

add_executable(NuclexPixelsNativeTests)

target_compile_definitions(
	NuclexPixelsNativeTests
	PRIVATE NUCLEX_PIXELS_EXECUTABLE
)

target_include_directories(
	NuclexPixelsNativeTests
	PUBLIC "Include"
)
#	PUBLIC NuclexSupportNative

target_sources(
	NuclexPixelsNativeTests
	PRIVATE ${headerFiles}
	PRIVATE ${sourceFiles}
	PRIVATE ${unittestFiles}
)

target_link_libraries(
	NuclexPixelsNativeTests
	PRIVATE GoogleTest::Static
	PRIVATE GoogleTest::Main
	PRIVATE Threads::Threads
	PUBLIC NuclexSupportNative
)

if(WANT_PNG)
	target_compile_definitions(NuclexPixelsNativeTests PRIVATE NUCLEX_PIXELS_HAVE_LIBPNG)
	target_link_libraries(NuclexPixelsNativeTests PRIVATE ZLib::Static)
	target_link_libraries(NuclexPixelsNativeTests PRIVATE LibPNG::Static)
endif()
if(WANT_JPG)
	target_compile_definitions(NuclexPixelsNativeTests PRIVATE NUCLEX_PIXELS_HAVE_LIBJPEG)
	target_link_libraries(NuclexPixelsNativeTests PRIVATE LibJPEG::Static)
endif()
if(WANT_EXR)
	target_compile_definitions(NuclexPixelsNativeTests PRIVATE NUCLEX_PIXELS_HAVE_OPENEXR)
	#target_link_libraries(NuclexPixelsNativeTests PRIVATE IlmBase::Static)
	target_link_libraries(NuclexPixelsNativeTests PRIVATE OpenEXR::Static)
endif()
if(WANT_AV1F)
	target_compile_definitions(NuclexPixelsNativeTests PRIVATE NUCLEX_PIXELS_HAVE_AV1F)
	target_link_libraries(NuclexPixelsNativeTests PRIVATE AV1F::Static)
endif()
if(WANT_WEBP)
	target_compile_definitions(NuclexPixelsNativeTests PRIVATE NUCLEX_PIXELS_HAVE_WEBP)
	target_link_libraries(NuclexPixelsNativeTests PRIVATE WebP::Static)
endif()

if(WIN32)
	set_target_properties(
		NuclexPixelsNativeTests PROPERTIES OUTPUT_NAME "Nuclex.Pixels.Native.Tests"
	)
endif()

# On Unix/Linux systems, look for shared libraries in the same directory
# (this is where libNuclexSupportNative.so is searched first)
set_target_properties(
	NuclexPixelsNativeTests PROPERTIES
	BUILD_RPATH_USE_ORIGIN ON
	BUILD_WITH_INSTALL_RPATH ON
	INSTALL_RPATH_USE_LINK_PATH OFF
	INSTALL_RPATH "\${ORIGIN}"
)

# -------------------------------------------------------------------------------------------------

install(
	TARGETS NuclexPixelsNative
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
	TARGETS NuclexPixelsNativeTests
	RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

# -------------------------------------------------------------------------------------------------

if(BUILD_DOCS)

	if(NOT DOXYGEN_FOUND)
		message(FATAL_ERROR "Can't build documentation because Doxygen was not found")
	endif()

	add_custom_target(
		NuclexPixelsNativeDocs ALL
		COMMAND ${DOXYGEN_EXECUTABLE} "Nuclex.Pixels.Native.doxygen.cfg"
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	)

endif()

# -------------------------------------------------------------------------------------------------

file(
	WRITE "${PROJECT_SOURCE_DIR}/NuclexPixelsNativeConfig.cmake"
	"#!/usr/bin/cmake

# Configuration to include Nuclex.Pixels.Native in a CMake-based project. If you want to
# reference Nuclex.Pixels.Native as an externally compiled static library, do this:
#
#    set(NuclexPixelsNative_DIR \"../Nuclex.Pixels.Native\")
#    find_package(NuclexPixelsNative REQUIRED CONFIG)
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE NuclexPixelsNative::Dynamic
#    )
#
# Alternatively, if you want to build Nuclex.Pixels.Native together with your project,
# use the normal CMakeLists.txt with CMake's add_subdirectory() command:
#
#    add_subdirectory(
#        \"\${PROJECT_SOURCE_DIR}/../Nuclex.Pixels.Native\"
#        \"\${CMAKE_BINARY_DIR}/nuclex.pixels.native\"
#    )
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE NuclexPixelsNative
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

add_library(NuclexPixelsNative::Dynamic SHARED IMPORTED)

# This may cause warnings on recent GCC versions (10.0.0+?) with LTO because GCC detects
# that the headers used during build (residing in build/) are not the same used when
# linking the library (copies resising in Include/).
#
# CMake doesn't run the install step during build, so the only way to get the headers
# in place before building would be by copying them rather than installing them.
set_target_properties(
	NuclexPixelsNative::Dynamic PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES \"\${CMAKE_CURRENT_LIST_DIR}/Include\"
	IMPORTED_LINK_INTERFACE_LANGUAGES \"C\"
)

if(WIN32)
	set_target_properties(
		NuclexPixelsNative::Dynamic PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/Nuclex.Pixels.Native.lib\"
	)
else()
	set_target_properties(
		NuclexPixelsNative::Dynamic PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/libNuclexPixelsNative.so\"
	)
endif()

message(STATUS \"Imported Nuclex.Pixels.Native targets with binaries in '\${CMAKE_CURRENT_LIST_DIR}'\")"
)

# -------------------------------------------------------------------------------------------------
