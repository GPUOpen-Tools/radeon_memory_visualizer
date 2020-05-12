# Known Issues

1.
   1. More than a single active device at a time within a single application is not supported. In this case, only the first device will be traced.
   2. More than a single Vulkan/DX12 process at a time is not supported.  Only the first started application will be traced.
2. In the Snapshots|Resource details pane, Physical memory mapped events may be shown before virtual allocate events.
3. Some of the Pane navigation shortcuts may conflict with the keyboard shortcuts used by the Radeon Settings (such as ALT-R). It is recommended to remap the Radeon settings so they don't conflict.
4. Some UI elements do not rescale properly when the OS's DPI scale settings are dynamically changed, or when dragging RMV between two monitors with different DPI scales. Close and re-open RMV to view at proper sizes.
5. Running multiple instances of the Radeon Developer Panel is not supported. This is also true when running the Radeon Developer Panel supplied with RMV at the same time as the Radeon Developer Panel supplied with the Radeon GPU Profiler.
6. Sparse texture are not fully supported.
