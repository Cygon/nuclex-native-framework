#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.18)

# -------------------------------------------------------------------------------------------------

project(
	Celero
	VERSION 2.7.2
	DESCRIPTION "Benchmarking library"
)

include("../../BuildSystem/cmake/cplusplus.cmake")

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/downloads/v2.7.2.tar.gz)
	message(STATUS "Downloading Celero sources")
	file(
		DOWNLOAD https://github.com/DigitalInBlue/Celero/archive/refs/tags/v2.7.2.tar.gz
		${PROJECT_SOURCE_DIR}/downloads/v2.7.2.tar.gz
		SHOW_PROGRESS
		EXPECTED_HASH SHA256=91ba6071043427b1073857c20a81175a9272901821e39b16c6c0b053eca7c992
	)
endif()

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/build)
	message(STATUS "Extracting Celero sources")
	file(
		ARCHIVE_EXTRACT
		INPUT ${PROJECT_SOURCE_DIR}/downloads/v2.7.2.tar.gz
		DESTINATION ${CMAKE_BINARY_DIR}/extract
	)
	file(
		RENAME
		${CMAKE_BINARY_DIR}/extract/Celero-2.7.2
		${PROJECT_SOURCE_DIR}/build
	)
endif()

file(
	COPY "${PROJECT_SOURCE_DIR}/TestFixture.h"
	DESTINATION "${PROJECT_SOURCE_DIR}/build/include/celero/"
)

# -------------------------------------------------------------------------------------------------

set(
	sourceFiles
	"build/src/Archive.cpp"
	"build/src/Benchmark.cpp"
	"build/src/Callbacks.cpp"
	"build/src/Celero.cpp"
	"build/src/Console.cpp"
	"build/src/Distribution.cpp"
	"build/src/Exceptions.cpp"
	"build/src/Executor.cpp"
	"build/src/JUnit.cpp"
	"build/src/Memory.cpp"
	"build/src/Print.cpp"
	"build/src/Experiment.cpp"
	"build/src/ExperimentResult.cpp"
	"build/src/ResultTable.cpp"
	"build/src/TestVector.cpp"
	"build/src/TestFixture.cpp"
	"build/src/ThreadTestFixture.cpp"
	"build/src/Timer.cpp"
	"build/src/UserDefinedMeasurementCollector.cpp"
	"build/src/Utilities.cpp"
)
set(
	headerFiles
	"build/include/celero/Archive.h"
	"build/include/celero/Benchmark.h"
	"build/include/celero/Callbacks.h"
	"build/include/celero/Celero.h"
	"build/include/celero/CommandLine.h"
	"build/include/celero/Console.h"
	"build/include/celero/Distribution.h"
	"build/include/celero/Exceptions.h"
	"build/include/celero/Executor.h"
	"build/include/celero/Export.h"
	"build/include/celero/Factory.h"
	"build/include/celero/FileReader.h"
	"build/include/celero/GenericFactory.h"
	"build/include/celero/JUnit.h"
	"build/include/celero/Memory.h"
	"build/include/celero/Pimpl.h"
	"build/include/celero/PimplImpl.h"
	"build/include/celero/Print.h"
	"build/include/celero/Experiment.h"
	"build/include/celero/ExperimentResult.h"
	"build/include/celero/ResultTable.h"
	"build/include/celero/Statistics.h"
	"build/include/celero/TestFixture.h"
	"build/include/celero/ThreadLocal.h"
	"build/include/celero/ThreadTestFixture.h"
	"build/include/celero/TestVector.h"
	"build/include/celero/Timer.h"
	"build/include/celero/UserDefinedMeasurement.h"
	"build/include/celero/UserDefinedMeasurementCollector.h"
	"build/include/celero/UserDefinedMeasurementTemplate.h"
	"build/include/celero/Utilities.h"
)

# -------------------------------------------------------------------------------------------------

add_library(Celero STATIC)
add_library(Celero::Static ALIAS Celero)

target_compile_definitions(
	Celero
	PUBLIC CELERO_STATIC
	PRIVATE CELERO_EXPORTS
)

target_include_directories(
	Celero
	PUBLIC "build/include"
)

target_sources(
	Celero
	PUBLIC ${headerFiles}
	PRIVATE ${sourceFiles}
)

set_target_properties(Celero PROPERTIES PREFIX "")
set_target_properties(Celero PROPERTIES OUTPUT_NAME "celero")

# -------------------------------------------------------------------------------------------------

install(
	FILES ${headerFiles}
	DESTINATION ${PROJECT_SOURCE_DIR}/Include
)

install(
	TARGETS Celero
	ARCHIVE	DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

# -------------------------------------------------------------------------------------------------

file(
	WRITE "${PROJECT_SOURCE_DIR}/CeleroConfig.cmake"
	"#!/usr/bin/cmake

# Configuration to include Celero in a CMake-based project. If you want to
# reference Celero as an externally compiled static library, do this:
#
#    set(Celero_DIR \"../ThirdParty/celero\")
#    find_package(Celero REQUIRED CONFIG)
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE Celero::Static
#    )
#
# Alternatively, if you want to build Celero together with your project,
# use the normal CMakeLists.txt with CMake's add_subdirectory() command:
#
#    add_subdirectory(
#        \"\${PROJECT_SOURCE_DIR}/../ThirdParty/celero\"
#        \"\${CMAKE_BINARY_DIR}/celero\"
#    )
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE Celero
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

add_library(Celero::Static STATIC IMPORTED)

# This may cause warnings on recent GCC versions (10.0.0+?) with LTO because GCC detects
# that the headers used during build (residing in build/) are not the same used when
# linking the library (copies resising in Include/).
#
# CMake doesn't run the install step during build, so the only way to get the headers
# in place before building would be by copying them rather than installing them.
set_target_properties(
	Celero::Static PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES \"\${CMAKE_CURRENT_LIST_DIR}/Include\"
	IMPORTED_LINK_INTERFACE_LANGUAGES \"C\"
)

if(WIN32)
	set_target_properties(
		Celero::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/celero.lib\"
	)
else()
	set_target_properties(
		Celero::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/celero.a\"
	)
endif()

message(STATUS \"Imported Celero targets with binaries in '\${CMAKE_CURRENT_LIST_DIR}'\")"
)

# -------------------------------------------------------------------------------------------------
