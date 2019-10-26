#!/usr/bin/env python

import os
import importlib
import platform
import types

from SCons.Environment import Environment
from SCons.Variables import Variables
from SCons.Variables import EnumVariable
from SCons.Variables import PathVariable
from SCons.Variables import BoolVariable
from SCons.Script import ARGUMENTS
from SCons.Script import Dir
from SCons.Util import WhereIs

# Nuclex SCons libraries
shared = importlib.import_module('shared')
cplusplus = importlib.import_module('cplusplus')
dotnet = importlib.import_module('dotnet')
blender = importlib.import_module('blender')
godot = importlib.import_module('godot')

# Inline stuff
#execfile('nuclex-cplusplus.py')


# Plan:
#   - if TARGET_ARCH is set, use it. For multi-builds,
#     this may result in failure, but that's okay
#   - multi-builds (typical case x86/x64 native + .net anycpu)
#     must control target arch manually anyway.

# ----------------------------------------------------------------------------------------------- #

def create_generic_environment():
    """Creates an general-purpose environment without specific support for any
    programming language or resource/asset system

    @return A new SCons environment without support for specific builds"""

    environment = Environment(
        variables = _parse_default_command_line_options()
    )

    _register_generic_extension_methods(environment)

    return environment

# ----------------------------------------------------------------------------------------------- #

def create_cplusplus_environment():
    """Creates a new environment with the required variables for building C/C++ projects

    @returns A new SCons environment set up for C/C++ builds"""

    environment = Environment(
        variables = _parse_default_command_line_options(),
        SOURCE_DIRECTORY = 'Source',
        HEADER_DIRECTORY = 'Include',
        TESTS_DIRECTORY = 'Tests',
        TESTS_RESULT_FILE = "gtest-results.xml",
        REFERENCES_DIRECTORY = 'ThirdParty'
    )

    # Extension methods from the C/C++ module
    cplusplus.setup(environment)

    # Nuclex standard build settings and extensions
    _set_standard_cplusplus_compiler_flags(environment)
    _set_standard_cplusplus_linker_flags(environment)
    _register_generic_extension_methods(environment)
    _register_cplusplus_extension_methods(environment)

    return environment

# ----------------------------------------------------------------------------------------------- #

def create_dotnet_environment():
    """Creates a new environment with the required variables for building .NET projects

    @returns A new scons environment set up for .NET builds"""

    environment = Environment(
        variables = _parse_default_command_line_options(),
        SOURCE_DIRECTORY = 'Source',
        TESTS_DIRECTORY = 'Tests',
        TESTS_RESULT_FILE = "nunit-results.xml",
        REFERENCES_DIRECTORY = 'References'
    )

    # Register extension methods and additional variables
    dotnet.setup(environment)

    _register_generic_extension_methods(environment)
    _register_dotnet_extension_methods(environment)

    return environment

# ----------------------------------------------------------------------------------------------- #

def create_blender_environment():
    """Creates a new environment with the required variables to export Blender models

    @returns A new scons environment set up for Blender exports"""

    environment = Environment(
        variables = _parse_default_command_line_options()
    )

    # Extension methods for Blender
    blender.setup(environment)

    _register_generic_extension_methods(environment)
    _register_blender_extension_methods(environment)

    return environment

# ----------------------------------------------------------------------------------------------- #

def create_godot_environment():
    """Creates a new environment with the required variables to export Godot games

    @returns A new scons environment set up for Godot exports"""

    environment = Environment(
        variables = _parse_default_command_line_options()
    )

    # Extension methods for Blender
    godot.setup(environment)

    _register_generic_extension_methods(environment)
    _register_godot_extension_methods(environment)

    return environment

# ----------------------------------------------------------------------------------------------- #

def build_all(environment, root_directory):
    """Compiles all SCons build scripts below the specified directory

    @param  environment     SCons environment that will be starting the nested builds
    @param  root_directory  Directory below which all SCons build scripts will be executed"""

    build_scripts = _get_all_build_scripts(root_directory)
    for build_script in build_scripts:
        environment.SConscript(build_script)

