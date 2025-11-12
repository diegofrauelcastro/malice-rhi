#include "vulkan_buffer.h"

#include "vulkan_device.h"
#include "vulkan_commandpool.h"


void VulkanBuffer::CreateBuffer(VulkanDevice& _device, uint64_t _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkBuffer& _buffer, VkDeviceMemory& _bufferMemory)
{
	// Create info about the buffer we want to create.
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = _size;
	bufferInfo.usage = _usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// Create the buffer and ensure it succeeded.
	VkResult result = vkCreateBuffer(_device.GetLogicalDeviceVkHandle(), &bufferInfo, nullptr, &_buffer);
	if (result != VK_SUCCESS)
		LOG_THROW("/!\\ Failed to create buffer!")

	// Get the memory requirements for the buffer.
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(_device.GetLogicalDeviceVkHandle(), _buffer, &memRequirements);

	// Create info about the memory allocation for the buffer.
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(_device, memRequirements.memoryTypeBits, _properties);

	// Allocate the memory and ensure it succeeded.
	result = vkAllocateMemory(_device.GetLogicalDeviceVkHandle(), &allocInfo, nullptr, &_bufferMemory);
	if (result != VK_SUCCESS)
		LOG_THROW("/!\\ Failed to allocate buffer memory!")

	// Bind the buffer with the allocated memory.
	vkBindBufferMemory(_device.GetLogicalDeviceVkHandle(), _buffer, _bufferMemory, 0);
}

void VulkanBuffer::CreateVertexBuffer(VulkanDevice& _device, VulkanCommandPool& _commandPool, uint64_t _size, void* _src)
{
	// Create a staging buffer that is host visible to upload the vertex data to it.
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(_device, _size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	// Map the memory and copy the vertex data to it.
	void* data;
	vkMapMemory(_device.GetLogicalDeviceVkHandle(), stagingBufferMemory, 0, _size, 0, &data);
	memcpy(data, _src, (size_t)_size);
	vkUnmapMemory(_device.GetLogicalDeviceVkHandle(), stagingBufferMemory);

	// Create the actual vertex buffer with device local memory.
	CreateBuffer(_device, _size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);
	// Copy the data from the staging buffer to the vertex buffer.
	CopyBuffer(_device, _commandPool, stagingBuffer, buffer, _size);

	// Destroy the staging buffer and free its memory.
	vkDestroyBuffer(_device.GetLogicalDeviceVkHandle(), stagingBuffer, nullptr);
	vkFreeMemory(_device.GetLogicalDeviceVkHandle(), stagingBufferMemory, nullptr);
}

void VulkanBuffer::CreateIndexBuffer(VulkanDevice& _device, VulkanCommandPool& _commandPool, uint64_t _size, void* _src)
{
	// Create a staging buffer that is host visible to upload the index data to it.
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(_device, _size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	// Map the memory and copy the index data to it.
	void* data;
	vkMapMemory(_device.GetLogicalDeviceVkHandle(), stagingBufferMemory, 0, _size, 0, &data);
	memcpy(data, _src, (size_t)_size);
	vkUnmapMemory(_device.GetLogicalDeviceVkHandle(), stagingBufferMemory);

	// Create the actual index buffer with device local memory.
	CreateBuffer(_device, _size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);
	// Copy the data from the staging buffer to the index buffer.
	CopyBuffer(_device, _commandPool, stagingBuffer, buffer, _size);

	// Destroy the staging buffer and free its memory.
	vkDestroyBuffer(_device.GetLogicalDeviceVkHandle(), stagingBuffer, nullptr);
	vkFreeMemory(_device.GetLogicalDeviceVkHandle(), stagingBufferMemory, nullptr);
}

void VulkanBuffer::CopyBuffer(VulkanDevice& _device, VulkanCommandPool& _commandPool, VkBuffer _srcBuffer, VkBuffer _dstBuffer, uint64_t _size)
{
	// Allocate a temporary command buffer for the copy operation.
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = _commandPool.GetVkHandle();
	allocInfo.commandBufferCount = 1;

	// Create the command buffer.
	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(_device.GetLogicalDeviceVkHandle(), &allocInfo, &commandBuffer);

	// Begin recording the command buffer.
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	// Define the region to copy.
	VkBufferCopy copyRegion{};
	copyRegion.size = _size;
	vkCmdCopyBuffer(commandBuffer, _srcBuffer, _dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	// Submit the command buffer and wait for it to finish.
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(_device.GetGraphicsQueueVkHandle(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(_device.GetGraphicsQueueVkHandle());

	// Free the temporary command buffer.
	vkFreeCommandBuffers(_device.GetLogicalDeviceVkHandle(), _commandPool.GetVkHandle(), 1, &commandBuffer);
}

uint32_t VulkanBuffer::FindMemoryType(VulkanDevice& _device, uint32_t _typeFilter, VkMemoryPropertyFlags _properties)
{
	// Get the memory properties of the physical device.
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(_device.GetPhysicalDeviceVkHandle(), &memProperties);

	// Find a memory type that fits the requirements.
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		// Check if the type is in the type filter and if it has the required properties.
		if (_typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & _properties) == _properties)
			return i;
	}
	LOG_THROW("/!\\ Failed to find suitable memory type!")
}

void VulkanBuffer::Create(IDevice* _device, ICommandPool* _commandPool, EBufferUsage _usage, uint64_t _size, void* _data)
{
	// Init the buffer with the given data for the specified usage.
	bufferType = _usage;
	size = _size;
	switch (bufferType)
	{
	case VERTEX_BUFFER:
		CreateVertexBuffer(_device->API_Vulkan(), _commandPool->API_Vulkan(), _size, _data);
		break;
	case INDEX_BUFFER:
		CreateIndexBuffer(_device->API_Vulkan(), _commandPool->API_Vulkan(), _size, _data);
		break;
	default:
		LOG_THROW("/!\\ Creation of an invalid type of buffer!")
	}
}

void VulkanBuffer::Destroy(IDevice* _device)
{
	// Free the buffer

	if (buffer != VK_NULL_HANDLE)
		vkDestroyBuffer(_device->API_Vulkan().GetLogicalDeviceVkHandle(), buffer, nullptr);
	if (bufferMemory != VK_NULL_HANDLE)
		vkFreeMemory(_device->API_Vulkan().GetLogicalDeviceVkHandle(), bufferMemory, nullptr);
}
