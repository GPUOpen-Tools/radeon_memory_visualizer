# Radeon™ Memory Visualizer

The Radeon Memory Visualizer (RMV) is a software tool that will allow users to analyze video memory usage on AMD Radeon GPUs.  RMV will reveal detailed information regarding an application’s video memory consumption and access patterns. This will allow both internal and external engineers to understand how memory is being leveraged and open the door to new optimization opportunities.

## Getting Started

1. Install the latest AMD Video/display driver with RMV support. Be sure to run DDU before installing the driver to ensure a clean install.
2. Unzip/Untar the download file. The directory contains the following:
 * Radeon Developer Service (RDS)
 * RadeonDeveloperServiceCLI (RDS headless)
 * Radeon Developer Panel (RDP)
 * Radeon Memory Visualizer (RMV)
3. To gather a memory trace from a game, run the Radeon Developer Panel.
 a. After making a connection, go to the 'SYSTEM' tab and add the name of the executable to be traced to the list in "My applications".
 b. Start your application. The Radeon Developer Panel will switch to the "APPLICATIONS" tab. The overlay window in the top left of the application being traced will show if RMV tracing is enabled. If not, make sure the app is running in Vulkan or DX12 mode.
 c. When tracing is complete, click "Dump trace" or close your application (in this case, the trace will be dumped automatically).
4. For further detailed instructions, please see the documentation provided in the Help. Help can be found in the following locations:
- * Help web pages exist in the "docs" sub directory
- * Help web pages can be accessed from the **Help** button (?) in the Developer Panel
- * Help web pages can be accessed from the Welcome screen in the Radeon Memory Visualizer, or from the **Help** menu

## Supported ASICs

* AMD Radeon RX 5000 series
* AMD Radeon VII
* AMD RX Vega 64 and RX Vega 56
* AMD Ryzen™ Processors with Radeon Vega Graphics
* AMD Radeon R9 Fury and Nano series
* AMD Radeon RX 400, RX 500 series
* AMD Tonga R9 285, R9 380

## Supported OS's and API's

### Windows10 only
* DirectX12
* Vulkan
