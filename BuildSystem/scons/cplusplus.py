#!/usr/bin/env python

import os
import platform
import subprocess
import re

"""
Helpers for building C/C++ projects with SCons
"""

# ----------------------------------------------------------------------------------------------- #

def setup(environment):
    """Registers extension methods for C/C++ builds into a SCons environment

    @param  environment  Environment the extension methods will be registered to"""

    # Build setup
    environment.AddMethod(_add_include_directory, "add_include_directory")
    environment.AddMethod(_add_library_directory, "add_library_directory")
    environment.AddMethod(_add_library, "add_library")
    environment.AddMethod(_add_preprocessor_constant, "add_preprocessor_constant")

    # Information gathering
    environment.AddMethod(_get_build_directory_name, "get_build_directory_name")
    environment.AddMethod(_get_variant_directory_name, "get_variant_directory_name")

# ----------------------------------------------------------------------------------------------- #

def enumerate_headers(header_directory, variant_directory = None):
    """Forms a list of all C/C++ header files in an include directory

    @param  header_directory     Directory containing the headers
    @param  variant_directory    Variant directory to which source paths will be rewritten
    @returns The path of all discovered headers in their actual location or remapped to
             the variant directory if one was specified."""

    source_file_extensions = [
        '.h',
        '.H',
        '.hpp',
        '.hh',
        '.hxx',
        '.inl',
        '.inc'
    ]

    headers = []

    # Form a list of all files in the specified directory recursively.
    for root, directory_names, file_names in os.walk(header_directory):
        for file_name in file_names:
            file_title, file_extension = os.path.splitext(file_name)

            # We're only interested in files with an extension indicating a header file
            if file_extension and any(file_extension in s for s in source_file_extensions):
                if variant_directory is None:
                    headers.append(os.path.join(root, file_name))
                else:
                    headers.append(os.path.join(variant_directory, os.path.join(root, file_name)))

    return headers

# ----------------------------------------------------------------------------------------------- #

def enumerate_sources(source_directory, variant_directory = None):
    """Forms a list of all C/C++ source code files in a source directory

    @param  source_directory     Directory containing the C/C++ source code files
    @param  variant_directory    Variant directory to which source paths will be rewritten
    @returns The path of all discovered source files in their actual location or remapped to
             the variant directory if one was specified."""

    source_file_extensions = [
        '.c',
        '.C',
        '.cpp',
        '.cc',
        '.cxx'
    ]

    sources = []

    # Form a list of all files in the input directories recursively.
    for root, directory_names, file_names in os.walk(source_directory):
        for file_name in file_names:
            file_title, file_extension = os.path.splitext(file_name)

            # We're only interested in files with an extension indicating a C/C++ source file
            if file_extension and any(file_extension in s for s in source_file_extensions):
                if variant_directory is None:
                    sources.append(os.path.join(root, file_name))
                else:
                    sources.append(os.path.join(variant_directory, os.path.join(root, file_name)))

    return sources

# ----------------------------------------------------------------------------------------------- #

def find_or_guess_include_directory(package_path):
    """Tries to locate the include directory for a package. A package is a typical
    C/C++ library distribution as it could be found in a .tar.gz archive.

    Several known conventions are tried, such as an 'include' folder and a folder
    named identical to the package.

    @param  package_path  Path to the package
    @returns The detected include directory or None if it couldn't be located"""

    # Try our own standard with an uppercase directory first
    # If this script runs on Windows, it'll match any case, but that's fine.
    candidate = os.path.join(package_path, 'Include')
    if os.path.isdir(candidate):
        return candidate

    # Try a standard also used by many libraries, a lowercase directory
    candidate = os.path.join(package_path, 'include')
    if os.path.isdir(candidate):
        return candidate

    # If that also didn't work, try the package name. Some libraries use their own
    # name as the header directory so the include path can be set to their base directory
    # and header included by prefixing them with the library name.
    package_name = os.path.basename(os.path.normpath(package_path))
    candidate = os.path.join(package_path, package_name)
    if os.path.isdir(candidate):
        return candidate

    return None

# ------------------------------------------------------------------------------------------- #

