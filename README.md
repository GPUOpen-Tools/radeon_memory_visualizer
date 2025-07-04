# Radeon™ Memory Visualizer

The Radeon Memory Visualizer (RMV) is a software tool that will allow users to analyze video memory usage on AMD Radeon GPUs. RMV will reveal detailed information regarding an application’s video memory consumption and access patterns. This will allow users to understand how memory is being leveraged and open the door to new optimization opportunities.

## Getting Started

1. Install the latest AMD Video/display driver. Completely remove previously installed drivers.  On Windows, the driver installation factory reset option should be used.
2. Unzip/Untar the download file. The directory includes the following:
   * Radeon Developer Service (RDS)
   * Radeon Developer Service CLI (RDS headless)
   * Radeon Developer Panel (RDP)
   * Radeon Memory Visualizer (RMV)
3. To gather a memory trace from a game, run the Radeon Developer Panel.
   * After making a connection, go to the 'SYSTEM' tab (this should be performed automatically for local connections).
   * Start your application. The Radeon Developer Panel will switch to the "APPLICATIONS" tab. The overlay window in the top left of the application being traced will show if RMV tracing is enabled. If not, make sure the app is running in Vulkan® or DirectX®12 mode.
   * When tracing is complete, click on the 'Memory Trace' tab and click "Dump Trace" or close your application (in this case, the trace will be dumped automatically).
4. For further detailed instructions, please see the documentation provided in the Help. Help can be found in the following locations:
   * Help web pages exist in the "help" sub directory
   * Help web pages can be accessed from the **Help** button (?) in the Developer Panel
   * Help web pages can be accessed from the Welcome screen in the Radeon Memory Visualizer, or from the **Help** menu
   * The documentation is hosted publicly at:
      * http://radeon-developer-panel.readthedocs.io/en/latest/
      * http://radeon-memory-visualizer.readthedocs.io/en/latest/

## Supported APIs
 * DirectX12
 * Vulkan

## Supported ASICs
* AMD Radeon RX 9000 series
* AMD Radeon RX 7000 series
* AMD Radeon RX 6000 series
* AMD Radeon RX 5000 series
* AMD Ryzen™ Processors with AMD RDNA™ Architecture Graphics

## Supported Operating Systems
* Windows® 10
* Windows® 11
* Ubuntu 22.04 LTS (Vulkan only)
* Ubuntu 24.04 LTS (Vulkan only)

Note: Before running RDP and capturing an RMV memory trace on Linux, be sure to make the necessary configuration changes by running the RDP setup.sh script each time the machine is rebooted. For more information, see the RDP documentation.

## Required hardware for SAM (Smart access memory) support
* AMD Radeon RX 6000 series or higher
* AMD Ryzen 5000 and higher series processors with AMD RDNA Architecture Graphics

## Build instructions
See [BUILD.md](BUILD.md) for more details.

## Support ##
For support, please visit the RMV repository github page: https://github.com/GPUOpen-Tools/radeon_memory_visualizer

## License ##
Radeon Memory Visualizer is licensed under the MIT license. See the [LICENSE.txt](LICENSE.txt) file for complete license information.

## Copyright information ##
Please see [NOTICES.txt](NOTICES.txt) for third party license information.

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


© 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
