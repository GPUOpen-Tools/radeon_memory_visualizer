The Radeon™ Memory Visualizer (RMV)
===================================

The Radeon Memory Visualizer is a memory optimization tool that can be used by
developers to optimize memory usage for DirectX® 12, Vulkan™ applications for
AMD RDNA™ hardware.

This document describes how the Radeon Memory Visualizer can be used to
examine a memory trace file.

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   quickstart.rst
   settings.rst
   timeline.rst
   snapshot_windows.rst
   compare_windows.rst
   resource_naming.rst
   

Supported graphics APIs, RDNA hardware, and operating systems
-------------------------------------------------------------

**Supported APIs**

-  DirectX 12

-  Vulkan

\ **Supported RDNA hardware**

-  AMD Radeon RX 9000 series

-  AMD Radeon RX 7000 series

-  AMD Radeon RX 6000 series

-  AMD Radeon RX 5000 series

-  AMD Ryzen™ Processors with AMD RDNA™ Architecture Graphics


\ **Supported Operating Systems**

-  Windows® 10/11

-  Ubuntu® 24.04 LTS (Vulkan only)

 -  With the introduction of 25.20-based Linux drivers, the AMDVLK driver is no longer included in the amdgpu-pro driver package. This is a result of the AMDVLK open-source project being discontinued as mentioned `here <https://github.com/GPUOpen-Drivers/AMDVLK/discussions/416>`_. Instead, the RADV open-source Vulkan® driver is installed by default. Consequently, the Radeon Developer Panel does not support capturing data from Vulkan applications when using these newer driver releases. To analyze Linux Vulkan workloads with Radeon GPU Profiler (RGP), Radeon Raytracing Analyzer (RRA), or Radeon Memory Visualizer (RMV), users can opt for a 25.10-based driver. Alternatively, analysis can be performed using the data capture mechanism integrated within the RADV driver, although this method is not supported by the Radeon Developer Panel. For more information on configuring RADV, refer to the environment variable documentation, specifically the `MESA_VK_TRACE_* environment variables <https://docs.mesa3d.org/envvars.html#envvar-MESA_VK_TRACE>`_ which can be utilized for enabling and configuring tracing.


DISCLAIMER
----------
The information contained herein is for informational purposes only, and is subject to change without notice. While every
precaution has been taken in the preparation of this document, it may contain technical inaccuracies, omissions and typographical
errors, and AMD is under no obligation to update or otherwise correct this information. Advanced Micro Devices, Inc. makes no
representations or warranties with respect to the accuracy or completeness of the contents of this document, and assumes no
liability of any kind, including the implied warranties of noninfringement, merchantability or fitness for particular purposes, with
respect to the operation or use of AMD hardware, software or other products described herein. No license, including implied or
arising by estoppel, to any intellectual property rights is granted by this document. Terms and limitations applicable to the purchase
or use of AMD's products are as set forth in a signed agreement between the parties or in AMD's Standard Terms and Conditions
of Sale. 

AMD, the AMD Arrow logo, Radeon, Ryzen, RDNA and combinations thereof are trademarks of Advanced Micro Devices, Inc. Other product names used in
this publication are for identification purposes only and may be trademarks of their respective companies.

DirectX is a registered trademark of Microsoft Corporation in the US and other jurisdictions.

Vulkan and the Vulkan logo are registered trademarks of the Khronos Group Inc.

OpenCL is a trademark of Apple Inc. used by permission by Khronos Group, Inc.

Microsoft is a registered trademark of Microsoft Corporation in the US and other jurisdictions.

Windows is a registered trademark of Microsoft Corporation in the US and other jurisdictions.


© 2018-2025 Advanced Micro Devices, Inc. All rights reserved.

