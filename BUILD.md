## Build instructions

### Download Source code

Clone the project radeon_memory_visualizer from github.com

git clone https://github.com/GPUOpen-Tools/radeon_memory_visualizer.git

### Building on Windows
As a preliminary step, make sure that you have the following installed on your system:
* CMake 3.11 or above.
* Python 3.7 or above.
* Qt® 5 or above (5.15.2 is the default and recommended).
* Visual Studio® 2015 or above (2019 is the default).

Qt V5.15.2 can be installed using the Qt online installer available from the Qt 5.15.2 release page here: https://www.qt.io/blog/qt-5.15.2-released
As an alternative, the Qt 5.12.6 offline installer can be used here: https://download.qt.io/archive/qt/5.12/5.12.6/
Qt should be installed to the default location (C:\Qt\Qt5.xx.x).
Be sure to select msvc2017/msvc2019 64-bit during Qt installation, depending on the compiler you decide to use.
A reboot is required after Qt is installed.

CMake can be downloaded from (https://cmake.org/download/).
Python (V3.x) can be downloaded from (https://www.python.org/). To build the documentation from Visual Studio, the Sphinx Python Document Generator is needed.
This can be installed once Python is installed, as follows:
* Open a command prompt and navigate to the scripts folder in the python install folder. Then type these 2 commands:
* pip install -U sphinx
* pip install sphinx_rtd_theme

Run the python pre_build.py script in the build folder from a command prompt. If no command line options are provided, the defaults will be used (Qt 5.15.2 and Visual Studio 2019)

Some useful options of the pre_build.py script:
* --vs <Visual Studio version>: generate the solution files for a specific Visual Studio version. For example, to target Visual Studio 2017, add --vs 2017 to the command.
* --qt <path>: full path to the folder from where you would like the Qt binaries to be retrieved. By default, CMake would try to auto-detect Qt on the system.

Once the script has finished, in the case of Visual Studio 2019, a sub-folder called 'vs2019' will be created containing the necessary build files.
Go into the 'vs2019' folder (build/win/vs2019) and double click on the RMV.sln file and build the 64-bit Debug and Release builds.
The Release and Debug builds of RMV will be available in the build/release and build/debug folders.

### Building on Ubuntu
Required dependencies can be installed as follows:

sudo apt-get update
sudo apt-get install build-essential python3 chrpath
sudo apt-get install python3-pip
pip install sphinx_rtd_theme
sudo snap install cmake --classic
sudo apt-get install git
sudo apt-get install python3-sphinx
sudo apt-get install libxcb-xinerama0
sudo apt-get install mesa-common-dev libglu1-mesa-dev

Qt V5.15.2 can be installed using the Qt online installer available from the Qt 5.15.2 release page here: https://www.qt.io/blog/qt-5.15.2-released
As an alternative, the Qt 5.12.6 offline installer can be used here: https://download.qt.io/archive/qt/5.12/5.12.6/ (the .run file) and installed
to ~/Qt/Qt5.12.6 (the default of ~/Qt5.12.6 will not work).

XCB libraries are required for Qt v5.15.x (they are not needed for older Qt versions). By default, the CMake configuration will attempt to copy
these files from the Qt lib folder. If these files are installed elsewhere on the system or an older version of Qt is being used to build RMV,
the --disable-extra-qt-lib-deploy pre_build.py script argument may be used. This will prevent the build configuration scripts from attempting to copy
the libraries in the post build step. If needed, the XCB library files (libxcb*) can be obtained from the /lib folder of the Radeon Developer Tool
Suite download found at https://gpuopen.com/tools/.

Run the python pre_build.py in the build folder.

$ python3 pre_build.py

Or run the pre_build.py script with the -qt option to specify another version of Qt (also use the --disable-extra-qt-lib-deploy flag since the XCB
libraries aren't needed). For example:

$ python3 pre_build.py --qt 5.12.6 --disable-extra-qt-lib-deploy

The pre_build.py script will construct the output folders and build the necessary makefiles.
To build the release build, use:

$ make -j5 -C linux/make/release

Similarly for the debug build, use:

$ make -j5 -C linux/make/debug

The pre_build.py script should only need to be used when adding or removing source files.

## DISCLAIMER ##
The information contained herein is for informational purposes only, and is subject to change without notice. While every
precaution has been taken in the preparation of this document, it may contain technical inaccuracies, omissions and typographical
errors, and AMD is under no obligation to update or otherwise correct this information. Advanced Micro Devices, Inc. makes no
representations or warranties with respect to the accuracy or completeness of the contents of this document, and assumes no
liability of any kind, including the implied warranties of noninfringement, merchantability or fitness for particular purposes, with
respect to the operation or use of AMD hardware, software or other products described herein. No license, including implied or
arising by estoppel, to any intellectual property rights is granted by this document. Terms and limitations applicable to the purchase
or use of AMD’s products are as set forth in a signed agreement between the parties or in AMD's Standard Terms and Conditions
of Sale.

AMD, the AMD Arrow logo, Radeon, Ryzen, RDNA and combinations thereof are trademarks of Advanced Micro Devices, Inc. Other product names used in
this publication are for identification purposes only and may be trademarks of their respective companies.

Visual Studio, DirectX and Windows are registered trademarks of Microsoft Corporation in the US and other jurisdictions.

Vulkan and the Vulkan logo are registered trademarks of the Khronos Group Inc.

Python is a registered trademark of the PSF. The Python logos (in several variants) are use trademarks of the PSF as well.

CMake is a registered trademark of Kitware, Inc.

Qt and the Qt logo are registered trademarks of the Qt Company Ltd and/or its subsidiaries worldwide.


© 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
