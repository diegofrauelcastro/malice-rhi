#pragma once

#include <iostream>

// Forward declarations
class VulkanUniformBuffers;
class IDevice;
class ICommandBuffers;
class ISwapChain;

// Uniform buffer interface
class IUniformBuffers
{
public:
    // Class destructor
    virtual ~IUniformBuffers() = default;

    /// Lifetime methods ///

    virtual void Create(IDevice* _device, ISwapChain* _swapChain, uint32_t _bufferSize) = 0;
    virtual void Destroy(IDevice* _device) = 0;


    /// Retrieving the backend ///

    // Retrieve backend for descriptor updates
    virtual VulkanUniformBuffers& API_Vulkan() { throw std::runtime_error("Bad API call : object is not a VulkanUniformBuffers."); }


    /// Class specific methods ///

    // Upload data to the uniform buffer, using the current frame stored in the command buffers.
    virtual void UploadData(ICommandBuffers* _commandBuffers, uint32_t _size, const void* _data) = 0;
};