#include "Vulkan/vulkan_commandpool.h"

#include "Vulkan/vulkan_device.h"

void VulkanCommandPool::CreateCommandPool(VulkanDevice& _device)
{
	LOG_CLEAN("\n\n===== COMMAND POOL CREATION =====\n")

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = _device.GetQueueFamiliesIndices().graphicsFamily;

	// Create the command pool for the drawing commands.
	VkResult result = vkCreateCommandPool(_device.GetLogicalDeviceVkHandle(), &poolInfo, nullptr, &commandPool);
	if (result != VK_SUCCESS)
		LOG_THROW("/!\\ Failed to create command pool!")
	else
		LOG_RHI("Command pool created successfully.")
}

void VulkanCommandPool::Create(IDevice* _device)
{
	CreateCommandPool(_device->API_Vulkan());
}

void VulkanCommandPool::Destroy(IDevice* _device)
{
	LOG_CLEAN("\n\n===== COMMAND POOL DESTRUCTION =====\n")

	// Destroy command pool.
	if (commandPool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool(_device->API_Vulkan().GetLogicalDeviceVkHandle(), commandPool, nullptr);
		LOG_RHI("Command pool destroyed successfully.")
	}
	else
		LOG_RHI("Something went wrong trying to destroy a command pool...")
}