def find_or_guess_library_directory(environment, library_builds_path):
    """Tries to locate the library directory for a package. There is no widely
    used schema for naming these to match specific compilers and platforms,
    so it looks for the 'build directory' convention used in this script.

    @param  environment          Environment used to look up architecture and compiler
    @param  library_builds_path  Path to the directory containing the library builds
                                 Each build is expected to be in a directory matching
                                 the build directory name (_get_variant_directory_name())
    @returns The library directory or None if it couldn't be found
    @remarks
        This can be used to automatically find the directory in which precompiled
        library binaries are stored."""

    # Determine whether this is a debug builkd
    is_debug_build = False
    if 'DEBUG' in environment:
        is_debug_build = environment['DEBUG']

    compiler_name = get_compiler_name(environment)
    if compiler_name is None:
        raise FileNotFoundError('C/C++ compiler could not be found')

    compiler_version = get_compiler_version(environment)
    if compiler_version is None:
        raise FileNotFoundError('C/C++ compiler could not be found')

    major_compiler_version = int(compiler_version[0])
    minor_compiler_version = int(compiler_version[1])

    # Check the library directory for the exact compiler we're using first,
    # this should be a hit for all but the sporadic precompiled library.
    matching_directory_name = _make_build_directory_name(
        environment, compiler_name, major_compiler_version, minor_compiler_version
    )
    matching_directory_path = os.path.join(library_builds_path, matching_directory_name)
    if os.path.isdir(matching_directory_path):
        return matching_directory_path

    # Build a regex by which compatible library build directories can be found
    # Example: '^(linux)-(clang|gcc)(\d+|\d+\.\d+)-(amd64)($|-(debug|release)$)'
    compatible_library_regex = _build_library_name_regex(environment)

    closest_major_difference = None
    closest_minor_version = None
    closest_build_type = None
    closest_directory = None

    # Now check all directories in the library directory and look for the best match
    # (primary concern is closest compiler version, secondary concern is build type match)
    for fileOrDir in os.listdir(library_builds_path):
        parts = re.match(compatible_library_regex, fileOrDir)
        if parts:
            if os.path.isdir(os.path.join(library_builds_path, fileOrDir)):
                version = parts[3].split('.')
                major_version = int(version[0])
                minor_version = int(0) if (len(version) == 1) else int(version[1])
                build_type = 'release' if (parts[6] is None) else str(parts[6])
                major_difference = abs(major_version - major_compiler_version)

                # If this is the first directory we check, take it blindly, otherwise check
                # if it's a closer match for the current compiler than we found so far.
                # Note that this all checks for "closer or equal" - in case of an equal
                # version, we'll then start comparing the build type (debug/release).
                if closest_major_difference is None:
                    is_closer_version = True
                else:

                    # Check if this version is closer to the closest version we have found
                    # up until now. If so, we'll take it as the new closest version
                    is_closer_version = (
                       (major_difference < closest_major_difference) or
                       (
                           (major_difference == closest_major_difference) and
                           (minor_version > closest_minor_version)
                       )
                    )

                    # Another path to a closer version if finding an equal version where
                    # the build type (debug/release) is a better fit
                    is_equal_version = (
                        (major_difference == closest_major_difference) and
                        (minor_version == closest_minor_version)
                    )
                    if is_equal_version:
                        is_closer_version = (
                            (is_debug_build and (build_type == 'debug')) or
                            (not is_debug_build) and (build_type == 'release')
                        )

                # If this directory promises to hold a better matching version of
                # the library, accept it as the new best version
                if is_closer_version:
                    closest_major_difference = major_difference
                    closest_minor_version  = minor_version
                    closest_build_type = build_type
                    closest_directory = fileOrDir

    # If a close match was found, display a warning and use it
    if not (closest_directory is None):
        print(
            '\033[93mWarning: no fitting binary for library "' + library_builds_path + '" ' +
            '(needed "' + matching_directory_name + '"), falling back to closest ' +
            'match, which is "' + closest_directory + '"\033[0m'
        )
        return os.path.join(library_builds_path, closest_directory)

    # No compiler-specified binaries, give the 'lib' dir a final try
    candidate = os.path.join(library_builds_path, 'lib')
    if os.path.isdir(candidate):
        print(
            '\033[93mWarning: no fitting binary for library "' + library_builds_path + '" ' +
            '(needed "' + matching_directory_name + '"), falling back to standard ' +
            '"lib" dir of unknown compiler and architecture.\033[0m'
        )
        return candidate

    # We failed.
    print(
        '\033[1;31mError: no fitting binary for library "' + library_builds_path + '" ' +
        '(needed "' + matching_directory_name + '") found. Giving up.\033[0m'
    )
    return None

