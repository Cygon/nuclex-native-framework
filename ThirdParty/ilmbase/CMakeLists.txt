#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.18)

# -------------------------------------------------------------------------------------------------

project(
	IlmBase
	VERSION 2.3.0
	DESCRIPTION "Bloaty reinvent-the-wheel library for OpenEXR"
)

include("../../BuildSystem/cmake/cplusplus.cmake")

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/downloads/ilmbase-2.3.0.tar.gz)
	message(STATUS "Downloading IlmBase sources")
	file(
		DOWNLOAD https://github.com/openexr/openexr/releases/download/v2.3.0/ilmbase-2.3.0.tar.gz
		${PROJECT_SOURCE_DIR}/downloads/ilmbase-2.3.0.tar.gz
		SHOW_PROGRESS
		EXPECTED_HASH SHA256=456978d1a978a5f823c7c675f3f36b0ae14dba36638aeaa3c4b0e784f12a3862
	)
endif()

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/build)
	message(STATUS "Extracting IlmBase sources")
	file(
		ARCHIVE_EXTRACT
		INPUT ${PROJECT_SOURCE_DIR}/downloads/ilmbase-2.3.0.tar.gz
		DESTINATION ${CMAKE_BINARY_DIR}/extract
	)
	file(
		RENAME
		${CMAKE_BINARY_DIR}/extract/ilmbase-2.3.0
		${PROJECT_SOURCE_DIR}/build
	)
endif()

file(
	COPY "${PROJECT_SOURCE_DIR}/IlmBaseConfig.h"
	DESTINATION "${PROJECT_SOURCE_DIR}/build/config/"
)
file(
	COPY "${PROJECT_SOURCE_DIR}/eLut.h"
	DESTINATION "${PROJECT_SOURCE_DIR}/build/Half/"
)
file(
	COPY "${PROJECT_SOURCE_DIR}/toFloat.h"
	DESTINATION "${PROJECT_SOURCE_DIR}/build/Half/"
)

# -------------------------------------------------------------------------------------------------

set(
	sourceFiles
	"build/Half/half.cpp"
	"build/Imath/ImathColorAlgo.cpp"
	"build/Imath/ImathFun.cpp"
	"build/Imath/ImathMatrixAlgo.cpp"
	"build/Imath/ImathRandom.cpp"
	"build/Imath/ImathVec.cpp"
	"build/Iex/IexBaseExc.cpp"
	"build/Iex/IexThrowErrnoExc.cpp"
	"build/IexMath/IexMathFloatExc.cpp"
	"build/IexMath/IexMathFpu.cpp"
	"build/IlmThread/IlmThread.cpp"
	"build/IlmThread/IlmThreadMutex.cpp"
	"build/IlmThread/IlmThreadPool.cpp"
	"build/IlmThread/IlmThreadSemaphore.cpp"
	"build/IlmThread/IlmThreadSemaphorePosixCompat.cpp"
)
if(WIN32)
	list(APPEND sourceFiles "build/IlmThread/IlmThreadWin32.cpp")
	list(APPEND sourceFiles "build/IlmThread/IlmThreadMutexWin32.cpp")
	list(APPEND sourceFiles "build/IlmThread/IlmThreadSemaphoreWin32.cpp")
else()
	list(APPEND sourceFiles "build/IlmThread/IlmThreadPosix.cpp")
	list(APPEND sourceFiles "build/IlmThread/IlmThreadMutexPosix.cpp")
	list(APPEND sourceFiles "build/IlmThread/IlmThreadSemaphorePosix.cpp")
endif()

set(
	headerFiles
	"build/Half/halfExport.h"
	"build/Half/halfFunction.h"
	"build/Half/half.h"
	"build/Half/halfLimits.h"
	"build/Iex/IexBaseExc.h"
	"build/Iex/IexErrnoExc.h"
	"build/Iex/IexExport.h"
	"build/Iex/IexForward.h"
	"build/Iex/Iex.h"
	"build/Iex/IexMacros.h"
	"build/Iex/IexMathExc.h"
	"build/Iex/IexNamespace.h"
	"build/Iex/IexThrowErrnoExc.h"
	"build/Imath/ImathBoxAlgo.h"
	"build/Imath/ImathBox.h"
	"build/Imath/ImathColorAlgo.h"
	"build/Imath/ImathColor.h"
	"build/Imath/ImathEuler.h"
	"build/Imath/ImathExc.h"
	"build/Imath/ImathExport.h"
	"build/Imath/ImathForward.h"
	"build/Imath/ImathFrame.h"
	"build/Imath/ImathFrustum.h"
	"build/Imath/ImathFrustumTest.h"
	"build/Imath/ImathFun.h"
	"build/Imath/ImathGL.h"
	"build/Imath/ImathGLU.h"
	"build/Imath/ImathHalfLimits.h"
	"build/Imath/ImathInt64.h"
	"build/Imath/ImathInterval.h"
	"build/Imath/ImathLimits.h"
	"build/Imath/ImathLineAlgo.h"
	"build/Imath/ImathLine.h"
	"build/Imath/ImathMath.h"
	"build/Imath/ImathMatrixAlgo.h"
	"build/Imath/ImathMatrix.h"
	"build/Imath/ImathNamespace.h"
	"build/Imath/ImathPlane.h"
	"build/Imath/ImathPlatform.h"
	"build/Imath/ImathQuat.h"
	"build/Imath/ImathRandom.h"
	"build/Imath/ImathRoots.h"
	"build/Imath/ImathShear.h"
	"build/Imath/ImathSphere.h"
	"build/Imath/ImathVecAlgo.h"
	"build/Imath/ImathVec.h"
	"build/IexMath/IexMathFloatExc.h"
	"build/IexMath/IexMathFpu.h"
	"build/IexMath/IexMathIeeeExc.h"
	"build/IlmThread/IlmThreadExport.h"
	"build/IlmThread/IlmThreadForward.h"
	"build/IlmThread/IlmThread.h"
	"build/IlmThread/IlmThreadMutex.h"
	"build/IlmThread/IlmThreadNamespace.h"
	"build/IlmThread/IlmThreadPool.h"
	"build/IlmThread/IlmThreadSemaphore.h"
)

