#pragma once

#include <iostream>

// Forward declarations
class VulkanDescriptorSetsGroup;
class IDevice;
class IPipeline;
class ISwapChain;

// Descriptor pool/sets interface
class IDescriptorSetsGroup
{
public:
	// Class destructor
	virtual ~IDescriptorSetsGroup() = default;


	/// Lifetime methods ///

	virtual void Create(IDevice* _device, IPipeline* _pipeline, ISwapChain* _swapChain) = 0;
	virtual void Destroy(IDevice* _device) = 0;


	/// Retrieving the backend ///

	virtual VulkanDescriptorSetsGroup& API_Vulkan() { throw std::runtime_error("Bad API call : object is not a VulkanDescriptorSetsGroups."); }
};