# ----------------------------------------------------------------------------------------------- #

def get_platform_specific_library_name(universal_library_name, static = False):
    """Forms the platform-specific library name from a universal library name

    @param  universal_library_name  Universal name of the library that will be converted
    @param  static                  Whether the name is for a static library
    @returns The platform-specific library name
    @remarks
        A universal library name is just the name of the library without extension,
        using dots to separate words - for example My.Awesome.Stuff. Depending on the platform,
        this might get turned into My.Awesome.Stuff.dll or libMyAwesomeStuff.so"""

    if platform.system() == 'Windows':

        if static:
            return universal_library_name + '.lib'
        else:
            return universal_library_name + '.dll'

    else:

        # Linux tools automatically add 'lib' and '.a'/'.so'
        #return universal_library_name.replace('.', '')

        # ...but we want to have the actual filename so we can copy shared libraries
        # into the artifact directory of builds that have them as a dependency!
        if static:
            return 'lib' + universal_library_name.replace('.', '') + '.a'
        else:
            return 'lib' + universal_library_name.replace('.', '') + '.so'

# ----------------------------------------------------------------------------------------------- #

def get_platform_specific_executable_name(universal_executable_name):
    """Forms the platform-specific executable name from a universal executable name

    @param  universal_executable_name  Universal name of the executable that will be converted
    @returns The platform-specific executable name
    @remarks
        A universal executable name is just the name of the executable without extension,
        using dots to separate words - for example My.Awesome.Program. Depending on the platform,
        this might get turned into My.Awesome.Program.exe or MyAwesomeProgram."""

    if platform.system() == 'Windows':
        return universal_executable_name + '.exe'
    else:
        return universal_executable_name.replace('.', '')

# ----------------------------------------------------------------------------------------------- #

def get_compiler_name(environment):
    """Returns a short string identifying the compiler (or compiler group) being used

    @param  environment  Environment from which the compiler will be looked up
    @returns The name of the compiler (or shared name of a group compilers) being used"""

    compiler_executable = None

    if 'CXX' in environment:
        compiler_executable = environment['CXX']
        if compiler_executable == "$CC":
            compiler_executable = environment['CC']
    elif 'CC' in environment:
        compiler_executable = environment['CC']
    else:
        raise FileNotFoundError('No C/C++ compiler found')

    if (compiler_executable == 'cl') or (compiler_executable == 'icc'):
        return 'msvc'
    elif (compiler_executable == 'gcc') or (compiler_executable == 'g++'):
        return 'gcc'
    elif (compiler_executable == 'clang') or (compiler_executable == 'clang++'):
        return 'clang'
    else:
        return compiler_executable

# ----------------------------------------------------------------------------------------------- #

def get_compiler_version(environment):
    """Determines the version number of the C/C++ compiler being used

    @param  environment  Environment from which the C/C++ compiler executable will be looked up
    @returns The compiler version number, as an array of [Major, Minor, Revision]"""

    # If we already checked which compiler the user is running, just return the cached version
    if 'COMPILER_VERSION' in environment:
        return environment['COMPILER_VERSION']

    compiler_executable = None

    # Pick up the compiler executable provided to us by SCons
    if 'CXX' in environment:
        compiler_executable = environment['CXX']
        if compiler_executable == "$CC":
	          compiler_executable = environment['CC']
    elif 'CC' in environment:
        compiler_executable = environment['CC']
    else:
        raise FileNotFoundError('No C/C++ compiler found')

    # If it's the Microsoft compiler, do the acrobatics to figure out its version
    if (compiler_executable == 'cl') or (compiler_executable == 'icc'):
        if 'MSVC_VERSION' in environment:
            compiler_version = environment['MSVC_VERSION']
            return compiler_version.split('.')

        if 'MSVS' in environment:
            cl_install_directory = environment['MSVS']['VCINSTALLDIR']

        msvc_process = subprocess.Popen(
            [compiler_executable], stdout=subprocess.PIPE
        )
        (stdout, stderr) = msvc_process.communicate()

        compiler_version = re.search('[0-9][0-9.]*', str(stdout))

    else: # Figure out which versio nof GCC or clang is running
        gcc_process = subprocess.Popen(
            [compiler_executable, '--version'], stdout=subprocess.PIPE
        )
        (stdout, stderr) = gcc_process.communicate()

        compiler_version = re.search('[0-9][0-9.]*', str(stdout))

    # If no match is found the compiler didn't provide the expected output
    # and we have no idea which version it might be
    if compiler_version is None:
        return None

    environment['COMPILER_VERSION'] = compiler_version.group().split('.')
    return environment['COMPILER_VERSION']