# ----------------------------------------------------------------------------------------------- #

def _get_all_build_scripts(root_directory):
    """Locates SCons build scripts in all directories below the specified root

    @param  root_directory  Directory below which SCons scripts will be collected
    @returns All SCons scripts below the specified root directory"""

    scripts = []

    for entry in os.listdir(root_directory):
        path = os.path.join(root_directory, entry)
        if os.path.isdir(path):
            _recursively_collect_build_scripts(scripts, path)

    #scripts.reverse()

    return scripts

# ----------------------------------------------------------------------------------------------- #

def _recursively_collect_build_scripts(scripts, directory):
    """Recursively searches for SCons build scripts and adds them to the provided list

    @param  scripts    List to which any discovered build scripts will be added
    @param  directory  Directory from which on the method will recursively search"""

    for entry in os.listdir(directory):
        path = os.path.join(directory, entry)
        if os.path.isdir(path):
            _recursively_collect_build_scripts(scripts, path)
        elif os.path.isfile(path):
            if ('SConstruct' in entry) or ('SConscript' in entry):
                scripts.append(path)


# ----------------------------------------------------------------------------------------------- #

def _parse_default_command_line_options():
    """Parses the command line options controlling various build settings

    @remarks
        This contains variables that work across all builds. Build-specific variables
        are discouraged, but would be irgnored by SCons' Variables class."""

    command_line_variables = Variables(None, ARGUMENTS)

    # Build configuration (also called build type in many SCons examples)
    command_line_variables.Add(
        BoolVariable(
            'DEBUG',
            'Whether to do an unoptimized debug build',
            False
        )
    )

    # Default architecture for the binaries. We follow the Debian practices,
    # which, while clueless and chaotic, are at least widely used.
    default_arch = 'amd64'
    if 'armv7' in platform.uname()[4]:
        default_arch = 'armhf'
    if 'aarch64' in platform.uname()[4]:
        default_arch = 'arm64'

    # CPU architecture to target
    command_line_variables.Add(
        EnumVariable(
            'TARGET_ARCH',
            'CPU architecture the binary will run on',
            default_arch,
            allowed_values=('armhf', 'arm64', 'x86', 'amd64', 'any')
        )
    )

    # Directory for intermediate files
    command_line_variables.Add(
        PathVariable(
            'INTERMEDIATE_DIRECTORY',
            'Directory in which intermediate build files will be stored',
            'obj',
            PathVariable.PathIsDirCreate
        )
    )

    # Directory for intermediate files
    command_line_variables.Add(
        PathVariable(
            'ARTIFACT_DIRECTORY',
            'Directory in which build artifacts (outputs) will be stored',
            'bin',
            PathVariable.PathIsDirCreate
        )
    )

    return command_line_variables

# ----------------------------------------------------------------------------------------------- #

def _register_generic_extension_methods(environment):
    """Registers general-purpose extension methodsinto a SCons environment

    @param  environment  Environment the extension methods will be registered to"""

    environment.AddMethod(_build_scons, 'build_scons')
    environment.AddMethod(_is_debug_build, 'is_debug_build')

# ----------------------------------------------------------------------------------------------- #

def _register_cplusplus_extension_methods(environment):
    """Registers extension methods for C/C++ builds into a SCons environment

    @param  environment  Environment the extension methods will be registered to"""

    environment.AddMethod(_add_cplusplus_package, 'add_package')
    environment.AddMethod(_add_cplusplus_project, 'add_project')
    environment.AddMethod(_add_cplusplus_source_directory, 'add_source_directory')
    environment.AddMethod(_build_cplusplus_library, 'build_library')
    environment.AddMethod(_build_cplusplus_unit_tests, 'build_unit_tests')
    environment.AddMethod(_build_cplusplus_executable, 'build_executable')
    environment.AddMethod(_run_cplusplus_unit_tests, 'run_unit_tests')

# ----------------------------------------------------------------------------------------------- #

def _register_dotnet_extension_methods(environment):
    """Registers extension methods for .NET builds into a SCons environment

    @param  environment  Environment the extension methods will be registered to"""

    environment.AddMethod(_build_msbuild_project, "build_project")
    environment.AddMethod(_build_msbuild_project_with_tests, "build_project_with_tests")

