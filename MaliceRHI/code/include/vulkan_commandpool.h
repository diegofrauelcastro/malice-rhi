#pragma once

#include "malicerhi_masterheader.h"
#include "icommandpool.h"

// Forward declarations
class VulkanDevice;

// Vulkan command pool class
class VulkanCommandPool : public ICommandPool
{
private:
	/// Class properties ///

	VkCommandPool commandPool;

	/// Helper functions ///

	// Create command pool.
	void CreateCommandPool(VulkanDevice& _device);

public:
	// Class destructor
	virtual ~VulkanCommandPool() = default;


	/// Lifetime methods ///

	void Create(IDevice* _device) override;
	void Destroy(IDevice* _device) override;


	/// Retrieving the backend ///

	VulkanCommandPool& API_Vulkan() override { return (*this); }
	VkCommandPool GetVkHandle() const { return commandPool; }
};