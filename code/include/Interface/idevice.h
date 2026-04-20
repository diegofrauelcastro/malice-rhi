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
	uint64_t GetMinUBOOffsetAlignment() const { return minUBOOffsetAlignment; }	// Get the minimum required alignment for uniform buffer offsets (for dynamic uniform buffers).


	/// Class specific methods ///

	// Wait until the device is idle.
	virtual void WaitIdle() = 0;
protected:
	// Class properties //

	uint64_t minUBOOffsetAlignment = 0;	// Minimum required alignment for uniform buffer offsets (for dynamic uniform buffers).
};