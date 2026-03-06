#pragma once

#include "Interface/malicerhi_masterheader.h"
#include "Interface/irenderpass.h"

// Forward declarations
class VulkanSwapChain;
class VulkanDevice;

// Render pass class
class VulkanRenderPass : public IRenderPass
{
private:
	/// Class properties ///
	
	VkRenderPass renderPass;


	/// Helper functions ///

	// Create Render Pass.
	void CreateRenderPass(VulkanDevice& _device, VulkanSwapChain& _swapchain);

public:
	// Class destructor
	virtual ~VulkanRenderPass() override = default;


	/// Lifetime methods ///

	void Create(IDevice* _device, ISwapChain* _swapChain) override;
	void Destroy(IDevice* _device) override;


	/// Retrieving the backend ///

	VulkanRenderPass& API_Vulkan() override { return (*this); }
	VkRenderPass GetVkHandle() const { return renderPass; }

};