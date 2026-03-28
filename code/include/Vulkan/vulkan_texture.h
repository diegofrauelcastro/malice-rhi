#pragma once

#include "Interface/malicerhi_masterheader.h"
#include "Interface/itexture.h"

//Forward declarations
class VulkanDevice;
class VulkanCommandPool;

class VulkanTexture : public ITexture
{
private:
	/// Class properties ///

	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageView imageView = VK_NULL_HANDLE;
	VkSampler sampler = VK_NULL_HANDLE;
	VkImageLayout currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;


	/// Helper methods ///

	void CreateImage(VulkanDevice& _device, uint32_t _width, uint32_t _height, VkFormat _vkFormat);
	void CreateImageView(VulkanDevice& _device, VkFormat _vkFormat);
	void CreateSampler(VulkanDevice& _device);
	void AllocateMemory(VulkanDevice& _device);
	void CreateStagingBuffer(VulkanDevice& _device, uint64_t _size, VkBuffer& _buffer, VkDeviceMemory& _bufferMemory);
	uint32_t FindMemoryType(VulkanDevice& _device, uint32_t _typeFilter, VkMemoryPropertyFlags _properties);
	void CopyBufferToImage(VulkanDevice& _device, VulkanCommandPool& _commandPool, VkBuffer _buffer, VkImage _image, uint32_t _width, uint32_t _height);
	VkCommandBuffer BeginSingleTimeCommands(VulkanDevice& _device, VulkanCommandPool& _commandPool);
	void EndSingleTimeCommands(VulkanDevice& _device, VulkanCommandPool& _commandPool, VkCommandBuffer _commandBuffer);

public:
	/// Lifetime methods ///

	void Create(IDevice* _device, ICommandPool* _commandPool, uint32_t _width, uint32_t _height, ETextureFormat _format, ETextureUsage _usage, const void* _data = nullptr) override;
	void Destroy(IDevice* _device) override;
	virtual ~VulkanTexture() override = default;


	/// Retrieve the backend ///

	VulkanTexture& API_Vulkan() override { return *this; }
	VkImage GetImage() const { return image; }
	VkImageView GetImageView() const { return imageView; }
	VkSampler GetSampler() const { return sampler; }

	
	/// Class specific methods ///

	void TransitionImageLayout(VulkanDevice& _device, VulkanCommandPool& _commandPool, VkImage _image, VkImageLayout _newLayout);
};