# ----------------------------------------------------------------------------------------------- #

def _register_blender_extension_methods(environment):
    """Registers extension methods for Blender exports into a SCons environment

    @param  environment  Environment the extension methods will be registered to"""

    #blender.register_extension_methods(environment)
    pass

# ----------------------------------------------------------------------------------------------- #

def _register_godot_extension_methods(environment):
    """Registers extension methods for Godot exports into a SCons environment

    @param  environment  Environment the extension methods will be registered to"""

    #godot.register_extension_methods(environment)
    pass

# ----------------------------------------------------------------------------------------------- #

def _is_debug_build(environment):
    """Checks whether a debug build has been requested

    @param  environment  Environment whose settings will be checked for a debug build
    @returns True if a debug build has been requested, otherwise False"""

    if 'DEBUG' in environment:
        return environment['DEBUG']
    else:
        return False

# ----------------------------------------------------------------------------------------------- #

def _set_standard_cplusplus_compiler_flags(environment):
    """Sets up standard flags for the compiler

    @param  environment  Environment in which the C++ compiler flags wlll be set."""

    if platform.system() == 'Windows':
        environment.Append(CFLAGS='/GF') # String pooling in debug and release
        #environment.Append(CFLAGS='/Gv') # Vectorcall for speed
        environment.Append(CFLAGS='/utf-8') # Source code and outputs are UTF-8 encoded
        environment.Append(CFLAGS='/W4') # Show all warnings
        environment.Append(CFLAGS='/GS-') # No buffer security checks (we make games!)
        environment.Append(CFLAGS='/fp:fast') # Allow floating point optimizations

        environment.Append(CXXFLAGS='/GF') # String pooling in debug and release
        #environment.Append(CXXFLAGS='/Gv') # Vectorcall for speed
        environment.Append(CXXFLAGS='/utf-8') # Source code and outputs are UTF-8 encoded
        environment.Append(CXXFLAGS='/W4') # Show all warnings
        environment.Append(CXXFLAGS='/GS-') # No buffer security checks (we make games!)
        environment.Append(CXXFLAGS='/fp:fast') # Allow floating point optimizations
        environment.Append(CXXFLAGS='/EHsc') # Only C++ exceptions, no Microsoft exceptions
        environment.Append(CXXFLAGS='/std:c++14') # Use a widely supported but current C++
        environment.Append(CXXFLAGS='/GR') # Generate RTTI for dynamic_cast and type_info

        if _is_debug_build(environment):
            environment.Append(CFLAGS='/Od') # No optimization for debugging
            environment.Append(CFLAGS='/MDd') # Link shared multithreaded debug runtime
            environment.Append(CFLAGS='/Zi') # Generate complete debugging information
            environment.Append(CFLAGS='/FS') # Support shared writing to the PDB file

            environment.Append(CXXFLAGS='/Od') # No optimization for debugging
            environment.Append(CXXFLAGS='/MDd') # Link shared multithreaded debug runtime
            environment.Append(CXXFLAGS='/Zi') # Generate complete debugging information
            environment.Append(CXXFLAGS='/FS') # Support shared writing to the PDB file
        else:
            environment.Append(CFLAGS='/O2') # Optimize for speed
            environment.Append(CFLAGS='/Oy') # Omit frame pointers
            environment.Append(CFLAGS='/Oi') # Enable intrinsic functions
            environment.Append(CFLAGS='/Gy') # Function-level linking for better trimming
            environment.Append(CFLAGS='/GL') # Whole program optimizaton (merged build)
            environment.Append(CFLAGS='/MD') # Link shared multithreaded release runtime
            environment.Append(CFLAGS='/Gw') # Enable whole-program *data* optimization

            environment.Append(CXXFLAGS='/O2') # Optimize for speed
            environment.Append(CXXFLAGS='/Oy') # Omit frame pointers
            environment.Append(CXXFLAGS='/Oi') # Enable intrinsic functions
            environment.Append(CXXFLAGS='/Gy') # Function-level linking for better trimming
            environment.Append(CXXFLAGS='/GL') # Whole program optimizaton (merged build)
            environment.Append(CXXFLAGS='/MD') # Link shared multithreaded release runtime
            environment.Append(CXXFLAGS='/Gw') # Enable whole-program *data* optimization

    else:
        environment.Append(CFLAGS='-fvisibility=hidden') # Default visibility: don't export
        environment.Append(CFLAGS='-Wpedantic') # Enable all ISO C++ deviation warnings
        environment.Append(CFLAGS='-Wall') # Show all common warnings
        environment.Append(CFLAGS='-Wextra') # Show extra warnings
        environment.Append(CFLAGS='-Wno-unknown-pragmas') # Don't warn about #pragma region
        #environment.Append(CFLAGS=['-flinker-output=pie']) # Position-independent executable
        environment.Append(CFLAGS='-shared-libgcc') # Use shared C/C++ runtime library
        environment.Append(CFLAGS='-fpic') # Use position-independent code
        environment.Append(CFLAGS='-funsafe-math-optimizations') # Allow float optimizations

        environment.Append(CXXFLAGS='-fvisibility=hidden') # Default visibility: don't export
        environment.Append(CXXFLAGS='-Wpedantic') # Enable all ISO C++ deviation warnings
        environment.Append(CXXFLAGS='-Wall') # Show all common warnings
        environment.Append(CXXFLAGS='-Wextra') # Show extra warnings
        environment.Append(CXXFLAGS='-Wno-unknown-pragmas') # Don't warn about #pragma region
        #environment.Append(CXXFLAGS=['-flinker-output=pie']) # Position-independent executable
        environment.Append(CXXFLAGS='-shared-libgcc') # Use shared C/C++ runtime library
        environment.Append(CXXFLAGS='-fpic') # Use position-independent code
        environment.Append(CXXFLAGS='-funsafe-math-optimizations') # Allow float optimizations
        environment.Append(CXXFLAGS='-std=c++14') # Use a widely supported but current C++
        environment.Append(CXXFLAGS='-fvisibility-inlines-hidden') # Inline code is also hidden

        if _is_debug_build(environment):
            #environment.Append(CFLAGS='-Og') # Tailor code for optimal debugging
            environment.Append(CFLAGS='-g3') # Generate debugging information
            environment.Append(CFLAGS='-ggdb') # Target the GDB debugger

            #environment.Append(CXXFLAGS='-Og') # Tailor code for optimal debugging
            environment.Append(CXXFLAGS='-g3') # Generate debugging information
            environment.Append(CXXFLAGS='-ggdb') # Target the GDB debugger
        else:
            environment.Append(CFLAGS='-O3') # Optimize for speed
            environment.Append(CFLAGS='-flto') # Merge all code before compiling

            environment.Append(CXXFLAGS='-O3') # Optimize for speed
            environment.Append(CXXFLAGS='-flto') # Merge all code before compiling

