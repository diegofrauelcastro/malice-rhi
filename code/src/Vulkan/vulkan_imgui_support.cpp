#include "Vulkan/vulkan_imgui_support.h"
#include "Vulkan/vulkan_device.h"
#include "Vulkan/vulkan_swapchain.h"

void VulkanMaliceToImGuiBridge::Create(IInstance* _instance, IDevice* _device, ISwapChain* _screenSwapChain, IRenderPass* _screenRenderPass, IFramebuffers* _screenFramebuffers)
{
	LOG_RHI_CLEAN("\n\n===== IMGUI BRIDGE CREATION =====")

	storedDevice = _device;
	storedInstance = _instance;

	storedScreenSwapChain = _screenSwapChain;
	storedScreenFramebuffers = _screenFramebuffers;
	storedScreenRenderPass = _screenRenderPass;

	// Create the big ImGui descriptor pool.
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	VkResult result = vkCreateDescriptorPool(_device->API_Vulkan().GetLogicalDeviceVkHandle(), &pool_info, nullptr, &imguiDescPool);
	if (result != VK_SUCCESS)
		LOG_RHI("/!\\ Failed to create descriptor pool for ImGui support!")

	LOG_RHI_CLEAN("")
}

void VulkanMaliceToImGuiBridge::Destroy(IDevice* _device)
{
	LOG_RHI_CLEAN("\n\n===== IMGUI BRIDGE DESTRUCTION =====")

	vkDestroyDescriptorPool(_device->API_Vulkan().GetLogicalDeviceVkHandle(), imguiDescPool, nullptr);
	storedDevice = nullptr;
	storedScreenSwapChain = nullptr;
	storedScreenRenderPass = nullptr;
	storedInstance = nullptr;

	LOG_RHI_CLEAN("")
}
