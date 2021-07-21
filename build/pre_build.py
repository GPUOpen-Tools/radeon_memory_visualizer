#! python3
# Copyright (c) 2020 Advanced Micro Devices, Inc. All rights reserved.
#
# Script to perform all necessary pre build steps. This includes:
#
#   - Fetching all dependencies
#   - Creating output directories
#   - Calling CMake with appropriate parameters to generate the build files
#
import os
import sys
import importlib.util
import argparse
import shutil
import subprocess
import distutils.spawn
import platform
import time

# Remember the start time
start_time = time.time()

# Enable/Disable options supported by this project
support_32_bit_build = False

# prevent fetch_dependency script from leaving a compiled .pyc file in the script directory
sys.dont_write_bytecode = True

# only import fetch_dependencies.py if the script exists
can_fetch = True
if importlib.util.find_spec("fetch_dependencies") is not None:
    import fetch_dependencies
else:
    can_fetch = False

# to allow the script to be run from anywhere - not just the cwd - store the absolute path to the script file
script_root = os.path.dirname(os.path.realpath(__file__))

# also store the basename of the file
script_name = os.path.basename(__file__)

if sys.platform == "win32":
    output_root = os.path.join(script_root, "win")
elif sys.platform == "darwin":
    output_root = os.path.join(script_root, "mac")
else:
    output_root = os.path.join(script_root, "linux")

# parse the command line arguments
parser = argparse.ArgumentParser(description="A script that generates all the necessary build dependencies for a project")
if sys.platform == "win32":
    parser.add_argument("--vs", default="2019", choices=["2017", "2019"], help="specify the version of Visual Studio to be used with this script (default: 2019)")
    parser.add_argument("--toolchain", default="2019", choices=["2017", "2019"], help="specify the compiler toolchain to be used with this script (default: 2019)")
    parser.add_argument("--qt-root", default="C:\\Qt", help="specify the root directory for locating QT on this system (default: C:\\Qt\\)")
    parser.add_argument("--qt-libver", default="2019", choices=["2017", "2019"], help="specify the Qt lib version to be used with this script (default: 2019)")
elif sys.platform == "darwin":
    parser.add_argument("--xcode", action="store_true", help="specify Xcode should be used as generator for CMake")
    parser.add_argument("--no-bundle", action="store_true", help="specify macOS application should be built as standard executable instead of app bundle")
    parser.add_argument("--qt-root", default="~/Qt", help="specify the root directory for locating QT on this system (default: ~/Qt) ")
else:
    parser.add_argument("--qt-root", default="~/Qt", help="specify the root directory for locating QT on this system (default: ~/Qt) ")
parser.add_argument("--qt", default="5.15.2", help="specify the version of QT to be used with the script (default: 5.15.2)" )
parser.add_argument("--clean", action="store_true", help="delete any directories created by this script")
parser.add_argument("--no-qt", action="store_true", help="build a headless version (not applicable for all products)")
parser.add_argument("--internal", action="store_true", help="configure internal builds of the tool (only used within AMD")
parser.add_argument("--disable-break", action="store_true", help="disable RGP_DEBUG_BREAK asserts in debug builds")
parser.add_argument("--update", action="store_true", help="Force fetch_dependencies script to update all dependencies")
parser.add_argument("--output", default=output_root, help="specify the output location for generated cmake and build output files (default = OS specific subdirectory of location of PreBuild.py script)")
parser.add_argument("--build", action="store_true", help="build all supported configurations on completion of prebuild step")
parser.add_argument("--build-jobs", default="4", help="number of simultaneous jobs to run during a build (default = 4)")
parser.add_argument("--analyze", action="store_true", help="perform static analysis of code on build (currently VS2017 only)")
if support_32_bit_build:
    parser.add_argument("--platform", default="x64", choices=["x64", "x86"], help="specify the platform (32 or 64 bit)")
args = parser.parse_args()

# Define the build configurations that will be generated
configs = ["debug", "release"]

# Generate the appropriate suffix to append for internal builds
internal_suffix = ""
if args.internal:
    internal_suffix = "-internal"

## Define some simple utility functions used lower down in the script

# Print a message to the console with appropriate pre-amble
def log_print(message):
    print ("\n" + script_name + ": " + message)

# Print an error message to the console with appropriate pre-amble then exit
def log_error_and_exit(message):
    print ("\nERROR: " + script_name + ": " + message)
    sys.stdout.flush()
    sys.exit(-1)

# Remove a directory and all subdirectories - printing relevant status
def rmdir_print(dir):
    log_print ("Removing directory - " + dir)
    if os.path.exists(dir):
        try:
            shutil.rmtree(dir)
        except Exception as e:
            log_error_and_exit ("Failed to delete directory - " + dir + ": " + str(e))
    else:
        log_print ("    " + dir + " doesn't exist!")

