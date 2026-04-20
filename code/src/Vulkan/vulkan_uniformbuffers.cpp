#include "Vulkan/vulkan_uniformbuffers.h"

#include "Vulkan/vulkan_device.h"
#include "Vulkan/vulkan_swapchain.h"
#include "Vulkan/vulkan_commandbuffers.h"

#include <cstring>

void VulkanUniformBuffers::CreateUniformBuffers(VulkanDevice& _device, uint32_t _framesInFlight)
{
	LOG_RHI_CLEAN("\n\n===== UNIFORM BUFFERS CREATION =====\n")

	// Resize the buffers vector to hold a uniform buffer for each frame.
	buffers.resize(_framesInFlight);
	// Create a uniform buffer for each frame.
	for (size_t i = 0; i < _framesInFlight; i++)
	{
		LOG_RHI("Creating uniform buffer %d.", (int)i)

		// Create buffer.
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = bufferSize;
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		// Create the buffer and ensure it succeeded.
		VkResult resultBufferCreate = vkCreateBuffer(_device.GetLogicalDeviceVkHandle(), &bufferInfo, nullptr, &buffers[i].buffer);
		if (resultBufferCreate != VK_SUCCESS)
			LOG_RHI_THROW("/!\\ Failed to create uniform buffer!")
		else
			LOG_RHI("Uniform buffer %d created successfully.", (int)i)
		
		// Get memory requirements for this buffer.
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(_device.GetLogicalDeviceVkHandle(), buffers[i].buffer, &memRequirements);

		// Allocate memory for this buffer.
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(_device, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		// Allocate the memory and ensure it succeeded.
		VkResult resultAlloc = vkAllocateMemory(_device.GetLogicalDeviceVkHandle(), &allocInfo, nullptr, &buffers[i].memory);
		if (resultAlloc != VK_SUCCESS)
			LOG_RHI_THROW("/!\\ Failed to allocate uniform buffer memory!")

		// Bind the buffer with the allocated memory.
		vkBindBufferMemory(_device.GetLogicalDeviceVkHandle(), buffers[i].buffer, buffers[i].memory, 0);
		// Map the buffer memory.
		vkMapMemory(_device.GetLogicalDeviceVkHandle(), buffers[i].memory, 0, bufferSize, 0, &buffers[i].mappedData);
		LOG_RHI("Uniform buffer %d memory allocated and bound successfully.", (int)i)
		LOG_RHI_CLEAN("")
	}
}

uint32_t VulkanUniformBuffers::FindMemoryType(VulkanDevice& _device, uint32_t _typeFilter, VkMemoryPropertyFlags _properties)
{
	// Get the memory properties of the physical device.
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(_device.GetPhysicalDeviceVkHandle(), &memProperties);

	// Find a memory type that fits the requirements.
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		// Check if the type is in the type filter and if it has the required properties.
		if (_typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & _properties) == _properties)
		{
			LOG_RHI("Found a suitable memory type for the uniform buffer.")
			return i;
		}
	}
	LOG_RHI_THROW("/!\\ Failed to find suitable memory type for the uniform buffer!")
}

void VulkanUniformBuffers::UploadData(ICommandBuffers* _commandBuffers, uint32_t _size, const void* _data)
{
	// Copy the data to the mapped memory of the uniform buffer for this frame.
	uint32_t currentFrameIndex = _commandBuffers->API_Vulkan().GetCurrentFrame();
	memcpy(buffers[currentFrameIndex].mappedData, _data, _size);
}

void VulkanUniformBuffers::UploadDataWithOffset(ICommandBuffers* _commandBuffers, uint32_t _offset, uint32_t _size, const void* _data)
{
	// Copy the data to the mapped memory of the uniform buffer for this frame.
	uint32_t currentFrameIndex = _commandBuffers->API_Vulkan().GetCurrentFrame();
	memcpy(static_cast<char*>(buffers[currentFrameIndex].mappedData) + _offset, _data, _size);
}

void VulkanUniformBuffers::Create(IDevice* _device, ISwapChain* _swapChain, uint32_t _bufferSize)
{
	// Create uniform buffers.
	bufferSize = static_cast<uint32_t>(_bufferSize);
	CreateUniformBuffers(_device->API_Vulkan(), _swapChain ? _swapChain->API_Vulkan().GetMaxFramesInFlight() : 1);
}

void VulkanUniformBuffers::Destroy(IDevice* _device)
{
	LOG_RHI_CLEAN("\n\n===== UNIFORM BUFFER DESTRUCTION =====\n")

	VulkanDevice& device = _device->API_Vulkan();

	// Destroy each uniform buffer.
	for (size_t i = 0; i < buffers.size(); i++)
	{
		// Unmap memory.
		if (buffers[i].mappedData)
		{
			vkUnmapMemory(device.GetLogicalDeviceVkHandle(), buffers[i].memory);
			LOG_RHI("Uniform buffer %d memory unmapped successfully.", (int)i)
		}
		else
			LOG_RHI("Uniform buffer %d memory was not mapped...", (int)i)
		// Destroy buffer.
		if (buffers[i].buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(device.GetLogicalDeviceVkHandle(), buffers[i].buffer, nullptr);
			LOG_RHI("Uniform buffer %d destroyed successfully.", (int)i)
		}
		else
			LOG_RHI("Something went wrong trying to destroy uniform buffer %d...", (int)i)
		// Free memory.
		if (buffers[i].memory != VK_NULL_HANDLE)
		{
			vkFreeMemory(device.GetLogicalDeviceVkHandle(), buffers[i].memory, nullptr);
			LOG_RHI("Uniform buffer %d memory freed successfully.", (int)i)
		}
		else
			LOG_RHI("Something went wrong trying to free uniform buffer %d memory...", (int)i)
		LOG_RHI_CLEAN("")
	}

	// Clear buffers vector.
	buffers.clear();
	buffers.shrink_to_fit();
}
