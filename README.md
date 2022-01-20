# Radeon™ Memory Visualizer

The Radeon Memory Visualizer (RMV) is a software tool that will allow users to analyze video memory usage on AMD Radeon GPUs. RMV will reveal detailed information regarding an application’s video memory consumption and access patterns. This will allow users to understand how memory is being leveraged and open the door to new optimization opportunities.

## Getting Started

1. Install the latest AMD Video/display driver. Be sure to run DDU before installing the driver to ensure a clean install.
2. Unzip/Untar the download file. The directory contains the following:
   * Radeon Developer Service (RDS)
   * Radeon Developer Service CLI (RDS headless)
   * Radeon Developer Panel (RDP)
   * Radeon Memory Visualizer (RMV)
3. To gather a memory trace from a game, run the Radeon Developer Panel.
   * After making a connection, go to the 'SYSTEM' tab (this should be performed automatically for local connections).
   * Start your application. The Radeon Developer Panel will switch to the "APPLICATIONS" tab. The overlay window in the top left of the application being traced will show if RMV tracing is enabled. If not, make sure the app is running in Vulkan® or DirectX®12 mode.
   * When tracing is complete, click on the 'Memory Trace' tab and click "Dump Trace" or close your application (in this case, the trace will be dumped automatically).
4. For further detailed instructions, please see the documentation provided in the Help. Help can be found in the following locations:
   * Help web pages exist in the "docs" sub directory
   * Help web pages can be accessed from the **Help** button (?) in the Developer Panel
   * Help web pages can be accessed from the Welcome screen in the Radeon Memory Visualizer, or from the **Help** menu
   * The documentation is hosted publicly at:
      * http://radeon-developer-panel.readthedocs.io/en/latest/
      * http://radeon-memory-visualizer.readthedocs.io/en/latest/

## Supported ASICs

* AMD Radeon RX 6000 series
* AMD Radeon RX 5000 series
* AMD Radeon VII
* AMD RX Vega 64 and RX Vega 56
* AMD Ryzen™ Processors with Radeon Vega Graphics
* AMD Radeon R9 Fury and Nano series
* AMD Radeon RX 400, RX 500 series
* AMD Tonga R9 285, R9 380

## Supported OS's and API's

### Windows® 10/11
* DirectX12
* Vulkan

## Build instructions

### Download Source code

Clone the project radeon_memory_visualizer from github.com

git clone https://github.com/GPUOpen-Tools/radeon_memory_visualizer.git

### Building on Windows ###
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

Cmake can be downloaded from (https://cmake.org/download/).
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

## Support ##
For support, please visit the RMV repository github page: https://github.com/GPUOpen-Tools/radeon_memory_visualizer

## License ##
Radeon Memory Visualizer is licensed under the MIT license. See the License.txt file for complete license information.

## Copyright information ##
Please see NOTICES.txt for third party license information.

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