# Make a directory if it doesn't exist - print information
def mkdir_print(dir):
    if not os.path.exists(dir):
        log_print ("Creating Directory: " + dir)
        os.makedirs(dir)

if args.analyze and not args.build:
    log_error_and_exit("--analyze option requires the --build option to also be specified")

# check that the default output directory exists
mkdir_print(args.output)

# Define the output directory for CMake generated files
cmake_output_dir = None

if sys.platform == "win32":
    cmake_output_dir = os.path.join(args.output, "vs" + args.toolchain)
else:
    cmake_output_dir = os.path.join(args.output, "make")

if support_32_bit_build:
    # Always add platform to output folder for 32 bit builds
    if args.platform == "x86":
        cmake_output_dir += args.platform

if sys.platform == "win32":
    if args.internal:
        cmake_output_dir += internal_suffix

# Clean all files generated by this script or the build process
if (args.clean):
    log_print ("Cleaning build ...\n")
    # delete the CMake output directory
    rmdir_print(cmake_output_dir)
    # delete the build output directories
    for config in configs:
        if args.internal:
            config += internal_suffix
        dir = os.path.join(args.output, config)
        rmdir_print(dir)
    sys.exit(0)

# Call fetch_dependencies script
if can_fetch:
    log_print ("Fetching project dependencies ...\n")
    if (fetch_dependencies.do_fetch_dependencies(args.update, args.internal) == False):
        log_error_and_exit("Unable to retrieve dependencies")

# Create the CMake output directory
mkdir_print(cmake_output_dir)

# locate the relevant QT libraries
# generate the platform specific portion of the QT path name
if sys.platform == "win32":
    qt_leaf = "msvc" + args.qt_libver + "_64"
elif sys.platform == "darwin":
    qt_leaf = "clang_64"
else:
    qt_leaf = "gcc_64"

# Generate the full path to QT, converting path to OS specific form
# Look for Qt path in specified Qt root directory
# Example:
# --qt-root=C:\\Qt
# --qt=5.9.6
# Look first for C:\\Qt\\Qt5.9.6\\5.9.6
#  (if not found..)
# Look next for C:\\Qt\\5.9.6
#
# If neither of those can be found AND we are using the default
# qt-root dir (i.e. the user did not specify --qt-root), then also
# go up one directory from qt-root and check both permutations
# again. This allows the default Qt install path on Linux to be
# found without needing to specify a qt-root

qt_expanded_root = os.path.expanduser(args.qt_root)

qt_path_not_found_error = "Unable to find Qt root dir. Use --qt-root to specify\n    Locations checked:"

qt_path = os.path.normpath(qt_expanded_root + "/" + "Qt" + args.qt + "/" + args.qt)
if not os.path.exists(qt_path):
    qt_path_not_found_error = qt_path_not_found_error + "\n      " + qt_path
    qt_path = os.path.normpath(qt_expanded_root + "/" + args.qt)
    if not os.path.exists(qt_path):
        qt_path_not_found_error = qt_path_not_found_error + "\n      " + qt_path
        # if there is no user-specified qt-root, then check additional locations
        # used by the various Qt installers
        if args.qt_root == parser.get_default('qt_root'):
            qt_path = os.path.normpath(qt_expanded_root + "/../" + "Qt" + args.qt + "/" + args.qt)
            if not os.path.exists(qt_path):
                qt_path_not_found_error = qt_path_not_found_error + "\n      " + qt_path
                qt_path = os.path.normpath(qt_expanded_root + "/../" + args.qt)
                if not os.path.exists(qt_path):
                    qt_path_not_found_error = qt_path_not_found_error + "\n      " + qt_path
                    log_error_and_exit(qt_path_not_found_error)
        else:
            log_error_and_exit(qt_path_not_found_error)

qt_path = os.path.normpath(qt_path + "/"  + qt_leaf)

if not os.path.exists(qt_path) and not args.no_qt:
    log_error_and_exit ("QT Path does not exist - " + qt_path)

# Specify the type of Build files to generate
qt_generator = None
if sys.platform == "win32":
    if args.vs == "2019":
        qt_generator="Visual Studio 16 2019"
    else:
        qt_generator="Visual Studio 15 2017"
        if not support_32_bit_build or args.platform != "x86":
            qt_generator = qt_generator + " Win64"

elif sys.platform == "darwin":
    if args.xcode:
        qt_generator="Xcode"
    else:
        qt_generator="Unix Makefiles"
else:
    qt_generator="Unix Makefiles"

