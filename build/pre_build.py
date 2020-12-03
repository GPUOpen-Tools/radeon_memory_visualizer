#! /usr/bin/python
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
import argparse
import shutil
import subprocess
import distutils.spawn
import platform
import time

# Remember the start time
startTime = time.time()

# Enable/Disable options supported by this project
support32BitBuild = False

# prevent fetch_dependency script from leaving a compiled .pyc file in the script directory
sys.dont_write_bytecode = True

import fetch_dependencies

# to allow the script to be run from anywhere - not just the cwd - store the absolute path to the script file
scriptRoot = os.path.dirname(os.path.realpath(__file__))

# also store the basename of the file
scriptName = os.path.basename(__file__)

if sys.platform == "win32":
    outputRoot = os.path.join(scriptRoot, "win")
elif sys.platform == "darwin":
    outputRoot = os.path.join(scriptRoot, "mac")
else:
    outputRoot = os.path.join(scriptRoot, "linux")

# parse the command line arguments
parser = argparse.ArgumentParser(description="A script that generates all the necessary build dependencies for a project")
if sys.platform == "win32":
    parser.add_argument("--vs", default="2017", choices=["2017", "2019"], help="specify the version of Visual Studio to be used with this script (default: 2017)")
    parser.add_argument("--toolchain", default="2017", choices=["2017", "2019"], help="specify the compiler toolchain to be used with this script (default: 2017)")
    parser.add_argument("--qt-root", default="C:\\Qt", help="specify the root directory for locating QT on this system (default: C:\\Qt\\)")
elif sys.platform == "darwin":
    parser.add_argument("--xcode", action="store_true", help="specify Xcode should be used as generator for CMake")
    parser.add_argument("--no-bundle", action="store_true", help="specify macOS application should be built as standard executable instead of app bundle")
    parser.add_argument("--qt-root", default="~/Qt", help="specify the root directory for locating QT on this system (default: ~/Qt) ")
else:
    parser.add_argument("--qt-root", default="~/Qt", help="specify the root directory for locating QT on this system (default: ~/Qt) ")
parser.add_argument("--qt", default="5.12.6", help="specify the version of QT to be used with the script (default: 5.12.6)" )
parser.add_argument("--clean", action="store_true", help="delete any directories created by this script")
parser.add_argument("--no-qt", action="store_true", help="build a headless version (not applicable for all products)")
parser.add_argument("--internal", action="store_true", help="configure internal builds of the tool (only used within AMD")
parser.add_argument("--disable-break", action="store_true", help="disable RGP_DEBUG_BREAK asserts in debug builds")
parser.add_argument("--update", action="store_true", help="Force fetch_dependencies script to update all dependencies")
parser.add_argument("--output", default=outputRoot, help="specify the output location for generated cmake and build output files (default = OS specific subdirectory of location of PreBuild.py script)")
parser.add_argument("--build", action="store_true", help="build all supported configurations on completion of prebuild step")
parser.add_argument("--build-jobs", default="4", help="number of simultaneous jobs to run during a build (default = 4)")
if support32BitBuild:
    parser.add_argument("--platform", default="x64", choices=["x64", "x86"], help="specify the platform (32 or 64 bit)")
args = parser.parse_args()

# Define the build configurations that will be generated
configs = ["debug", "release"]

# Generate the appropriate suffix to append for internal builds
internalSuffix = ""
if args.internal:
    internalSuffix = "-Internal"

## Define some simple utility functions used lower down in the script

# Print a message to the console with appropriate pre-amble
def logPrint(message):
    print ("\n" + scriptName + ": " + message)

# Print an error message to the console with appropriate pre-amble then exit
def logErrorAndExit(message):
    print ("\nERROR: " + scriptName + ": " + message)
    sys.stdout.flush()
    sys.exit(-1)

# Remove a directory and all subdirectories - printing relevant status
def rmdirPrint(dir):
    logPrint ("Removing directory - " + dir)
    if os.path.exists(dir):
        try:
            shutil.rmtree(dir)
        except Exception as e:
            logErrorAndExit ("Failed to delete directory - " + dir + ": " + str(e))
    else:
        logPrint ("    " + dir + " doesn't exist!")

# Make a directory if it doesn't exist - print information
def mkdirPrint(dir):
    if not os.path.exists(dir):
        logPrint ("Creating Directory: " + dir)
        os.makedirs(dir)

# check that the default output directory exists
mkdirPrint(args.output)

# Define the output directory for CMake generated files
cmakeOutputDir = None

if sys.platform == "win32":
    cmakeOutputDir = os.path.join(args.output, "vs" + args.toolchain)
