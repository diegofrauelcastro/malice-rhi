#pragma once

#include "malicerhi_masterheader.h"
#include "iframebuffers.h"


// Forward declarations
class VulkanDevice;
class VulkanSwapChain;
class VulkanRenderPass;

// Framebuffers interface
class VulkanFramebuffers : public IFramebuffers
{
private:
	/// Class properties ///

	std::vector<VkFramebuffer> framebuffers;


	/// Helper functions ///

	// Create framebuffers.
	void CreateFramebuffers(VulkanDevice& _device, VulkanSwapChain& _swapChain, VulkanRenderPass& _renderPass);

public:
	/// Lifetime methods ///

	void Create(IDevice* _device, ISwapChain* _swapChain, IRenderPass* _renderPass) override;
	void Destroy(IDevice* _device) override;


	/// Retrieving the backend ///

	VulkanFramebuffers& API_Vulkan() override { return (*this); }
	std::vector<VkFramebuffer> GetVkHandles() const { return framebuffers; }
};