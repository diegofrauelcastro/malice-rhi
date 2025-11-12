#include "vulkan_commandpool.h"

#include "vulkan_device.h"

void VulkanCommandPool::CreateCommandPool(VulkanDevice& _device)
{
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = _device.GetQueueFamiliesIndices().graphicsFamily;

	// Create the command pool for the drawing commands.
	VkResult result = vkCreateCommandPool(_device.GetLogicalDeviceVkHandle(), &poolInfo, nullptr, &commandPool);
	if (result != VK_SUCCESS)
		LOG_THROW("/!\\ Failed to create command pool!")
}

void VulkanCommandPool::Create(IDevice* _device)
{
	CreateCommandPool(_device->API_Vulkan());
}

void VulkanCommandPool::Destroy(IDevice* _device)
{
	// Destroy command pool.
	if (commandPool != VK_NULL_HANDLE)
		vkDestroyCommandPool(_device->API_Vulkan().GetLogicalDeviceVkHandle(), commandPool, nullptr);
}
