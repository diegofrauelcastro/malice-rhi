#include "vulkan_framebuffers.h"

#include "vulkan_device.h"
#include "vulkan_renderpass.h"
#include "vulkan_swapchain.h"


void VulkanFramebuffers::CreateFramebuffers(VulkanDevice& _device, VulkanSwapChain& _swapChain, VulkanRenderPass& _renderPass)
{
	// Resize to hold all framebuffers (one for each ImageView in our swap chain images).
	framebuffers.resize(_swapChain.GetImageViewsVkHandle().size());
	// Iterate through every image view and create a framebuffer for each.
	for (size_t i = 0; i < _swapChain.GetImageViewsVkHandle().size(); i++)
	{
		VkImageView attachments[] = {
			_swapChain.GetImageViewsVkHandle()[i]
		};

		// Info for this framebuffer.
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = _renderPass.GetVkHandle();
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = _swapChain.GetImageExtent().width;
		framebufferInfo.height = _swapChain.GetImageExtent().height;
		framebufferInfo.layers = 1;

		// Create the framebuffer.
		VkResult result = vkCreateFramebuffer(_device.GetLogicalDeviceVkHandle(), &framebufferInfo, nullptr, &framebuffers[i]);
		if (result != VK_SUCCESS)
			throw std::runtime_error("/!\\ Failed to create framebuffer!");
	}
}

void VulkanFramebuffers::Create(IDevice* _device, ISwapChain* _swapChain, IRenderPass* _renderPass)
{
	CreateFramebuffers(_device->API_Vulkan(), _swapChain->API_Vulkan(), _renderPass->API_Vulkan());
}

void VulkanFramebuffers::Destroy(IDevice* _device)
{
	// Destroy framebuffers.
	for (VkFramebuffer framebuffer : framebuffers)
		vkDestroyFramebuffer(_device->API_Vulkan().GetLogicalDeviceVkHandle(), framebuffer, nullptr);
}
