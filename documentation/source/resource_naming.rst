Resource naming
===============

Vulkan resource naming
----------------------
Pipelines, images and buffers can be given unique names and these names will
show up in the RMV UI. The Vulkan extension VK_EXT_debug_utils can be used for this.

For more information, see the document:

https://www.lunarg.com/wp-content/uploads/2018/05/Vulkan-Debug-Utils_05_18_v1.pdf

specifically the section "Naming Objects"

DirectX 12 resource naming
--------------------------
Memory traces for DirectX applications captured with the Radeon Developer Panel
can include unique names for resources like heaps, buffers and textures.  Names can be assigned to
DirectX resources using the ``ID3D12Object::SetName()`` method.
For more information, please review the Microsoft DirectX 12 documentation.

Calling the ``SetName()``
method results in ETW (Event Tracing for Windows) events being emitted and picked
up by the Panel. This resource naming information is then included in the RMV trace
file.

Due to the asynchronous nature of the event tracing, memory events that happen a short time before the process exit
may show up incorrectly in RMV in terms of naming, as well as
marking and filtering out implicit heaps (created for committed resources)
and implicit buffers (that D3D12 runtime creates automatically for every explicitly created heap).
To overcome this problem, it is recommended to introduce a delay of few seconds
between memory events of interest and the process exit.

Viewing resource names
----------------------
The resource names will show up in the RMV UI in the resource list pane, for example:

.. image:: media/vk_resource_naming_1.png