# ----------------------------------------------------------------------------------------------- #

def _set_standard_cplusplus_linker_flags(environment):
    """Sets up standard flags for the linker

    @param  environment  Environment in which the C++ compiler linker wlll be set."""

    if platform.system() == 'Windows':
        if _is_debug_build(environment):
            pass
        else:
            environment.Append(LINKFLAGS='/LTCG') # Merge all code before compiling
            environment.Append(LIBFLAGS='/LTCG') # Merge all code before compiling

    else:
        environment.Append(LINKFLAGS='-z defs') # Detect unresolved symbols in shared object
        environment.Append(LINKFLAGS='-Bsymbolic') # Prevent replacement on shared object syms
        environment.Append(LINKFLAGS='-flto') # Compile all code in one unit at link time

# ----------------------------------------------------------------------------------------------- #

def _build_scons(environment, source, arguments, target):
    """Builds another SCons script.

    @param  environment  Environment that will be used to search for the SCons executable
    @param  source       Input file(s) for the build
    @param  arguments    Arguments that will be passed to SCons
    @param  target       Output file(s) produced by the build
    @returns A scons build action producing the target file"""

    # Clone the environment and use the real search PATH. This will not pollute
    # the environment in which the SCons subprocess runs, but is the only way
    # to invoke SCons in Windows because it's a batch file that exepcts Python
    # to also be in the system search PATH.
    cloned_environment = environment.Clone(ENV=os.environ)

    # Try to locate SCons with the environment's new search path
    scons_path = cloned_environment.WhereIs('scons')

    # If not found, try with SCons.Util.WhereIs() which actually works differently
    # from environment.WhereIs()...
    if scons_path is None:
        scons_path = WhereIs('scons')

    # Still not found? Just blindly shout 'scons' and if it fails that will
    # at least produce a meaningful error message that might make the user add
    # SCons to the system search PATH.
    if scons_path is None:
        scons_path = 'scons'

    if platform.system() == 'Windows':
        return cloned_environment.Command(
            source = source,
            action = '"' + scons_path + '" ' + arguments,
            target = target
        )
    else:
        return cloned_environment.Command(
            source = source,
            action = scons_path + ' ' + arguments,
            target = target
        )