else:
    cmakeOutputDir = os.path.join(args.output, "make")

if support32BitBuild:
    # Always add platform to output folder for 32 bit builds
    if args.platform == "x86":
        cmakeOutputDir += args.platform

if sys.platform == "win32":
    if args.internal:
        cmakeOutputDir += internalSuffix

# Clean all files generated by this script or the build process
if (args.clean):
    logPrint ("Cleaning build ...\n")
    # delete the CMake output directory
    rmdirPrint(cmakeOutputDir)
    # delete the build output directories
    for config in configs:
        if args.internal:
            config += internalSuffix
        dir = os.path.join(args.output, config)
        rmdirPrint(dir)
    sys.exit(0)

# Call fetch_dependencies script
logPrint ("Fetching project dependencies ...\n")
if (fetch_dependencies.doFetchDependencies(args.update, args.internal) == False):
    logErrorAndExit("Unable to retrieve dependencies")

# Create the CMake output directory
mkdirPrint(cmakeOutputDir)

# locate the relevant QT libraries
# generate the platform specific portion of the QT path name
if sys.platform == "win32":
    qtLeaf = "msvc" + args.toolchain + "_64"
elif sys.platform == "darwin":
    qtLeaf = "clang_64"
else:
    qtLeaf = "gcc_64"

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

qtExpandedRoot = os.path.expanduser(args.qt_root)

qtPathNotFoundError = "Unable to find Qt root dir. Use --qt-root to specify\n    Locations checked:"

qtPath = os.path.normpath(qtExpandedRoot + "/" + "Qt" + args.qt + "/" + args.qt)
if not os.path.exists(qtPath):
    qtPathNotFoundError = qtPathNotFoundError + "\n      " + qtPath
    qtPath = os.path.normpath(qtExpandedRoot + "/" + args.qt)
    if not os.path.exists(qtPath):
        qtPathNotFoundError = qtPathNotFoundError + "\n      " + qtPath
        # if there is no user-specified qt-root, then check additional locations
        # used by the various Qt installers
        if args.qt_root == parser.get_default('qt_root'):
            qtPath = os.path.normpath(qtExpandedRoot + "/../" + "Qt" + args.qt + "/" + args.qt)
            if not os.path.exists(qtPath):
                qtPathNotFoundError = qtPathNotFoundError + "\n      " + qtPath
                qtPath = os.path.normpath(qtExpandedRoot + "/../" + args.qt)
                if not os.path.exists(qtPath):
                    qtPathNotFoundError = qtPathNotFoundError + "\n      " + qtPath
                    logErrorAndExit(qtPathNotFoundError)
        else:
            logErrorAndExit(qtPathNotFoundError)

qtPath = os.path.normpath(qtPath + "/"  + qtLeaf)

if not os.path.exists(qtPath) and not args.no_qt:
    logErrorAndExit ("QT Path does not exist - " + qtPath)

# Specify the type of Build files to generate
qtGenerator = None
if sys.platform == "win32":
    if args.vs == "2019":
        qtGenerator="Visual Studio 16 2019"
    else:
        qtGenerator="Visual Studio 15 2017"
        if not support32BitBuild or args.platform != "x86":
            qtGenerator = qtGenerator + " Win64"

elif sys.platform == "darwin":
    if args.xcode:
        qtGenerator="Xcode"
    else:
        qtGenerator="Unix Makefiles"
else:
    qtGenerator="Unix Makefiles"

