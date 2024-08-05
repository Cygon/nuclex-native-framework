#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.18)

# -------------------------------------------------------------------------------------------------

project(
	OpenEXR
	VERSION 2.3.0
	DESCRIPTION "Library for loading and saving EXR (floating point HDR) images"
)

include("../../build-system/cmake/cplusplus.cmake")

add_subdirectory(${PROJECT_SOURCE_DIR}/../ilmbase ${CMAKE_BINARY_DIR}/ilmbase)
if(NOT (TARGET ZLib))
	add_subdirectory(${PROJECT_SOURCE_DIR}/../zlib ${CMAKE_BINARY_DIR}/zlib)
endif()

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/downloads/openexr-2.3.0.tar.gz)
	message(STATUS "Downloading OpenEXR sources")
	file(
		DOWNLOAD https://github.com/openexr/openexr/releases/download/v2.3.0/openexr-2.3.0.tar.gz
		${PROJECT_SOURCE_DIR}/downloads/openexr-2.3.0.tar.gz
		SHOW_PROGRESS
		EXPECTED_HASH SHA256=fd6cb3a87f8c1a233be17b94c74799e6241d50fc5efd4df75c7a4b9cf4e25ea6
	)
endif()

# -------------------------------------------------------------------------------------------------

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/build)
	message(STATUS "Extracting OpenEXR sources")
	file(
		ARCHIVE_EXTRACT
		INPUT ${PROJECT_SOURCE_DIR}/downloads/openexr-2.3.0.tar.gz
		DESTINATION ${CMAKE_BINARY_DIR}/extract
	)
	file(
		RENAME
		${CMAKE_BINARY_DIR}/extract/openexr-2.3.0
		${PROJECT_SOURCE_DIR}/build
	)
endif()

file(
	COPY "${PROJECT_SOURCE_DIR}/OpenEXRConfig.h"
	DESTINATION "${PROJECT_SOURCE_DIR}/build/config/"
)
file(
	COPY "${PROJECT_SOURCE_DIR}/b44ExpLogTable.h"
	DESTINATION "${PROJECT_SOURCE_DIR}/build/IlmImf/"
)
file(
	COPY "${PROJECT_SOURCE_DIR}/dwaLookups.h"
	DESTINATION "${PROJECT_SOURCE_DIR}/build/IlmImf/"
)
file(
	COPY "${PROJECT_SOURCE_DIR}/ImfAttribute.cpp"
	DESTINATION "${PROJECT_SOURCE_DIR}/build/IlmImf/"
)

# -------------------------------------------------------------------------------------------------

