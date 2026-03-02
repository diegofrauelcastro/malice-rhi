#pragma once

#include <iostream>


// Forward declarations
class VulkanCommandPool;
class IDevice;

// Command pool interface
class ICommandPool
{
public:
	// Class destructor
	virtual ~ICommandPool() = default;


	/// Lifetime methods ///

	virtual void Create(IDevice* _device) = 0;
	virtual void Destroy(IDevice* _device) = 0;


	/// Retrieving the backend ///

	virtual VulkanCommandPool& API_Vulkan() { throw std::runtime_error("Bad API call : object is not a VulkanCommandPool."); }

};