# ----------------------------------------------------------------------------------------------- #

def _add_cplusplus_package(environment, universal_package_name, universal_library_names = None):
    """Adds a precompiled package consisting of some header files and a code library
    to the current build.

    @param  environment              Environment to which a package will be added
    @param  universal_package_name   Name of the package that will be added to the build
    @param  universal_library_names  Names of libraries (inside the package) that need to
                                     be linked.
    @remarks
        If no universal_library_names are given, a library with the same name as
        the package is assumed. The universal_library_name can be used if a package
        offers multiple linkable library (i.e. boost modules, gtest + gtest_main)"""

    references_directory = os.path.join('..', environment['REFERENCES_DIRECTORY'])
    package_directory = os.path.join(references_directory, universal_package_name)

    # Path for the package's headers
    include_directory = cplusplus.find_or_guess_include_directory(package_directory)
    if include_directory is None:
        raise FileNotFoundError(
            'Could not find include directory for package in ' + package_directory
        )

    environment.add_include_directory(include_directory)

    # Path for the package's libraries
    library_directory = cplusplus.find_or_guess_library_directory(environment, package_directory)
    if library_directory is None:
        print('Retrying library with project layout instead of package layout...')
        library_directory = cplusplus.find_or_guess_library_directory(environment, os.path.join(package_directory, 'bin'))
        if library_directory is None:
            raise FileNotFoundError(
                'Could not find library directory for package in ' + package_directory
            )

    environment.add_library_directory(library_directory)

    # Library that needs to be linked
    if universal_library_names is None:
        environment.add_library(universal_package_name)
    elif isinstance(universal_library_names, list):
        for universal_library_name in universal_library_names:
            environment.add_library(universal_library_name)
    else:
        environment.add_library(universal_library_names)

# ----------------------------------------------------------------------------------------------- #

def _add_cplusplus_project(environment, project_directory, universal_package_name = None):
    """Adds another project (its include directory and expected build output if it is
    using this build script as well)

    @param  environment             Environment to which a package will be added
    @param  project_directory       Directory holding the project
    @param  universal_package_name  Name of the package produced by the project. If empty,
                                    the directory name is assumed to match the package name."""

    #project_directory = os.path.join('..', project_directory_name)

    # Path for the package's headers
    include_directory = cplusplus.find_or_guess_include_directory(project_directory)
    if include_directory is None:
        raise FileNotFoundError(
            'Could not find include directory for project in ' + project_directory
        )

    environment.add_include_directory(include_directory)

    # Path for the package's libraries
    project_artifact_directory = os.path.join(project_directory, environment['ARTIFACT_DIRECTORY'])
    library_directory = cplusplus.find_or_guess_library_directory(
        environment, project_artifact_directory
    )
    if library_directory is None:
        raise FileNotFoundError(
            'Could not find library directory for package in ' + project_directory
        )

    environment.add_library_directory(library_directory)

    # Library that needs to be linked
    project_directory_name = os.path.basename(project_directory)
    if universal_package_name is None:
        environment.add_library(
            cplusplus.get_platform_specific_library_name(project_directory_name, True)
        )
    elif isinstance(universal_package_name, list):
        for single_universal_package_name in universal_package_name:
            environment.add_library(
                cplusplus.get_platform_specific_library_name(single_universal_package_name, True)
            )
    else:
        environment.add_library(
            cplusplus.get_platform_specific_library_name(universal_package_name, True)
        )