set(
	sourceFiles
	"build/IlmImf/ImfAcesFile.cpp"
	"build/IlmImf/ImfAttribute.cpp"
	"build/IlmImf/ImfB44Compressor.cpp"
	"build/IlmImf/ImfBoxAttribute.cpp"
	"build/IlmImf/ImfChannelList.cpp"
	"build/IlmImf/ImfChannelListAttribute.cpp"
	"build/IlmImf/ImfInputPartData.cpp"
	"build/IlmImf/ImfIntAttribute.cpp"
	"build/IlmImf/ImfIO.cpp"
	"build/IlmImf/ImfKeyCode.cpp"
	"build/IlmImf/ImfKeyCodeAttribute.cpp"
	"build/IlmImf/ImfLineOrderAttribute.cpp"
	"build/IlmImf/ImfLut.cpp"
	"build/IlmImf/ImfMatrixAttribute.cpp"
	"build/IlmImf/ImfMisc.cpp"
	"build/IlmImf/ImfRational.cpp"
	"build/IlmImf/ImfRationalAttribute.cpp"
	"build/IlmImf/ImfRgbaFile.cpp"
	"build/IlmImf/ImfRgbaYca.cpp"
	"build/IlmImf/ImfRle.cpp"
	"build/IlmImf/ImfRleCompressor.cpp"
	"build/IlmImf/ImfScanLineInputFile.cpp"
	"build/IlmImf/ImfDeepScanLineInputPart.cpp"
	"build/IlmImf/ImfDeepScanLineOutputFile.cpp"
	"build/IlmImf/ImfDeepScanLineOutputPart.cpp"
	"build/IlmImf/ImfDeepTiledInputFile.cpp"
	"build/IlmImf/ImfDeepTiledInputPart.cpp"
	"build/IlmImf/ImfDeepTiledOutputFile.cpp"
	"build/IlmImf/ImfDeepTiledOutputPart.cpp"
	"build/IlmImf/ImfDoubleAttribute.cpp"
	"build/IlmImf/ImfDwaCompressor.cpp"
	"build/IlmImf/ImfChromaticities.cpp"
	"build/IlmImf/ImfDeepScanLineInputFile.cpp"
	"build/IlmImf/ImfPxr24Compressor.cpp"
	"build/IlmImf/ImfTiledInputFile.cpp"
	"build/IlmImf/ImfTiledInputPart.cpp"
	"build/IlmImf/ImfTiledMisc.cpp"
	"build/IlmImf/ImfTiledOutputFile.cpp"
	"build/IlmImf/ImfTiledOutputPart.cpp"
	"build/IlmImf/ImfTiledRgbaFile.cpp"
	"build/IlmImf/ImfTileOffsets.cpp"
	"build/IlmImf/ImfTimeCode.cpp"
	"build/IlmImf/ImfTimeCodeAttribute.cpp"
	"build/IlmImf/ImfVecAttribute.cpp"
	"build/IlmImf/ImfVersion.cpp"
	"build/IlmImf/ImfWav.cpp"
	"build/IlmImf/ImfZip.cpp"
	"build/IlmImf/ImfZipCompressor.cpp"
	"build/IlmImf/ImfEnvmap.cpp"
	"build/IlmImf/ImfEnvmapAttribute.cpp"
	"build/IlmImf/ImfFastHuf.cpp"
	"build/IlmImf/ImfFloatAttribute.cpp"
	"build/IlmImf/ImfFloatVectorAttribute.cpp"
	"build/IlmImf/ImfFrameBuffer.cpp"
	"build/IlmImf/ImfFramesPerSecond.cpp"
	"build/IlmImf/ImfGenericInputFile.cpp"
	"build/IlmImf/ImfGenericOutputFile.cpp"
	"build/IlmImf/ImfHeader.cpp"
	"build/IlmImf/ImfHuf.cpp"
	"build/IlmImf/ImfInputFile.cpp"
	"build/IlmImf/ImfInputPart.cpp"
	"build/IlmImf/ImfMultiPartInputFile.cpp"
	"build/IlmImf/ImfMultiPartOutputFile.cpp"
	"build/IlmImf/ImfMultiView.cpp"
	"build/IlmImf/ImfOpaqueAttribute.cpp"
	"build/IlmImf/ImfOutputFile.cpp"
	"build/IlmImf/ImfOutputPart.cpp"
	"build/IlmImf/ImfOutputPartData.cpp"
	"build/IlmImf/ImfPartType.cpp"
	"build/IlmImf/ImfPizCompressor.cpp"
	"build/IlmImf/ImfPreviewImage.cpp"
	"build/IlmImf/ImfPreviewImageAttribute.cpp"
	"build/IlmImf/ImfChromaticitiesAttribute.cpp"
	"build/IlmImf/ImfCompositeDeepScanLine.cpp"
	"build/IlmImf/ImfCompressionAttribute.cpp"
	"build/IlmImf/ImfCompressor.cpp"
	"build/IlmImf/ImfConvert.cpp"
	"build/IlmImf/ImfCRgbaFile.cpp"
	"build/IlmImf/ImfDeepCompositing.cpp"
	"build/IlmImf/ImfDeepFrameBuffer.cpp"
	"build/IlmImf/ImfDeepImageStateAttribute.cpp"
	"build/IlmImf/ImfStandardAttributes.cpp"
	"build/IlmImf/ImfStdIO.cpp"
	"build/IlmImf/ImfStringAttribute.cpp"
	"build/IlmImf/ImfStringVectorAttribute.cpp"
	"build/IlmImf/ImfSystemSpecific.cpp"
	"build/IlmImf/ImfTestFile.cpp"
	"build/IlmImf/ImfThreading.cpp"
	"build/IlmImf/ImfTileDescriptionAttribute.cpp"
)
set(
	headerFiles
	"build/IlmImf/ImfAcesFile.h"
	"build/IlmImf/ImfArray.h"
	"build/IlmImf/ImfAttribute.h"
	"build/IlmImf/ImfAutoArray.h"
	"build/IlmImf/ImfB44Compressor.h"
	"build/IlmImf/ImfBoxAttribute.h"
	"build/IlmImf/ImfChannelList.h"
	"build/IlmImf/ImfChannelListAttribute.h"
	"build/IlmImf/ImfCheckedArithmetic.h"
	"build/IlmImf/ImfInputPartData.h"
	"build/IlmImf/ImfInputStreamMutex.h"
	"build/IlmImf/ImfInt64.h"
	"build/IlmImf/ImfIntAttribute.h"
	"build/IlmImf/ImfIO.h"
	"build/IlmImf/ImfKeyCode.h"
	"build/IlmImf/ImfKeyCodeAttribute.h"
	"build/IlmImf/ImfLineOrder.h"
	"build/IlmImf/ImfLineOrderAttribute.h"
	"build/IlmImf/ImfLut.h"
	"build/IlmImf/ImfMatrixAttribute.h"
	"build/IlmImf/ImfPxr24Compressor.h"
	"build/IlmImf/ImfRational.h"
	"build/IlmImf/ImfRationalAttribute.h"
	"build/IlmImf/ImfRgba.h"
	"build/IlmImf/ImfRgbaFile.h"
	"build/IlmImf/ImfRgbaYca.h"
	"build/IlmImf/ImfRle.h"
	"build/IlmImf/ImfRleCompressor.h"
	"build/IlmImf/ImfScanLineInputFile.h"
	"build/IlmImf/ImfDeepScanLineInputFile.h"
	"build/IlmImf/ImfDeepScanLineInputPart.h"
	"build/IlmImf/ImfDeepScanLineOutputFile.h"
	"build/IlmImf/ImfDeepScanLineOutputPart.h"
	"build/IlmImf/ImfDeepTiledInputFile.h"
	"build/IlmImf/ImfDeepTiledInputPart.h"
	"build/IlmImf/ImfDeepTiledOutputFile.h"
	"build/IlmImf/ImfDeepTiledOutputPart.h"
	"build/IlmImf/ImfDoubleAttribute.h"
	"build/IlmImf/ImfDwaCompressor.h"
	"build/IlmImf/ImfDwaCompressorSimd.h"
	"build/IlmImf/ImfInputPart.h"
	"build/IlmImf/ImfMisc.h"
	"build/IlmImf/ImfSimd.h"
	"build/IlmImf/ImfTiledInputFile.h"
	"build/IlmImf/ImfTiledInputPart.h"
	"build/IlmImf/ImfTiledMisc.h"
	"build/IlmImf/ImfTiledOutputFile.h"
	"build/IlmImf/ImfTiledOutputPart.h"
	"build/IlmImf/ImfTiledRgbaFile.h"
	"build/IlmImf/ImfTileOffsets.h"
	"build/IlmImf/ImfTimeCode.h"
	"build/IlmImf/ImfTimeCodeAttribute.h"
	"build/IlmImf/ImfVecAttribute.h"
	"build/IlmImf/ImfVersion.h"
	"build/IlmImf/ImfWav.h"
	"build/IlmImf/ImfXdr.h"
	"build/IlmImf/ImfZip.h"
	"build/IlmImf/ImfZipCompressor.h"
	"build/IlmImf/ImfEnvmap.h"
	"build/IlmImf/ImfEnvmapAttribute.h"
	"build/IlmImf/ImfExport.h"
	"build/IlmImf/ImfFastHuf.h"
	"build/IlmImf/ImfFloatAttribute.h"
	"build/IlmImf/ImfFloatVectorAttribute.h"
	"build/IlmImf/ImfForward.h"
	"build/IlmImf/ImfFrameBuffer.h"
	"build/IlmImf/ImfFramesPerSecond.h"
	"build/IlmImf/ImfGenericInputFile.h"
	"build/IlmImf/ImfGenericOutputFile.h"
	"build/IlmImf/ImfHeader.h"
	"build/IlmImf/ImfHuf.h"
	"build/IlmImf/ImfInputFile.h"
	"build/IlmImf/ImfMultiPartInputFile.h"
	"build/IlmImf/ImfMultiPartOutputFile.h"
	"build/IlmImf/ImfMultiView.h"
	"build/IlmImf/ImfName.h"
	"build/IlmImf/ImfNamespace.h"
	"build/IlmImf/ImfOpaqueAttribute.h"
	"build/IlmImf/ImfOptimizedPixelReading.h"
	"build/IlmImf/ImfOutputFile.h"
	"build/IlmImf/ImfOutputPart.h"
	"build/IlmImf/ImfOutputPartData.h"
	"build/IlmImf/ImfOutputStreamMutex.h"
	"build/IlmImf/ImfPartHelper.h"
	"build/IlmImf/ImfPartType.h"
	"build/IlmImf/ImfPixelType.h"
	"build/IlmImf/ImfPizCompressor.h"
	"build/IlmImf/ImfPreviewImage.h"
	"build/IlmImf/ImfPreviewImageAttribute.h"
	"build/IlmImf/ImfChromaticities.h"
	"build/IlmImf/ImfChromaticitiesAttribute.h"
	"build/IlmImf/ImfCompositeDeepScanLine.h"
	"build/IlmImf/ImfCompression.h"
	"build/IlmImf/ImfCompressionAttribute.h"
	"build/IlmImf/ImfCompressor.h"
	"build/IlmImf/ImfConvert.h"
	"build/IlmImf/ImfCRgbaFile.h"
	"build/IlmImf/ImfDeepCompositing.h"
	"build/IlmImf/ImfDeepFrameBuffer.h"
	"build/IlmImf/ImfDeepImageState.h"
	"build/IlmImf/ImfDeepImageStateAttribute.h"
	"build/IlmImf/ImfStandardAttributes.h"
	"build/IlmImf/ImfStdIO.h"
	"build/IlmImf/ImfStringAttribute.h"
	"build/IlmImf/ImfStringVectorAttribute.h"
	"build/IlmImf/ImfSystemSpecific.h"
	"build/IlmImf/ImfTestFile.h"
	"build/IlmImf/ImfThreading.h"
	"build/IlmImf/ImfTileDescription.h"
	"build/IlmImf/ImfTileDescriptionAttribute.h"
)