# ----------------------------------------------------------------------------------------------- #

def _add_include_directory(environment, include_directory, system = False):
    """Adds an C/C++ include directory to the build

    @param  environment        Environment the C/C++ include directory will be added to
    @param  include_directory  Include directory that will be added
    @param  system             Whether this is a system header directory
                               (suppresses warnings for headers you have no control over)
    @remarks
        Consider using add_package() instead to automatically set up all include and
        library directories as well as link the libraries themselves."""

    if system:
        if platform.system() == 'Windows':
            # Supported in recent update to Visual Studio 2017
            # https://devblogs.microsoft.com/cppblog/broken-warnings-theory/
            environment.Append(CCFLAGS=('/external:I', include_directory))
        else:
            environment.Append(CCFLAGS=('-isystem', include_directory))
    else:
        environment.Append(CPPPATH=[include_directory])

# ----------------------------------------------------------------------------------------------- #

def _add_library_directory(environment, library_directory):
    """Adds a C/C++ library directory to the build

    @param  environment        Environment the C/C++ library directory will be added to
    @param  library_directory  Library directory that will be added
    @remarks
        Consider using add_package() instead to automatically set up all include and
        library directories as well as link the libraries themselves."""

    environment.Append(LIBPATH=[library_directory])

# ----------------------------------------------------------------------------------------------- #

def _add_library(environment, library_name):
    """Adds a C/C++ library to the build

    @param  environment   Environment the C/C++ library will be linked to
    @param  library_name  Name of the library that will be linked
    @remarks
        Consider using add_package() instead to automatically set up all include and
        library directories as well as link the libraries themselves.

        The libary name is platform-specific. On Windows systems, the library is usually
        called MyAwesomeThing.lib while on GNU/Linux systems, the convention is
        libMyAwesomeThing.so (the toolchain will automatically try the lib prefix, though)"""

    environment.Append(LIBS=[library_name])

# ----------------------------------------------------------------------------------------------- #

def _add_preprocessor_constant(environment, constant_name, constant_value = None):
    """Adds a C/C++ preprocessor constant to the build

    @param  environment     Environment the C/C++ preprocessor constant will be set in
    @param  constant_name   Name of the preprocessor constant that will be set
    @param  constant_value  Value that will be assigned to the constant"""

    if constant_value is None:
        environment.Append(CPPDEFINES=[constant_name])
    else:
        environment.Append(CPPDEFINES={constant_name: constant_value})

# ----------------------------------------------------------------------------------------------- #

def _get_build_directory_name(environment):
    """Determines the name of the build directory for the current compiler version
    and output settings (such as platform and whether it's a debug or release build)

    @param  environment  Environment for which the build directory will be determined
    @returns The name the build directory should have
    @remarks
        The build directory is a directory whose name uniquely identifies the compiler,
        platform and build configuration used. When shipping cross-platform libraries,
        for example, a compiled binary of the library can be shipped for each compiler,
        architecture and build configuration supported, allowing developers to pick
        the right one to link depending on their system."""

    compiler_name = get_compiler_name(environment)
    if compiler_name is None:
        raise FileNotFoundError("C/C++ compiler could not be found")

    compiler_version = get_compiler_version(environment)
    if compiler_version is None:
        raise FileNotFoundError("C/C++ compiler could not be found")

    return _make_build_directory_name(
        environment, compiler_name, compiler_version[0], compiler_version[1]
    )

# ----------------------------------------------------------------------------------------------- #

def _get_variant_directory_name(environment):
    """Determines the name of the variant directory for the current compiler version
    and output settings (such as platform and whether it's a debug or release build)
    plus the current build suffix.

    @param  environment  Environment for which the variant directory will be determined
    @returns The name the variant directory should have
    @remarks
        The variant directory typically matches the build directory, but may have
        an extra suffix if more than one code module is compiled (thus, the risk for
        overlapping build outputs exists)."""

    if 'INTERMEDIATE_SUFFIX' in environment:
        suffix = environment['INTERMEDIATE_SUFFIX']
        return _get_build_directory_name(environment) + '-' + suffix
    else:
        return _get_build_directory_name(environment)