# ----------------------------------------------------------------------------------------------- #

def _add_cplusplus_source_directory(
    environment, source_directory, sources = None,
    scons_issue_2908_workaround_needed = False
):
    """Adds a directory containing C/C++ source code files to the build.
    The directory is recursively scanned and compiled using a variant directory.

    @param  environment       SCons build environment holding the build settings
    @param  source_directory  Directory containing the source code files
    @param  source            Source code files that will be compiled. If 'None',
                              all C/C++ sources will be recursively search. If
                              specified, all sources must be in 'source_directory'
    @param  scons_issue_2908_workaround_needed  Enable this if you're explicitly specifying
                                                source files that are generated by yourself"""

    variant_sources = None
    if '_VARIANT_SOURCES' in environment:
        variant_sources = environment['_VARIANT_SOURCES']
    else:
        variant_sources = []

    # Append the build directory. This directory is unique per build setup,
    # so that debug/release and x86/amd64 builds can life side by side or happen
    # in parallel.
    intermediate_build_directory = os.path.join(
        environment['INTERMEDIATE_DIRECTORY'],
        environment.get_variant_directory_name()
    )
    variant_directory = os.path.join(intermediate_build_directory, source_directory)

    # Set up the variant directory so that object files get stored separately
    environment.VariantDir(variant_directory, source_directory, duplicate = 0)

    if sources is None:
        sources = cplusplus.enumerate_sources(source_directory)

    for file_path in sources:
        if not str(file_path).startswith(source_directory):
            raise ValueError(
                'Explicitly specified source file was not inside source directory'
            )

        variant_file_path = os.path.join(intermediate_build_directory, file_path)
        variant_sources.append(variant_file_path)

        # Bug where SCons forgets the dependency chain for files in a variant dir
        if scons_issue_2908_workaround_needed:
            environment.Depends(variant_file_path, file_path)

    environment['_VARIANT_SOURCES'] = variant_sources

# ----------------------------------------------------------------------------------------------- #

def _install_artifacts(environment, artifacts):
    artifact_directory = os.path.join(
        environment['ARTIFACT_DIRECTORY'],
        environment.get_build_directory_name()
    )

    # Library that needs to be linked
    if artifacts is None:
        raise FileNotFoundError('Artifact not found')
    elif isinstance(artifacts, list):
        #for artifact in artifacts:
        return environment.Install(artifact_directory, artifacts)
    else:
        return environment.Install(artifact_directory, artifacts)

# ----------------------------------------------------------------------------------------------- #

def _build_cplusplus_library(
  environment, universal_library_name, static = False
):
    """Creates a shared C/C++ library

    @param  environment             Environment controlling the build settings
    @param  universal_library_name  Name of the library in universal format
                                    (i.e. 'My.Awesome.Stuff')
    @param  static                  Whether to build a static library (default: no)
    @remarks
        Assumes the default conventions, i.e. all source code is contained in a directory
        named 'Source' and all headers in a directory named 'Include'.

        See get_platform_specific_library_name() for how the universal_library_name parameter
        is used to produce the output filename on different platforms."""

    environment = environment.Clone()

    # Include directories
    # These will automatically be scanned by SCons for changes
    if 'HEADER_DIRECTORY' in environment:
        environment.add_include_directory(environment['HEADER_DIRECTORY'])

    # Recursively search for the source code files or transform the existing file list
    if 'SOURCE_DIRECTORY' in environment:
        environment.add_source_directory(environment['SOURCE_DIRECTORY'])

    library_path = _put_in_intermediate_path(
        environment, cplusplus.get_platform_specific_library_name(universal_library_name, static)
    )

    if platform.system() == 'Windows':
        if _is_debug_build(environment):
            pdb_file_path = os.path.splitext(library_path)[0] + '.pdb'
            pdb_file_absolute_path = environment.File(pdb_file_path).srcnode().abspath
            environment.Append(CXXFLAGS='/Fd"' + pdb_file_absolute_path + '"')
            environment.Append(CFLAGS='/Fd"' + pdb_file_absolute_path + '"')

    # Grab the list of source files from the environment
    variant_sources = None
    if '_VARIANT_SOURCES' in environment:
        variant_sources = environment['_VARIANT_SOURCES']
    else:
        raise FileNotFoundError('No source files added to compile')

    # Build either a static or a shared library
    build_library = None
    if static:
        build_library = environment.StaticLibrary(library_path, variant_sources)
    else:
        build_library = environment.SharedLibrary(library_path, variant_sources)

    # If we're on Windows, a side effect of building a library in debug mode is
    # that a PDB file will be generated. Deal with that.
    if (platform.system() == 'Windows') and _is_debug_build(environment):
        build_debug_database = environment.SideEffect(pdb_file_absolute_path, build_library)
        return _install_artifacts(environment, build_library + build_debug_database)
    else:
        return _install_artifacts(environment, build_library)

