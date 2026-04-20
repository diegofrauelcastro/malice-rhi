#pragma once

#include "Interface/malicerhi_masterheader.h"
#include "Interface/iuniformbuffers.h"

// Forward declarations
class VulkanDevice;
class VulkanSwapChain;
class VulkanCommandBuffers;

// Uniform buffer class
class VulkanUniformBuffers : public IUniformBuffers
{
private:
	/// Per-frame UBO structure ///
	struct PerFrameUBO
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		void* mappedData = nullptr;
	};


	/// Class properties ///

	std::vector<PerFrameUBO> buffers;
	uint32_t bufferSize;

	/// Helper functions ///

	// Create uniform buffers.
	void CreateUniformBuffers(VulkanDevice& _device, uint32_t _framesInFlight);

	// Find a suitable memory type for the buffer.
	uint32_t FindMemoryType(VulkanDevice& _device, uint32_t _typeFilter, VkMemoryPropertyFlags _properties);

public:
	// Class destructor
	virtual ~VulkanUniformBuffers() override = default;


	/// Lifetime methods ///
	void Create(IDevice* _device, ISwapChain* _swapChain, uint32_t _bufferSize) override;
	void Destroy(IDevice* _device) override;


	/// Retrieving the backend ///

	VulkanUniformBuffers& API_Vulkan() override { return *this; }
	std::vector<PerFrameUBO> GetBuffers() const { return buffers; }
	uint32_t GetBufferSize() const { return bufferSize; }


	/// Class specific methods ///

	// Upload data to the uniform buffer, using the current frame stored in the command buffers.
	void UploadData(ICommandBuffers* _commandBuffers, uint32_t _size, const void* _data) override;

	// Upload data to the uniform buffer, using the current frame stored in the command buffers. Use carefully, as this can cause memory issues if not handled correctly due to offset.
	virtual void UploadDataWithOffset(ICommandBuffers* _commandBuffers, uint32_t _offset, uint32_t _size, const void* _data) override;
};