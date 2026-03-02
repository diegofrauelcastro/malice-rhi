#pragma once

#include <iostream>


// Forward declarations
class IInstance;
class ISurface;
class VulkanDevice;

// Device interface
class IDevice
{
public:
	// Class destructor
	virtual ~IDevice() = default;


	/// Lifetime methods ///

	virtual void Create(IInstance* _instance, ISurface* _surface) = 0;
	virtual void Destroy() = 0;
	

	/// Retrieving the backend ///

	virtual VulkanDevice& API_Vulkan() { throw std::runtime_error("Bad API call : object is not a VulkanDevice."); }


	/// Class specific methods ///

	// Wait until the device is idle.
	virtual void WaitIdle() = 0;
};