# ----------------------------------------------------------------------------------------------- #

def _build_cplusplus_executable(
    environment, universal_executable_name, console = False
):
    """Creates a vanilla C/C++ executable

    @param  environment                Environment controlling the build settings
    @param  universal_executable_name  Name of the executable in universal format
                                       (i.e. 'My.Awesome.App')
    @param  console                    Whether to build a shell/command line executable
    @remarks
        Assumes the default conventions, i.e. all source code is contained in a directory
        named 'Source' and all headers in a directory named 'Include'.

        See get_platform_specific_executable_name() for how the universal_library_name
        parameter is used to produce the output filename on different platforms."""

    environment = environment.Clone()

    # Include directories
    # These will automatically be scanned by SCons for changes
    if 'HEADER_DIRECTORY' in environment:
        environment.add_include_directory(environment['HEADER_DIRECTORY'])

    # Recursively search for the source code files or transform the existing file list
    if 'SOURCE_DIRECTORY' in environment:
        environment.add_source_directory(environment['SOURCE_DIRECTORY'])

    executable_path = _put_in_intermediate_path(
        environment, cplusplus.get_platform_specific_executable_name(universal_executable_name)
    )

    if platform.system() == 'Windows':
        if _is_debug_build(environment):
            pdb_file_path = os.path.splitext(executable_path)[0] + '.pdb'
            pdb_file_absolute_path = environment.File(pdb_file_path).srcnode().abspath
            environment.Append(CXXFLAGS='/Fd"' + pdb_file_absolute_path + '"')
            environment.Append(CFLAGS='/Fd"' + pdb_file_absolute_path + '"')

        if console:
            environment.Append(LINKFLAGS='/SUBSYSTEM:CONSOLE')
        else:
            environment.Append(LINKFLAGS='/SUBSYSTEM:WINDOWS')

    else:
        environment.Append(CXXFLAGS='-fpie') # Build position-independent executable
        environment.Append(CFLAGS='-fpie') # Build position-independent executable

    # Grab the list of source files from the environment
    variant_sources = None
    if '_VARIANT_SOURCES' in environment:
        variant_sources = environment['_VARIANT_SOURCES']
    else:
        raise FileNotFoundError('No source files added to compile')

    # Build the executable
    build_executable = environment.Program(executable_path, variant_sources)
    if (platform.system() == 'Windows') and _is_debug_build(environment):
        build_debug_database = environment.SideEffect(pdb_file_absolute_path, build_executable)
        return _install_artifacts(environment, build_executable + build_debug_database)
    else:
        return _install_artifacts(environment, build_executable)

# ----------------------------------------------------------------------------------------------- #

