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
	VulkanSwapChain* sc = nullptr;


	/// Helper functions ///

	// Create Render Pass.
	void CreateRenderPass(VulkanDevice& _device);

public:
	// Class destructor
	virtual ~VulkanRenderPass() override = default;


	/// Lifetime methods ///

	// Create a RenderPass with a SwapChain to present on screen. To create an offscreen RenderPass, use Create() with RenderPassParams instead of a SwapChain.
	void Create(IDevice* _device, ISwapChain* _swapChain, bool _hasDepth) override;
	// Create a RenderPass manually to be able to render offscreen.
	void Create(IDevice* _device, const RenderPassParams& _params) override;
	void Destroy(IDevice* _device) override;


	/// Retrieving the backend ///

	VulkanRenderPass& API_Vulkan() override { return (*this); }
	VkRenderPass GetVkHandle() const { return renderPass; }

};