# Common code related to generating a build configuration
def generateConfig(config):
    if (config != ""):
        cmakeDir = os.path.join(cmakeOutputDir, config + internalSuffix)
        mkdirPrint(cmakeDir)
    else:
        cmakeDir = cmakeOutputDir

    cmakelistPath = os.path.join(scriptRoot, os.path.normpath(".."))

    releaseOutputDir = os.path.join(args.output, "release" + internalSuffix)
    debugOutputDir = os.path.join(args.output, "debug" + internalSuffix)

    if args.no_qt:
        cmakeArgs = ["cmake", cmakelistPath, "-DHEADLESS=TRUE"]
    else:
        cmakeArgs = ["cmake", cmakelistPath, "-DCMAKE_PREFIX_PATH=" + qtPath, "-G", qtGenerator]

    if sys.platform == "win32":
        if args.vs == "2019":
            if not support32BitBuild or args.platform != "x86":
                cmakeArgs.extend(["-A" + "x64"])

            if args.toolchain == "2017":
                cmakeArgs.extend(["-Tv141"])

    if args.internal:
        cmakeArgs.extend(["-DINTERNAL_BUILD:BOOL=TRUE"])

    if args.disable_break:
        cmakeArgs.extend(["-DDISABLE_RGP_DEBUG_BREAK:BOOL=TRUE"])

    cmakeArgs.extend(["-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=" + releaseOutputDir])
    cmakeArgs.extend(["-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE=" + releaseOutputDir])
    cmakeArgs.extend(["-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG=" + debugOutputDir])
    cmakeArgs.extend(["-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG=" + debugOutputDir])

    if sys.platform != "win32":
        if "RELEASE" in config.upper():
            cmakeArgs.extend(["-DCMAKE_BUILD_TYPE=Release"])
        elif "DEBUG" in config.upper():
            cmakeArgs.extend(["-DCMAKE_BUILD_TYPE=Debug"])
        else:
            logErrorAndExit("unknown configuration: " + config)

    if sys.platform == "darwin":
            cmakeArgs.extend(["-DNO_APP_BUNDLE=" + str(args.no_bundle)])

    if not distutils.spawn.find_executable(cmakeArgs[0]):
        logErrorAndExit("cmake not found")

    p = subprocess.Popen(cmakeArgs, cwd=cmakeDir, stderr=subprocess.STDOUT)
    p.wait()
    sys.stdout.flush()
    if(p.returncode != 0):
        logErrorAndExit("cmake failed with %d" % p.returncode)

logPrint("\nGenerating build files ...\n")
if sys.platform == "win32":
    # On Windows always generates both Debug and Release configurations in a single solution file
    generateConfig("")
else:
    # For Linux & Mac - generate both Release and Debug configurations
    for config in configs:
        generateConfig(config)

# Optionally, the user can choose to build all configurations on conclusion of the prebuild job
if (args.build):
    for config in configs:
        logPrint( "\nBuilding " + config + " configuration\n")
        if sys.platform == "win32":
            # Define the path to the Visual Studio directory that contains the VsDevCmd.bat file
            vsDevPath = os.path.normpath ("C:/Program Files (x86)/Microsoft Visual Studio/2017/Professional/Common7/Tools/")
            vsDevCommand = "VsDevCmd.bat"
            msBuildCommand = "msbuild /nodeReuse:false /m:" + args.build_jobs + " /t:Build /p:Configuration=" + config + " /verbosity:minimal "

            # search in cmakeOutputDir for a Visual Studio solution file - we assume there is only 1 solution file in this directory

            solutionFile = None
            for file in os.listdir(cmakeOutputDir):
                if file.endswith('.sln'):
                    solutionFile = file

            if solutionFile == None:
                logErrorAndExit("Unable to find solution file in location: " + cmakeOutputDir)

            # Specify the solution to be used for the Windows build
            solution = os.path.normpath(os.path.join(cmakeOutputDir, solutionFile))

            # Build it
            # All 3 of these commands need to be called like this to ensure the environment propagates through to the final call to msbuild
            if os.system("c:" + " & cd " + vsDevPath + " & " + vsDevCommand + " & " + msBuildCommand + solution) != 0:
                logErrorAndExit(config + " build failed for " + solution)

            # Build the documentation
            documentationProject = os.path.join(cmakeOutputDir, "Documentation.vcxproj")
            if os.system("c:" + "& cd " + vsDevPath + " & " + vsDevCommand + " & " + msBuildCommand + documentationProject) != 0:
                logErrorAndExit(config + " build failed for " + documentationProject)

        else:
            # linux & mac use the same commands

            # generate the path to the config specific makefile
            makeDir = os.path.join(cmakeOutputDir, config + internalSuffix)

            makeArgs = ["make", "-j" + args.build_jobs, "-C", makeDir]

            # Build it
            logPrint ("Building configuration: " + config)
            p = subprocess.Popen(makeArgs, stderr=subprocess.STDOUT)
            p.wait()
            sys.stdout.flush()
            if(p.returncode != 0):
                logErrorAndExit("make failed with %d" % p.returncode)

            makeArgs.extend(["Documentation"])

            # Build the documentation
            logPrint ("Building documentation for configuration: " + config)
            p = subprocess.Popen(makeArgs, stderr=subprocess.STDOUT)
            p.wait()
            sys.stdout.flush()
            if(p.returncode != 0):
                logErrorAndExit("make Documentation failed with %d" % p.returncode)


minutes, seconds = divmod(time.time() - startTime, 60)
logPrint("Successfully completed in {0:.0f} minutes, {1:.1f} seconds".format(minutes,seconds))
sys.exit(0)
