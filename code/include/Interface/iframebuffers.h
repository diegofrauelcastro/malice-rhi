#pragma once

#include <iostream>


// Forward declarations
class VulkanFramebuffers;
class IDevice;
class ISwapChain;
class IRenderPass;

// Framebuffers interface
class IFramebuffers
{
public:
	// Class destructor
	virtual ~IFramebuffers() = default;


	/// Lifetime methods ///

	virtual void Create(IDevice* _device, ISwapChain* _swapChain, IRenderPass* _renderpass) = 0;
	virtual void Destroy(IDevice* _device) = 0;


	/// Retrieving the backend ///

	virtual VulkanFramebuffers& API_Vulkan() { throw std::runtime_error("Bad API call : object is not a VulkanFramebuffers."); }
};