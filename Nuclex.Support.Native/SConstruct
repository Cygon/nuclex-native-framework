#!/usr/bin/env python

import sys
import importlib
import os
import platform

# Nuclex SCons libraries
sys.path.append('../BuildSystem/scons')
nuclex = importlib.import_module('nuclex')

# ----------------------------------------------------------------------------------------------- #

use_gcc10 = False
use_clang = False

# ----------------------------------------------------------------------------------------------- #

# Standard C/C++ build environment with Nuclex extension methods
common_environment = nuclex.create_cplusplus_environment()

if use_gcc10:
    common_environment['CXX'] = 'g++-10.2.0'
    common_environment['CC'] = 'gcc-10.2.0'

if use_clang:
    common_environment['ENV'] = os.environ
    common_environment['CXX'] = 'clang++-11'
    common_environment['CC'] = 'clang-11'
    #common_environment.Append(LINKFLAGS='-flto')
    #common_environment.Append(LINKFLAGS='-fuse-ld=gold')

# Compile the main library
library_environment = common_environment.Clone()
library_binaries = library_environment.build_library('Nuclex.Support.Native')

# Compile the unit test executable
unit_test_environment = common_environment.Clone()
unit_test_environment.add_preprocessor_constant('NUCLEX_SUPPORT_EXECUTABLE')
unit_test_binaries = unit_test_environment.build_unit_tests(
    'Nuclex.Support.Native.Tests'
)

# ----------------------------------------------------------------------------------------------- #

artifact_directory = os.path.join(
        unit_test_environment['ARTIFACT_DIRECTORY'],
        unit_test_environment.get_build_directory_name()
)
unit_test_results = unit_test_environment.Command(
    source = unit_test_binaries,
    action = '-$SOURCE --gtest_color=yes --gtest_output=xml:$TARGET',
    target = os.path.join(artifact_directory, 'gtest-results.xml')
)

# ----------------------------------------------------------------------------------------------- #

AlwaysBuild(unit_test_results)

#if platform.system() != 'Windows':
#    list_exported_symbols = common_environment.Command(
#        source = library_binaries,
#        action = 'nm --demangle --extern-only $SOURCE | grep Nuclex',
#        target = None
#    )
    # On Windows, use dumpbin