# -------------------------------------------------------------------------------------------------

add_library(IlmBase STATIC)
add_library(IlmBase::Static ALIAS IlmBase)

if(${CMAKE_PROJECT_NAME} STREQUAL "IlmBase")
	enable_target_compiler_warnings(IlmBase)
else()
	disable_target_compiler_warnings(IlmBase)
endif()

target_include_directories(
	IlmBase
	PUBLIC "build/config"
	PUBLIC "build/Iex"
	PUBLIC "build/IexMath"
	PUBLIC "build/Imath"
	PUBLIC "build/IlmThread"
	PUBLIC "build/Half"
)

target_sources(
	IlmBase
	PUBLIC ${headerFiles}
	PRIVATE ${sourceFiles}
)

#set_target_properties(IlmBase PROPERTIES PREFIX "")
set_target_properties(IlmBase PROPERTIES OUTPUT_NAME "ilmbase")

# -------------------------------------------------------------------------------------------------

install(
	DIRECTORY build/Half
	DESTINATION ${PROJECT_SOURCE_DIR}/Include
	FILES_MATCHING PATTERN "*.h"
)
install(
	DIRECTORY build/Iex
	DESTINATION ${PROJECT_SOURCE_DIR}/Include
	FILES_MATCHING PATTERN "*.h"
)
install(
	DIRECTORY build/Imath
	DESTINATION ${PROJECT_SOURCE_DIR}/Include
	FILES_MATCHING PATTERN "*.h"
)
install(
	DIRECTORY build/IexMath
	DESTINATION ${PROJECT_SOURCE_DIR}/Include
	FILES_MATCHING PATTERN "*.h"
)
install(
	DIRECTORY build/IlmThread
	DESTINATION ${PROJECT_SOURCE_DIR}/Include
	FILES_MATCHING PATTERN "*.h"
)

install(
	TARGETS IlmBase
	ARCHIVE
	DESTINATION ${PROJECT_SOURCE_DIR}/bin/${CMAKE_COMPILER_TAG}
)

# -------------------------------------------------------------------------------------------------

file(
	WRITE "${PROJECT_SOURCE_DIR}/IlmBaseConfig.cmake"
	"#!/usr/bin/cmake

# Configuration to include IlmBase in a CMake-based project. If you want to
# reference IlmBase as an externally compiled static library, do this:
#
#    set(IlmBase_DIR \"../ThirdParty/ilmbase\")
#    find_package(IlmBase REQUIRED CONFIG)
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE IlmBase::Static
#    )
#
# Alternatively, if you want to build IlmBase together with your project,
# use the normal CMakeLists.txt with CMake's add_subdirectory() command:
#
#    add_subdirectory(
#        \"\${PROJECT_SOURCE_DIR}/../ThirdParty/ilmbase\"
#        \"\${CMAKE_BINARY_DIR}/ilmbase\"
#    )
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE IlmBase
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

add_library(IlmBase::Static STATIC IMPORTED)

# This may cause warnings on recent GCC versions (10.0.0+?) with LTO because GCC detects
# that the headers used during build (residing in build/) are not the same used when
# linking the library (copies resising in Include/).
#
# CMake doesn't run the install step during build, so the only way to get the headers
# in place before building would be by copying them rather than installing them.
set_target_properties(
	IlmBase::Static PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES \"\${CMAKE_CURRENT_LIST_DIR}/Include\"
	IMPORTED_LINK_INTERFACE_LANGUAGES \"C\"
)

if(WIN32)
	set_target_properties(
		IlmBase::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/ilmbase.lib\"
		COMPILE_DEFINITIONS OS_LINUX
	)
else()
	set_target_properties(
		IlmBase::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/ilmbase.a\"
	)
endif()

message(STATUS \"Imported IlmBase targets with binaries in '\${CMAKE_CURRENT_LIST_DIR}'\")"
)

# -------------------------------------------------------------------------------------------------