# Common code related to generating a build configuration
def generate_config(config):
    if (config != ""):
        cmake_dir = os.path.join(cmake_output_dir, config + internal_suffix)
        mkdir_print(cmake_dir)
    else:
        cmake_dir = cmake_output_dir

    cmakelist_path = os.path.join(script_root, os.path.normpath(".."))

    release_output_dir = os.path.join(args.output, "release" + internal_suffix)
    debug_output_dir = os.path.join(args.output, "debug" + internal_suffix)

    if args.no_qt:
        cmake_args = ["cmake", cmakelist_path, "-DHEADLESS=TRUE"]
    else:
        cmake_args = ["cmake", cmakelist_path, "-DCMAKE_PREFIX_PATH=" + qt_path, "-G", qt_generator]

    if sys.platform == "win32":
        if args.vs == "2019":
            if not support_32_bit_build or args.platform != "x86":
                cmake_args.extend(["-A" + "x64"])

            if args.toolchain == "2017":
                cmake_args.extend(["-Tv141"])

    if args.internal:
        cmake_args.extend(["-DINTERNAL_BUILD:BOOL=TRUE"])

    if args.disable_break:
        cmake_args.extend(["-DDISABLE_RGP_DEBUG_BREAK:BOOL=TRUE"])

    cmake_args.extend(["-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=" + release_output_dir])
    cmake_args.extend(["-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE=" + release_output_dir])
    cmake_args.extend(["-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG=" + debug_output_dir])
    cmake_args.extend(["-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG=" + debug_output_dir])

    if sys.platform == "win32":
        cmake_args.extend(["-Dgtest_force_shared_crt=true"])
    else:
        if "RELEASE" in config.upper():
            cmake_args.extend(["-DCMAKE_BUILD_TYPE=Release"])
        elif "DEBUG" in config.upper():
            cmake_args.extend(["-DCMAKE_BUILD_TYPE=Debug"])
        else:
            log_error_and_exit("unknown configuration: " + config)

    if sys.platform == "darwin":
            cmake_args.extend(["-DNO_APP_BUNDLE=" + str(args.no_bundle)])

    if not distutils.spawn.find_executable(cmake_args[0]):
        log_error_and_exit("cmake not found")

    p = subprocess.Popen(cmake_args, cwd=cmake_dir, stderr=subprocess.STDOUT)
    p.wait()
    sys.stdout.flush()
    if(p.returncode != 0):
        log_error_and_exit("cmake failed with %d" % p.returncode)

log_print("\nGenerating build files ...\n")
if sys.platform == "win32":
    # On Windows always generates both Debug and Release configurations in a single solution file
    generate_config("")
else:
    # For Linux & Mac - generate both Release and Debug configurations
    for config in configs:
        generate_config(config)

# Optionally, the user can choose to build all configurations on conclusion of the prebuild job
if (args.build):
    for config in configs:
        log_print( "\nBuilding " + config + " configuration\n")
        build_dir = ""

        cmake_args_docs = ""
        if sys.platform == "win32":
            build_dir = cmake_output_dir

            # For Visual Studio, specify the config to build
            cmake_args = ["cmake", "--build", build_dir, "--config", config, "--target", "ALL_BUILD", "--",  "/m:" + args.build_jobs]
            if args.analyze:
#                cmake_args.append("/p:CodeAnalysisTreatWarningsAsErrors=true")
#                cmake_args.append("/p:CodeAnalysisRuleSet=NativeRecommendedRules.ruleset")
                cmake_args.append("/p:CodeAnalysisRuleSet=NativeMinimumRules.ruleset")
                cmake_args.append("/p:RunCodeAnalysis=true")

            cmake_args_docs = ["cmake", "--build", build_dir, "--config", config, "--target", "Documentation", "--", "/m:" + args.build_jobs]
        else:
            # linux & mac use the same commands
            # generate the path to the config specific makefile
            build_dir = os.path.join(cmake_output_dir, config + internal_suffix)

            cmake_args = ["cmake", "--build", build_dir, "--parallel", args.build_jobs]

            cmake_args_docs = ["cmake", "--build", build_dir, "--config", config, "--target", "Documentation", "--parallel", args.build_jobs]

        p = subprocess.Popen(cmake_args, cwd=cmake_output_dir, stderr=subprocess.STDOUT)
        p.wait()
        sys.stdout.flush()

        if(p.returncode != 0):
            log_error_and_exit("CMake build failed with %d" % p.returncode)

        log_print( "\nBuilding Documentation\n")

        p = subprocess.Popen(cmake_args_docs, cwd=cmake_output_dir, stderr=subprocess.STDOUT)
        p.wait()
        sys.stdout.flush()

minutes, seconds = divmod(time.time() - start_time, 60)
log_print("Successfully completed in {0:.0f} minutes, {1:.1f} seconds".format(minutes,seconds))
sys.exit(0)
