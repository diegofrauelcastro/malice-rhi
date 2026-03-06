# MaliceRHI - Rendering Hardware Interface

## Summary

### Overview of the project
- [Overview](#overview)
- [How to Build](#how-to-build)
- [Architectural Summary](#architectural-summary)  
- [Most important features](#most-important-features)

### Usage Examples  
- [Example of User-Side Initialization](#example-of-user-side-initialization)

### Classes  
- [IRenderInterface / VulkanRenderInterface](#irenderinterface--vulkanrenderinterface)  
- [IInstance / VulkanInstance](#iinstance--vulkaninstance)  
- [ISurface / VulkanSurface](#isurface--vulkansurface)  
- [IDevice / VulkanDevice](#idevice--vulkandevice)  
- [ISwapChain / VulkanSwapChain](#iswapchain--vulkanswapchain)  
- [IRenderPass / VulkanRenderPass](#irenderpass--vulkanrenderpass)  
- [IFramebuffers / VulkanFramebuffers](#iframebuffers--vulkanframebuffers)  
- [IShaderModules / VulkanShaderModules](#ishadermodules--vulkanshadermodules)  
- [IPipeline / VulkanPipeline](#ipipeline--vulkanpipeline)  
- [IBuffer / VulkanBuffer](#ibuffer--vulkanbuffer)  
- [IUniformBuffers / VulkanUniformBuffers](#iuniformbuffers--vulkanuniformbuffers)  
- [IDescriptorSetsGroup / VulkanDescriptorSetsGroup](#idescriptorsetsgroup--vulkandescriptorsetsgroup)  
- [ICommandPool / VulkanCommandPool](#icommandpool--vulkancommandpool)  
- [ICommandBuffers / VulkanCommandBuffers](#icommandbuffers--vulkancommandbuffers)

### Issues
- [Known issues](#known-issues)

### Credits
- [Credits and Thanks](#credits-and-thanks)

## Overview

MaliceRHI is a student project that implements a small Rendering Hardware Interface (RHI) designed to abstract low-level graphics APIs.  
The goal is to let applications create and use rendering resources (buffers, pipelines, command buffers, descriptor sets, uniform buffers, etc.) without interacting with Vulkan directly.

All interactions go through minimalistic interface classes, and each interface has a Vulkan backend implementation.
This keeps user code clean and independent from the graphics API while still exposing the most explicit control over GPU resources.

Although only Vulkan is implemented for now, the design makes it possible to add other graphics APIs later. Please also note that this is our first Vulkan experience/project ever, coming from OpenGL training (modern and legacy versions).

The project is structured as a static library, and also includes a separate example project that uses it in a for test/demo purposes.
\
\
\
**Usability**
- This project is usable as-is, after building and compiling the project.
- This project is easily usable as a **github submodule** for any project that **also uses CMake**. Simply make sure to also be using **GLFW** for windowing, and don't forget to **include the subdirectory of MaliceRHI** in **your own** `CMakeLists.txt`

## How to Build

**Step-by-step guide :**

- **1) `git clone` this project from [the github repository](https://github.com/diegofrauelcastro/malice-rhi), wherever you want on your computer.**
  - Then, run the command `git submodule update --init --recursive` to install the **library dependencies** of MaliceRHI (GLFW and Volk).
  - **Note :** *Make sure your device supports Vulkan, or else this library will not work at all.*

- Next, refer to the section related to your current OS for the next steps.

### Windows

- **2) Download Vulkan SDK** from https://vulkan.lunarg.com/, and make sure to have the SDK set in **PATH**. If you already have Vulkan setup on your machine, skip this step.

- **3) Using Microsoft Visual Studio (with CMake) :**
  - Right-click open the cloned `malice-rhi/` project's folder with Visual Studio.
  - Wait for Visual Studio to finish building CMakeLists.
  - Toggle view **CMake Targets**
  - **If you wish to see the example project**, in the **root** `CMakeLists.txt`, enable **"Build tests" option**. Skip this if you just want to build the library alone.
  - Build the desired projects inside Visual Studio. ***Main library project : `MaliceRHI`***

- **3 bis) Using CMake commands**, if you know how to do it.

- **4) Inside the out/build directory generated, find the library/executable.** The main library should be located under `code/`, and the test executable should be inside `tests/MaliceFwdRenderer/`. If you used CMake commands to generate a VS Solution instead, open the newly generated `.sln` and compile the code. You're good to go.

### Linux

**Note :** Since I don't know much yet about Linux, I have only tried with **Debian, Ubuntu, and similar distros**. For other type of distributions, make sure to know what you're doing, because I don't.

- **2) Make sure to have the basic requirement packages installed :** `build-essential`, `cmake` and `libwayland-dev` *(for glfw)*.
  - If not, run `sudo apt install build-essential cmake libwayland-dev`.

- **3) To install Vulkan, run :**
  - `sudo apt install libvulkan-dev vulkan-tools`
  - Then `sudo apt install vulkan-validation layers` to install the necessary validation layers that MaliceRHI needs.

- **4) Run the commands in the following order :**
  - `cd malice-rhi` (unless you're already located there)
  - `mkdir build`
  - `cd build`
  - `cmake ..` <-- **If this fails**, there is probably something wrong with the packages you need. Check the given error description whenever that happens, solve it and look at point **4 bis)** before going to the next command.
  - `cmake --build .`
  - `make`

- **4 bis) If at any point of the build process an error appears**, it is often safe, after solving the issue, to delete the `build/` folder and recreate it. To do so, place yourself inside `malice-rhi/`, and run `rm -rf build`. Then, start again the build process described in point **4)**.

- **5) Inside the out/build directory generated, find the library/executable.** The main library should be located under `code/`, and the test executable should be inside `tests/MaliceFwdRenderer/`. You're good to go.

## Architectural Summary

```
(User Application)
       |
       v
  IRenderInterface
       |
       +--- IInstance              --> VulkanInstance
       +--- IDevice                --> VulkanDevice
       +--- ISurface               --> VulkanSurface
       +--- ISwapChain             --> VulkanSwapChain
       +--- IRenderPass            --> VulkanRenderPass
       +--- IFramebuffers          --> VulkanFramebuffers
       +--- IShaderModules         --> VulkanShaderModules
       +--- IPipeline              --> VulkanPipeline
       +--- ICommandPool           --> VulkanCommandPool
       +--- ICommandBuffers        --> VulkanCommandBuffers
       +--- IBuffer                --> VulkanBuffer
       +--- IUniformBuffers        --> VulkanUniformBuffers
       +--- IDescriptorSetsGroup   --> VulkanDescriptorSetsGroup
```

Each interface is implemented exactly once per backend. The user only interacts with the interface layer and remains entirely unaware of Vulkan objects.

---

![UML Class Diagram](docs/MaliceRHI_UMLdiagram.png "UML Class Diagram of Malice RHI")
Here is the UML Class Diagram of the project, with most important classes, methods per classes, and some dependencies like Volk and GLFW.

---

## Most important features

- Runtime selection of rendering backend (dynamic RHI)
- Explicit and constant object lifetimes (`Create(...)` / `Destroy(...)`)
- Separation of concerns across instance, device, pipeline, buffer, and descriptor management while simplifying as much as possible the verbosity of Vulkan, while giving the user some control on the backend settings
- Per-frame resource allocation where appropriate (uniform buffers, descriptor sets)
- User customizable graphics pipeline, uniforms and vertex input data
- Resizable window supported.

---

# Example of User-Side Initialization

```cpp
#include <malice_rhi/malice_rhi.h>

#include <GLFW/glfw3.h>       // Mandatory for window handling.
#include <glm/glm.hpp>        // Recommended for sending types to the shaders.

IRenderInterface* RHI = new VulkanRenderInterface();

IInstance* instance = RHI->InstantiateInstance();
instance->Create("Malice RHI");

ISurface* surface = RHI->InstantiateSurface();
surface->Create(instance, windowHandle);

IDevice* device = RHI->InstantiateDevice();
device->Create(instance, surface);

ISwapChain* swapChain = RHI->InstantiateSwapChain();
swapChain->Create(device, surface, windowHandle);

...

swapChain->Destroy(device);
RHI->DeleteSwapChain(swapChain);
swapChain = nullptr;

... 
```
**Note :** This RHI depends heavily on GLFW for the window handle. It is not currently possible to use any other way such as SDL or other.

Pipeline and draw example:
```cpp
// Acquire next image from the swap chain, from the current frame stored inside the command buffers.
uint32_t frame = m_Commands->GetCurrentFrame();
uint32_t img = m_SwapChain->AcquireNextImage(m_Device, frame);

commands->SetClearColor({0.0f, 0.0f, 0.0f, 0.0f});
commands->BeginDraw(renderPass, swapChain, framebuffers, imageIndex);
  commands->BindPipeline(pipeline);
  commands->BindDescriptorSets(pipeline, descriptorSets);
  commands->UpdateUniformBuffer(device, descriptorSets, uniformBuffer, 0, 0, 1);
  commands->DrawIndexed(indexCount, vertexBuffer, indexBuffer);
commands->EndDraw();

commands->SubmitAndPresent(device, swapChain, framebuffers, imageIndex);
```

For a full demo, look inside the folder `./tests/MaliceFwdRenderer/` to see more code.

---

# Classes

## IRenderInterface / VulkanRenderInterface

### Purpose
Acts as a factory for all RHI objects. The user obtains all rendering resources through this interface.

### Primary Responsibilities
```cpp
IInstance*            InstantiateInstance();
IDevice*              InstantiateDevice();
ISurface*             InstantiateSurface();
ISwapChain*           InstantiateSwapChain();
IRenderPass*          InstantiateRenderPass();
IFramebuffers*        InstantiateFramebuffers();
IPipeline*            InstantiatePipeline();
ICommandPool*         InstantiateCommandPool();
ICommandBuffers*      InstantiateCommandBuffers();
IBuffer*              InstantiateBuffer();
IUniformBuffers*      InstantiateUniformBuffers();
IDescriptorSetsGroup* InstantiateDescriptorSetsGroup();
IShaderModules*       InstantiateShaderModules();
```
As well as a function for each class to use C++'s `delete _ptr` on each pointer to deallocate them.

---

## IInstance / VulkanInstance

### Purpose
Represents the rendering API instance.

### Vulkan Implementation Contains
- `VkInstance`
- Debug messenger
- Validation layers
- Surface support utilities

---

## ISurface / VulkanSurface

### Purpose
Abstraction of the windowing surface.

### Vulkan Implementation Contains
- `VkSurfaceKHR`

---

## IDevice / VulkanDevice

### Purpose
Encapsulates the physical and logical Vulkan device.

### Vulkan Implementation Contains
- `VkPhysicalDevice`
- `VkDevice`
- Queue family indices
- Graphics and presentation queues

---

## ISwapChain / VulkanSwapChain

### Purpose
Represents the swap chain and frame presentation infrastructure.

### Vulkan Implementation Contains
- `VkSwapchainKHR`
- Swap chain images and image views
- Synchronization objects per frame (semaphores, fences)
- Swap chain configuration utilities

---

## IRenderPass / VulkanRenderPass

### Purpose
Describes the render pass used for draw operations.

### Notes
- Currently supports a single color attachment
- Depth is not yet implemented

---

## IFramebuffers / VulkanFramebuffers

### Purpose
Contains one framebuffer per swap chain image. Created after both swap chain and render pass.

---

## IShaderModules / VulkanShaderModules

### Purpose
Loads SPIR-V shader modules and provides both shader handles and pipeline input descriptions.\
**Note :** the user must provide compiled SPIR-V shaders, not simply the written shaders.

### Additional Responsibilities
- Vertex input attribute descriptions
  - Location
  - Variable data type
  - Offset from start of Vertex data.

```cpp
struct VertexInputLocationParams
{
	uint32_t location = 0;
	uint32_t memoryOffset = 0;
	EShaderDataType type = NONE;
};
```

Example registration of a Vertex containing a vec2 position and a vec3 color:
```cpp
uint32_t vertexTotalSize = sizeof(UserVertex);

VertexInputLocationParams posParams;
posParams.location = 0;
posParams.type = VEC2;
posParams.memoryOffset = offsetof(UserVertex, UserVertex::pos);

VertexInputLocationParams colorParams;
colorParams.location = 1;
colorParams.type = VEC3;
colorParams.memoryOffset = offsetof(UserVertex, UserVertex::color);

std::vector<VertexInputLocationParams> params = { posParams, colorParams };
```
The user must provide these parameters (total size of vertex and individual input settings) at the creation of the shaders.

- Descriptor set layout definitions, including:
  - Set index
  - Binding index
  - Descriptor count
  - Shader stage flags
  - Descriptor type

Example registration:
```cpp
shaders->AddDescriptorBinding(0, 0, 1, FRAGMENT_SHADER);
shaders->AddDescriptorBinding(0, 1, 1, VERTEX_SHADER);
shaders->AddDescriptorBinding(1, 0, 1, ALL);
```
The user can call these methods whenever, as long as it's before the pipeline creation after which the pipeline will record the state of the shaders and will stay fixed unless it is manually recreated.

---

## IPipeline / VulkanPipeline

### Purpose
Represents a fully configured graphics pipeline.

### Vulkan Implementation Contains
- `VkPipeline`
- `VkPipelineLayout`
- Descriptor set layouts derived from shader module bindings
- Vertex input configuration
- Almost fully customizable rasterizer settings (with default settings shown below), to be provided by the user upon pipeline creation. All the enums are abstract, and are later translated into their Vulkan equivalents : 

```cpp
struct PipelineParams
{
	ETopologyMode inputTopologyMode = TRIANGLE_LIST;
	EPolygonMode polygonMode = FILL;
	EFrontFace frontFace = COUNTER_CLOCKWISE;
	ECullMode cullingMode = CULL_BACK_FACE;
	float rasterizerLineWidth = 1.0f;
	bool enableRasterizerDiscard = false;
	bool enableDepthClamp = false;
	bool enableDepthBias = false;
	bool enableColorBlend = false;
	bool enablePrimitiveRestart = false;
};
```

---

## IBuffer / VulkanBuffer

### Purpose
General-purpose GPU buffer supporting:
- Vertex buffers
- Index buffers
- Staging buffers (internally)
- General device-local buffers

### Vulkan Implementation Contains
- `VkBuffer`
- `VkDeviceMemory`

---

## IUniformBuffers / VulkanUniformBuffers

### Purpose
Manages one uniform buffer per frame-in-flight.

### Features
- Persistent CPU mapping for each UBO
- Automated per-frame indexing
- Easy data upload interface:
```cpp
ubo->UploadData(ICommandBuffers* commandBuffers, uint32_t dataSize, const void* dataPtr);
```

---

## IDescriptorSetsGroup / VulkanDescriptorSetsGroup

### Purpose
Owns all descriptor sets and the descriptor pool for a given pipeline.

### Responsibilities
- Create descriptor pool
- Allocate descriptor sets per frame
- Used during commands recording to update/bind uniform buffers and send them to the GPU.

---

## ICommandPool / VulkanCommandPool

### Purpose
Owns and manages the Vulkan command pool from which command buffers are allocated.

---

## ICommandBuffers / VulkanCommandBuffers

### Purpose
Represents a set of command buffers, typically one per frame-in-flight.

### Supported Operations
- Begin/End render pass / recording commands.
- Bind graphics pipeline
- Bind descriptor sets
- Issue draw calls, with given index and vertex buffers
- Submit work and present images
- Clear color (background) change (RGBA float).
- Update uniform buffers at a given set+binding, with a given count for each binding (useful for lists/arrays)

Example usage:
```cpp
commands->BindDescriptorSets(pipeline, descriptorSets)
commands->BindUniformBuffer(device, descriptorSets, modelMat, 0, 0, 1);       // Set 0, binding 0, count = 1.
commands->BindUniformBuffer(device, descriptorSets, viewMat, 0, 1, 1);        // Set 0, binding 1, count = 1.
commands->BindUniformBuffer(device, descriptorSets, projMat, 0, 2, 1);        // Set 0, binding 2, count = 1.
commands->BindUniformBuffer(device, descriptorSets, mvpMatsStruct, 1, 0, 1);  // Set 1, binding 0, count = 1, even for a struct with several data inside.
```

---

# Known issues

- No depth test is currently made.
- Render pass only has one attachment.
- The descriptor sets were made with Vulkan implementation in mind, therefore it might not be scalable in other APIs, but I don't know in which cases specifically.
- The viewport space WILL be wrong when changing APIs because Vulkan has the up coordinate "upside-down" in comparison to OpenGL for instance.
- The window is resizable by default, and this is not changeable by the user. If a static window is wanted, that is not possible to setup because viewport and scissors have been coded in backend to be always dynamic.
- I wasn't sure about my own understanding of sync objects, and their use in other APIs, so I preferred keeping them under the hood to avoid overcomplexifying my debug sessions.
- The user must still choose manually in the first lines of the program between all of the Render Interfaces. (For now only Vulkan)

## Credits and Thanks

### Author

**[Diego FRAUEL CASTRO](https://github.com/diegofrauelcastro)** - Student ***ISART Digital - Promo 2029***

### Thanks

**[Maxime ROUFFET](https://github.com/mrouffet)** - Teacher & project reviewer


\
\
\
*Project started November 1st 2025*
\
*Document last updated March 6th 2026*