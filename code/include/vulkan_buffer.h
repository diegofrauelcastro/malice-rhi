#pragma once

#include "malicerhi_masterheader.h"
#include "ibuffer.h"


// Forward declarations
class VulkanDevice;
class VulkanCommandPool;

// General vulkan buffer class
class VulkanBuffer : public IBuffer
{
protected:
	/// Class properties ///

	// Buffer
	VkBuffer buffer;
	// Buffer memory
	VkDeviceMemory bufferMemory;

	/// Helper functions ///

	// Create a buffer.
	void CreateBuffer(VulkanDevice& _device, uint64_t _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkBuffer& _buffer, VkDeviceMemory& _bufferMemory);

	// Copy buffer data.
	void CopyBuffer(VulkanDevice& _device, VulkanCommandPool& _commandPool, VkBuffer _srcBuffer, VkBuffer _dstBuffer, uint64_t _size);

	// Create a vertex buffer.
	void CreateVertexBuffer(VulkanDevice& _device, VulkanCommandPool& _commandPool, uint64_t _size, const void* _src);

	// Create an index buffer.
	void CreateIndexBuffer(VulkanDevice& _device, VulkanCommandPool& _commandPool, uint64_t _size, const void* _src);

	// Find memory type for a buffer.
	uint32_t FindMemoryType(VulkanDevice& _device, uint32_t _typeFilter, VkMemoryPropertyFlags _properties);

public:
	// Class destructor
	virtual ~VulkanBuffer() override = default;


	/// Lifetime methods ///

	void Create(IDevice* _device, ICommandPool* _commandPool, EBufferUsage _usage, uint64_t _size, const void* _data) override;
	void Destroy(IDevice* _device) override;


	/// Retrieving the backend ///

	VulkanBuffer& API_Vulkan() override { return (*this); }
	VkBuffer GetVkHandle() const { return buffer; }
	VkDeviceMemory GetMemoryVkHandle() const { return bufferMemory; }

	/// Class specific methods ///

	// Returns the type of the buffer.
	uint32_t GetBufferType() const { return bufferType; }
};