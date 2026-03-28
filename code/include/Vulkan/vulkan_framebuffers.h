#pragma once

#include "Interface/malicerhi_masterheader.h"
#include "Interface/iframebuffers.h"

// Forward declarations
class VulkanDevice;
class VulkanSwapChain;
class VulkanRenderPass;

// Framebuffers class
class VulkanFramebuffers : public IFramebuffers
{
private:
	/// Class properties ///

	std::vector<VkFramebuffer> framebuffers;

	VkRenderPass renderPass;
	VulkanSwapChain* sc = nullptr;

	/// Helper functions ///

	// Create framebuffers.
	void CreateFramebuffers(VulkanDevice& _device, VkRenderPass _renderPass);

public:
	// Class destructor
	virtual ~VulkanFramebuffers() override = default;


	/// Lifetime methods ///

	// Direct screen rendering. For offscreen rendering, use Create() with FramebufferParams instead of an ISwapChain.
	void Create(IDevice* _device, IRenderPass* _renderPass, ISwapChain* _swapChain, ITexture* _depthTex) override;
	// Flexible creation of framebuffers, can be used for offscreen rendering.
	void Create(IDevice* _device, IRenderPass* _renderPass, const FramebufferParams& _params) override;
	void Destroy(IDevice* _device) override;


	/// Retrieving the backend ///

	VulkanFramebuffers& API_Vulkan() override { return (*this); }
	std::vector<VkFramebuffer> GetVkHandles() const { return framebuffers; }

	/// Class specific methods ///

	// Recreate framebuffers.
	void Recreate(IDevice* _device, ISwapChain* _swapChain);
};