def _build_cplusplus_unit_tests(
    environment, universal_executable_name
):
    """Creates a C/C++ executable that runs the unit tests contained in itself

    @param  environment                Environment controlling the build settings
    @param  universal_executable_name  Name of the library in universal format
                                       (i.e. 'My.Awesome.Stuff')"""

    environment.add_package('gtest', [ 'gtest', 'gtest_main' ])
    if not (platform.system() == 'Windows'):
        environment.add_library('pthread')

    environment['INTERMEDIATE_SUFFIX'] = 'tests'

    if 'TESTS_DIRECTORY' in environment:
        environment.add_source_directory(environment['TESTS_DIRECTORY'])

    return _build_cplusplus_executable(
        environment, universal_executable_name, console = True
    )

# ----------------------------------------------------------------------------------------------- #

def _run_cplusplus_unit_tests(environment, universal_test_executable_name):
    """Runs the unit tests executable comiled from a build_unit_test_executable() call

    @param  environment                     Environment used to locate the unit test executable
    @param  universal_test_executable_name  Name of the unit test executable from the build step
    @remarks
        This executes the unit test executable and produces an XML file detailing
        the test results for CI servers and other processing."""

    environment = environment.Clone()

    # Figure out the path the unit tests executable would have been compiled to
    test_executable_name = cplusplus.get_platform_specific_executable_name(
        universal_test_executable_name
    )

    test_executable_path = _put_in_artifact_path(
        environment, test_executable_name
    )

    test_results_path = None
    if 'TESTS_RESULT_FILE' in environment:
        test_results_path = _put_in_artifact_path(
            environment, environment['TESTS_RESULT_FILE']
        )
    else:
        test_results_path = _put_in_artifact_path(
            environment, 'gtest-results.xml'
        )

    return environment.Command(
        source = test_executable_path,
        action = '-$SOURCE --gtest_output=xml:$TARGET',
        target = test_results_path
    )

# ----------------------------------------------------------------------------------------------- #

def _build_msbuild_project(environment, msbuild_project_path):
    """Builds an MSBuild project

    @param  environment           Environment the MSBuild project will be compiled in
    @param  msbuild_project_path  Path to the MSBuild project file that will be built"""

    msbuild_project_file = environment.File(msbuild_project_path)
    dotnet_version_tag = dotnet.detect_msbuild_target_framework(msbuild_project_file)

    build_directory_name = environment.get_variant_directory_name(dotnet_version_tag)

    intermediate_build_directory = os.path.join(
        environment['INTERMEDIATE_DIRECTORY'], build_directory_name
    )

    return environment.MSBuild(
        msbuild_project_file.srcnode().abspath,
        intermediate_build_directory
    )

# ----------------------------------------------------------------------------------------------- #

def _build_msbuild_project_with_tests(
    environment, msbuild_project_path, tests_msbuild_project_path
):
    """Builds an MSBuild project and its associated unit test project

    @param  environment                 Environment the MSBuild project will be compiled in
    @param  msbuild_project_path        Path to the MSBuild project file that will be built
    @param  tests_msbuild_project_path  Path of the MSBuild project for the unit tests"""

    build_main = _build_msbuild_project(environment, msbuild_project_path)
    build_tests = _build_msbuild_project(environment, tests_msbuild_project_path)

    return build_main + build_tests

# ----------------------------------------------------------------------------------------------- #

def _put_in_intermediate_path(environment, filename):
    """Determines the intermediate path for a file with the specified name

    @param  environment  Environment for which the intermediate path will be determined
    @param  filename     Filename for which the intermediate path will be returned
    @returns The intermediate path for a file with the specified name"""

    intermediate_directory = os.path.join(
        environment['INTERMEDIATE_DIRECTORY'], environment.get_variant_directory_name()
    )

    return os.path.join(intermediate_directory, filename)

# ----------------------------------------------------------------------------------------------- #

def _put_in_artifact_path(environment, filename):
    """Determines the artifact path for a file with the specified name

    @param  environment  Environment for which the artifact path will be determined
    @param  filename     Filename for which the artifact path will be returned
    @returns The artifact path for a file with the specified name"""

    artifact_directory = os.path.join(
        environment['ARTIFACT_DIRECTORY'],
        environment.get_build_directory_name()
    )

    return os.path.join(artifact_directory, filename)

# ----------------------------------------------------------------------------------------------- #