# ----------------------------------------------------------------------------------------------- #

def _make_build_directory_name(
    environment, compiler_name, compiler_major_version, compiler_minor_version = None
):
    """Forms the build directory name given a compiler name, compiler version,
    target architecture and build configuration.

    @param  environment             Environment providing additional build settings
    @param  compiler_name           Name of the compiler that is being used
    @param  compiler_major_version  Major version number of the compiler that is being used
    @param  compiler_minor_version  Minor version number of the compiler that is being used
    @returns The build directory name for the specified compiler and architecture
    @remarks
        The build directory name is a short string uniquely identifying the target OS,
        processor architecture, compiler version and build type. It should be sufficient
        to discriminate between library builds to use when linking third-party libraries
        and to keep output directories non-overlapping even when compiling for multiple
        target platforms at the same time.

        Examples: 'linux-gcc7.1-amd64-release' or 'windows-msvc14.1-amd64-debug'"""

    if platform.system() == 'Windows':
        platform_name = 'windows'
    else:
        platform_name = 'linux'

    architecture = _get_architecture_or_default(environment)

    # Determine whether this is a debug builkd
    is_debug_build = False
    if 'DEBUG' in environment:
        is_debug_build = environment['DEBUG']

    # Append either 'debug' or 'release' depending on the build type
    if is_debug_build:
        build_configuration = 'debug'
    else:
        build_configuration = 'release'

    # The compiler has its version number appended to it. We can't predict
    # which compiler versions are interoperable, especially with LTO!
    compiler = compiler_name + str(compiler_major_version)
    if not (compiler_minor_version is None):
        compiler = compiler + '.' + str(compiler_minor_version)

    # Form the complete build directory name
    return (
        platform_name + '-' +
        compiler + '-' +
        architecture + '-' +
        build_configuration
    )

# ----------------------------------------------------------------------------------------------- #

def _get_architecture_or_default(environment):
    """Returns the current target architecture or the default architecture if none
    has been explicitly set.

    @param  environment  Environment the target architecture will be looked up from
    @returns The name of the target architecture from the environment or a default"""

    architecture = environment['TARGET_ARCH']
    if architecture is None:
        return 'amd64'
    else:
        return architecture

# ------------------------------------------------------------------------------------------- #

def _build_library_name_regex(environment):
    """Builds a regular expression that matches library directory names.
    
    @param  environment  SCons build environment provided additional information
    @remarks
        This regular expression can be used to extract compiler and version information
        from a library build directory name."""

    # If we already built the regex, reuse it
    if 'COMPATIBLE_LIBRARY_NAME_REGEX' in environment:
        return environment['COMPATIBLE_LIBRARY_NAME_REGEX']

    # Figure out if this is a debug build
    is_debug_build = False
    if 'DEBUG' in environment:
        is_debug_build = environment['DEBUG']

    # Match start of string. No other characters may be before the library name.
    libraryRegex = '^'

    # Require the same platform as the one we're currently building on
    if platform.system() == 'Windows':
        libraryRegex += '(windows)-('
    else:
        libraryRegex += '(linux)-('
    
    # Require a compatible compiler, but accept any version of that compiler
    libraryRegex += ('|').join(_get_compatible_compiler_tags(environment))
    libraryRegex += ')(\d+|\d+\.\d+)-('

    # The architecture must match exactly
    libraryRegex += _get_architecture_or_default(environment)
    libraryRegex += ')'

    # Debug builds can link both debug and release libraries (this is so third-party
    # libraries for which no debug build is available can be used)
    if is_debug_build:
        libraryRegex += '($|-(debug|release)$)'
    else: # Release builds can only link release libraries
        libraryRegex += '($|-(release)$)'

    environment['COMPATIBLE_LIBRARY_NAME_REGEX'] = libraryRegex
    return libraryRegex

# ----------------------------------------------------------------------------------------------- #

def _get_compatible_compiler_tags(environment):
    """Builds a list of compiler tags whose object file and library formats can
    be mixed with the currently used compiler

    @param  environment  Build environment providing additional information
    @returns A list of compiler tags that 
    @remarks
        This is used when linking libraries. On Linux, for example, libraries built by
        clang and GCC can be mixed so long as LTO is disabled. """

    if platform.system() == 'Windows':
        return [ 'msvc', 'icc' ]
    else:
        return [ 'gcc', 'clang' ]