# -------------------------------------------------------------------------------------------------

add_library(OpenEXR STATIC)
add_library(OpenEXR::Static ALIAS OpenEXR)

if(${CMAKE_PROJECT_NAME} STREQUAL "OpenEXR")
	enable_target_compiler_warnings(OpenEXR)
else()
	disable_target_compiler_warnings(OpenEXR)
endif()

target_link_libraries(
	OpenEXR
	PUBLIC IlmBase::Static
	PUBLIC ZLib::Static
)

target_include_directories(
	OpenEXR
	PUBLIC "build/config"
	PUBLIC "build"
	PUBLIC "build/IlmImf"
)

target_sources(
	OpenEXR
	PUBLIC ${headerFiles}
	PRIVATE ${sourceFiles}
)

#set_target_properties(OpenEXR PROPERTIES PREFIX "")
set_target_properties(OpenEXR PROPERTIES OUTPUT_NAME "openexr")

# -------------------------------------------------------------------------------------------------

install(
	DIRECTORY build/config
	DESTINATION ${PROJECT_SOURCE_DIR}/Include
	FILES_MATCHING PATTERN "*.h"
)
install(
	DIRECTORY build/IlmImf
	DESTINATION ${PROJECT_SOURCE_DIR}/Include
	FILES_MATCHING PATTERN "*.h"
)

