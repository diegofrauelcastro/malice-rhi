#pragma once

#include "Interface/malicerhi_masterheader.h"
#include "Interface/imgui_support.h"

class VulkanMaliceToImGuiBridge : public IMaliceToImGuiBridge
{
private:
	/// Class Properties ///

	VkDescriptorPool imguiDescPool = VK_NULL_HANDLE;

public:
	// Class destructor
	virtual ~VulkanMaliceToImGuiBridge() override = default;


	/// Lifetime methods ///

	void Create(IInstance* _instance, IDevice* _device, ISwapChain* _swapChain, IRenderPass* _renderPass, IFramebuffers* _framebuffers) override;
	void Destroy(IDevice* _device) override;


	/// Retrieving the backend ///

	virtual VulkanMaliceToImGuiBridge& API_Vulkan() { return *this; }
	VkDescriptorPool GetDescriptorPool() const { return imguiDescPool; }
};