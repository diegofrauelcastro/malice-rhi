#pragma once

#include "malicerhi_masterheader.h"
#include "iuniformbuffers.h"

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
	void CreateUniformBuffers(VulkanDevice& _device, VulkanSwapChain& _swapChain);

	// Find a suitable memory type for the buffer.
	uint32_t FindMemoryType(VulkanDevice& _device, uint32_t _typeFilter, VkMemoryPropertyFlags _properties);

public:
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
};