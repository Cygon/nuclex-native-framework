#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.8)

# ----------------------------------------------------------------------------------------------- #

# Target C++17 if any C++ code is compiled.
set(CMAKE_CXX_STANDARD 17)

# Require C++17 and refuse to build otherwise.
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# No GNU or MSVC extensions to the language allowed.
set(CMAKE_CXX_EXTENSIONS OFF)

# Build code with relative jump addresses, allows dynamic linking (for shared libraries)
# and allows executabls to work on systems with forcd ASLR.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ----------------------------------------------------------------------------------------------- #

# Detect which compiler is being used
#
# This sets one of the following flags
#   CMAKE_COMPILER_IS_INTEL  Intel's special optimizing compiler, exists on multiple platforms
#   CMAKE_COMPILER_IS_CLANG  CLang, an alternative to GCC that's become pretty popular
#   CMAKE_COMPILER_IS_GCC    GNU C/C++ compiler, the standard compiler on Linux systems
#   CMAKE_COMPILER_IS_MSVC   Microsoft Visual C++, the stndard compiler on Windows systems
#
# F*CK! CMake doesn't check the compiler until the first project() directory has run.
# Dear CMake developers, what is wrong in your heads? Does anything ever work right here?
#
if(
    (NOT CMAKE_COMPILER_IS_INTEL) AND
    (NOT CMAKE_COMPILER_IS_CLANG) AND
    (NOT CMAKE_COMPILER_IS_GCC) AND
    (NOT CMAKE_COMPILER_IS_MSVC)
)

    if(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
        set(CMAKE_COMPILER_IS_INTEL ON)
        message(STATUS "Assuming compiler is the Intel C++ compiler")
    elseif(
        (CMAKE_CXX_COMPILER_ID STREQUAL "Clang") OR
        (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    )
        set(CMAKE_COMPILER_IS_CLANG ON)
        message(STATUS "Assuming compiler is Clang")
    elseif(
        (CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR
        CMAKE_COMPILER_IS_GNUCXX OR
        CMAKE_COMPILER_IS_GNUCC
    )
        set(CMAKE_COMPILER_IS_GCC ON)
        message(STATUS "Assuming compiler is the GNU compiler (GCC)")
    elseif(MSVC)
        set(CMAKE_COMPILER_IS_MSVC ON)
        message(STATUS "Assuming compiler is Microsoft Visual C++")
    else()
        message(WARNING "Unsupported compiler used, flags will not be optimal")
    endif()

endif()

# ----------------------------------------------------------------------------------------------- #

# Target architecture
#
# Looks like CMake... just forgot this one. Of course. Mature build system and all that.
#
# We can get the current system's architecture, but without CMake's support we have no
# way to detect a cross-compile.
#
if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(CMAKE_TARGET_ARCHITECTURE "x86")
    else()
        set(CMAKE_TARGET_ARCHITECTURE "amd64")
    endif()
else()
    EXECUTE_PROCESS(
        COMMAND uname -m
        COMMAND tr -d '\n'
        OUTPUT_VARIABLE architecture
    )
    string(FIND ${architecture} "arm" armArchitectureIndex)
    if(NOT ${armArchitectureIndex} EQUAL -1)
        if(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(CMAKE_TARGET_ARCHITECTURE "armhf")
        else()
            set(CMAKE_TARGET_ARCHITECTURE "arm64")
        endif()
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(CMAKE_TARGET_ARCHITECTURE "x86")
    else()
        set(CMAKE_TARGET_ARCHITECTURE "amd64")
    endif()
endif()

# ----------------------------------------------------------------------------------------------- #

if(NOT NUCLEX_COMPILER_TAG)

    # Set up common compiler flags depending on the platform used
    #
    # Visual C++ flags (matched against Visual C++ 2017)
    if(CMAKE_COMPILER_IS_MSVC OR CMAKE_COMPILER_IS_INTEL)

        if(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:SSE2") # Target CPUs from 2003 and later
            #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:AVX") # Target CPUs from 2011 and later

            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2") # Target CPUs from 2003 and later
            #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX") # Target CPUs from 2011 and later
        else()
            # Note that SSE2 is in the AMD64 specification, so all 64-bit CPUs have SSE2
            #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:AVX") # Target CPUs from 2011 and later
            #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX") # Target CPUs from 2011 and later
        endif()

        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /GF") # String pooling in debug and release
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8") # Source code and outputs are UTF-8
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /GS-") # No buffer checks (we make games!)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /FS") # PDBs can be written from multiple processes

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GF") # String pooling in debug and release
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8") # Source code and outputs are UTF-8
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GS-") # No buffer checks (we make games!)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FS") # PDBs can be written from multiple processes

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc") # Enable only C++ exceptions
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17") # Target a specific, recent standard
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR") # Generate RTTI for dynamic_cast/type_info

        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MDd") # Debug runtime
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Od") # No optimization
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Zi") # Debugging information

        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd") # Debug runtime
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od") # No optimization
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi") # Debugging information

        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MD") # DLL runtime
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2") # Optimize for speed
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Gy") # Function-level linking
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /GL") # Whole program optimization
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Gw") # Optimize data across units

        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD") # DLL runtime
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2") # Optimize for speed
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Gy") # Function-level linking
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL") # Whole program optimization
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Gw") # Optimize data across units

        set(STATIC_LIBRARY_FLAGS_RELEASE "${STATIC_LIBRARY_FLAGS_RELEASE} /LTCG")
        set(LINK_FLAGS_RELEASE "${LINK_FLAGS_RELEASE} /LTCG")

    endif()

    # GCC flags (matched against GCC 9.3)
    if(CMAKE_COMPILER_IS_GCC OR CMAKE_COMPILER_IS_CLANG)

        # Target hardware
        if(${CMAKE_TARGET_ARCHITECTURE} STREQUAL "armhf")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a+crc+simd") # Raspberry PI 3 CPU
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mtune=cortex-a53") # Raspberry PI 3 CPU
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpu=crypto-neon-fp-armv8") # Raspberry PI 3 FPU

            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a+crc+simd") # Raspberry PI 3 CPU
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mtune=cortex-a53") # Raspberry PI 3 CPU
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=crypto-neon-fp-armv8") # Raspberry PI 3 FPU
        else()
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=nocona") # Target CPUs from 2003 and later
            #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=bdver1") # Target CPUs from 2011 and later

            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=nocona") # Target CPUs from 2003 and later
            #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=bdver1") # Target CPUs from 2011 and later
        endif()

        # C language and build settings
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden") # Don't expose by default
        if(NOT CMAKE_COMPILER_IS_CLANG)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -shared-libgcc") # Use shared libgcc
        endif()
        #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpic") # Use position-independent code
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmerge-all-constants") # Data deduplication

        # C math routine behavior
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -funsafe-math-optimizations") # Allow optimizations
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-trapping-math") # Don't detect 0-div / overflow
        if(NOT CMAKE_COMPILER_IS_CLANG)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-signaling-nans") # NaN never causes exceptions
        endif()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-math-errno") # Don't set errno for math calls
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-rounding-math") # Blindly assume round-to-nearest
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -freciprocal-math") # Allow x/y to become x * (1/y)

        # C++ language and build settings
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17") # Target a specific, recent standard
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden") # Don't expose by default
        if(NOT CMAKE_COMPILER_IS_CLANG)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -shared-libgcc") # Use shared libgcc
        endif()
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic") # Use position-independent code
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmerge-all-constants") # Data deduplication
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden") # Inline code is hidden

        # C++ math routine behavior
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funsafe-math-optimizations") # Allow optimizations
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-trapping-math") # Don't detect 0-div / overflow
        if(NOT CMAKE_COMPILER_IS_CLANG)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-signaling-nans") # NaN never causes exceptions
        endif()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-math-errno") # Don't set errno for math calls
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rounding-math") # Blindly assume round-to-nearest
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -freciprocal-math") # Allow x/y to become x * (1/y)

        # Optimization flags for release builds
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3") # Optimize for speed
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -flto") # Link-time optimization
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-stack-protector") # Unprotected

        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3") # Optimize for speed
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto") # Link-time optimization
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-stack-protector") # Unprotected

        # Debugger flags for debug builds
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g3") # Generate debug information
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb") # Target the GDB debugger
        #set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Og") # Optimize for debug (nope!)
        #set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fbounds-checking") # Array bounds check

        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3") # Generate debug information
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb") # Target the GDB debugger
        #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og") # Optimize for debug (nope!)
        #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fbounds-checking") # Array bounds check

        # CMake has its own mechanism to set a relative rpath and maybe not all libraries
        # we compile want that. Disabled for now.
        #set(CMAKE_LINK_FLAGS "${CMAKE_LINK_FLAGS} -Wl,-rpath='\${ORIGIN}'")

    endif()

endif()

# ----------------------------------------------------------------------------------------------- #

# Build a compiler tag
#
# CMake's design is dumb and assumes you only ever want one build and configuration, to the point
# of regenerating the entire Makefile to switch between debug and release. For packaging and
# deployment, we want a short tag that identifies target platform, compiler and architecture.
#
# It will look like these examples:
#   linux-gcc9.3-amd64-release
#   windows-msvc14.1-amd64-debug
#   linux-clang11.2-armhf-release
#

if(NOT NUCLEX_COMPILER_TAG)

    # Target OS
    if(WIN32)
        set(NUCLEX_COMPILER_TAG "windows")
    else()
        set(NUCLEX_COMPILER_TAG "linux")
    endif()

    # Compiler name
    if(CMAKE_COMPILER_IS_INTEL)
        set(NUCLEX_COMPILER_TAG "${NUCLEX_COMPILER_TAG}-icc")
    elseif(CMAKE_COMPILER_IS_CLANG)
        set(NUCLEX_COMPILER_TAG "${NUCLEX_COMPILER_TAG}-clang")
    elseif(CMAKE_COMPILER_IS_GCC)
        set(NUCLEX_COMPILER_TAG "${NUCLEX_COMPILER_TAG}-gcc")
    elseif(CMAKE_COMPILER_IS_MSVC)
        set(NUCLEX_COMPILER_TAG "${NUCLEX_COMPILER_TAG}-msvc")
    else()
        set(NUCLEX_COMPILER_TAG "${NUCLEX_COMPILER_TAG}-unknown")
    endif()

    # Compiler version (only major.minor)
    string(FIND ${CMAKE_CXX_COMPILER_VERSION} . firstDotIndex)
    string(SUBSTRING ${CMAKE_CXX_COMPILER_VERSION} 0 ${firstDotIndex} majorVersion)
    math(EXPR firstDotIndex "${firstDotIndex} + 1")
    string(SUBSTRING ${CMAKE_CXX_COMPILER_VERSION} ${firstDotIndex} -1 remainder)
    string(FIND ${remainder} . secondDotIndex)
    string(SUBSTRING ${remainder} 0 ${secondDotIndex} minorVersion)

    # From regular compiler versions to a giant steaming mess
    # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warnings-by-compiler-version?view=msvc-160
    if(majorVersion EQUAL 19)
        if(minorVersion GREATER_EQUAL 20)
            set(NUCLEX_COMPILER_TAG "${NUCLEX_COMPILER_TAG}14.2") # VS2019
        elseif(minorVersion GREATER_EQUAL 10)
            set(NUCLEX_COMPILER_TAG "${NUCLEX_COMPILER_TAG}14.1") # VS2017
        else()
            set(NUCLEX_COMPILER_TAG "${NUCLEX_COMPILER_TAG}14.0") # VS2015
        endif()
    else()
        set(NUCLEX_COMPILER_TAG "${NUCLEX_COMPILER_TAG}${majorVersion}.${minorVersion}")
    endif()

    # Target architecture
    set(NUCLEX_COMPILER_TAG "${NUCLEX_COMPILER_TAG}-${CMAKE_TARGET_ARCHITECTURE}")

    # Debug/Release mode
    string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_LOWERCASE)
    if(CMAKE_BUILD_TYPE_LOWERCASE STREQUAL "debug")
        set(NUCLEX_COMPILER_TAG "${NUCLEX_COMPILER_TAG}-debug")
    else()
        set(NUCLEX_COMPILER_TAG "${NUCLEX_COMPILER_TAG}-release")
    endif()

    set(CMAKE_COMPILER_TAG "${NUCLEX_COMPILER_TAG}") # Backwards compatibility with old scripts
    message(STATUS "Compiler tag for this build is ${NUCLEX_COMPILER_TAG}")

endif()

# ----------------------------------------------------------------------------------------------- #

function(enable_target_compiler_warnings target_name)

    if(CMAKE_COMPILER_IS_MSVC OR CMAKE_COMPILER_IS_INTEL)

        # Can't do /Wall on Microsoft. You get 10000+ warnings just for including
        # some standard library headers.
        #target_compile_options(${target_name} PRIVATE /Wall)

        target_compile_options(${target_name} PRIVATE /W4)

        #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Wall") # Enable all warnings
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall") # Enable all warnings

    endif()

    if(CMAKE_COMPILER_IS_GCC OR CMAKE_COMPILER_IS_CLANG)

        target_compile_options(${target_name} PRIVATE -Wall) # Enable all warnings
        target_compile_options(${target_name} PRIVATE -Wextra) # Even more warnings
        target_compile_options(${target_name} PRIVATE -Wpedantic) # Warn on standard deviations
        target_compile_options(${target_name} PRIVATE -Wno-unknown-pragmas) # No pragma warnings

        # C compiler warnings
        #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall") # Enable all warnings
        #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra") # Enable even more warnings
        #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wpedantic") # Enable standard deviation warnings
        #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unknown-pragmas") # Don't warn about pragmas

        # C++ compiler warnings
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall") # Enable all warnings
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra") # Enable even more warnings
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic") # Enable standard deviation warnings
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas") # Don't warn about pragmas

    endif()

    message(STATUS "Compiler warnings enabled for target ${target_name}")

endfunction()

# ----------------------------------------------------------------------------------------------- #

function(disable_target_compiler_warnings target_name)

    if(CMAKE_COMPILER_IS_MSVC OR CMAKE_COMPILER_IS_INTEL)

        target_compile_options(${target_name} PRIVATE /w)

        #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /w") # Disable all warnings
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w") # Disable all warnings

    endif()

    if(CMAKE_COMPILER_IS_GCC OR CMAKE_COMPILER_IS_CLANG)

        target_compile_options(${target_name} PRIVATE -w)

        #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w") # Disable all warnings
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w") # Disable all warnings

    endif()

    message(STATUS "Compiler warnings disabled for target ${target_name}")

endfunction()

# ----------------------------------------------------------------------------------------------- #

function(set_coverage_c_cxx_flags)

    if(CMAKE_COMPILER_IS_GCC)

        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-instr-generate")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcoverage-mapping")

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoverage-mapping")

    endif()

    if(CMAKE_COMPILER_IS_CLANG)

        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ftest-coverage")

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftest-coverage")

    endif()

endfunction()

# ----------------------------------------------------------------------------------------------- #