install(
	TARGETS OpenEXR
	ARCHIVE
	DESTINATION ${PROJECT_SOURCE_DIR}/bin/${CMAKE_COMPILER_TAG}
)

# -------------------------------------------------------------------------------------------------

file(
	WRITE "${PROJECT_SOURCE_DIR}/OpenEXRConfig.cmake"
	"#!/usr/bin/cmake

# Configuration to include OpenEXR in a CMake-based project. If you want to
# reference OpenEXR as an externally compiled static library, do this:
#
#    set(OpenEXR_DIR \"../ThirdParty/openexr\")
#    find_package(OpenEXR REQUIRED CONFIG)
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE OpenEXR::Static
#    )
#
# Alternatively, if you want to build OpenEXR together with your project,
# use the normal CMakeLists.txt with CMake's add_subdirectory() command:
#
#    add_subdirectory(
#        \"\${PROJECT_SOURCE_DIR}/../ThirdParty/openexr\"
#        \"\${CMAKE_BINARY_DIR}/openexr\"
#    )
#
#    target_link_libraries(
#        MyAwesomeProject
#        PRIVATE OpenEXR
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

add_library(OpenEXR::Static STATIC IMPORTED)

# This may cause warnings on recent GCC versions (10.0.0+?) with LTO because GCC detects
# that the headers used during build (residing in build/) are not the same used when
# linking the library (copies resising in Include/).
#
# CMake doesn't run the install step during build, so the only way to get the headers
# in place before building would be by copying them rather than installing them.
set_target_properties(
	OpenEXR::Static PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES \"\${CMAKE_CURRENT_LIST_DIR}/Include\"
	IMPORTED_LINK_INTERFACE_LANGUAGES \"C\"
)

if(WIN32)
	set_target_properties(
		OpenEXR::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/openexr.lib\"
		COMPILE_DEFINITIONS OS_LINUX
	)
else()
	set_target_properties(
		OpenEXR::Static PROPERTIES
		IMPORTED_LOCATION \"\${CMAKE_CURRENT_LIST_DIR}/bin/\${NUCLEX_COMPILER_TAG}/openexr.a\"
	)
endif()

message(STATUS \"Imported OpenEXR targets with binaries in '\${CMAKE_CURRENT_LIST_DIR}'\")"
)

# -------------------------------------------------------------------------------------------------
