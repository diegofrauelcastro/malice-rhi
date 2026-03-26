#include "Vulkan/vulkan_texture.h"
#include "Vulkan/vulkan_device.h"
#include "Vulkan/vulkan_commandpool.h"

#include <stdexcept>
#include <cstring>

static VkFormat ConvertFormat(ETextureFormat format)
{
	switch (format)
	{
		case ETextureFormat::RGBA8:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case ETextureFormat::BGRA8:
			return VK_FORMAT_B8G8R8A8_UNORM;
		case ETextureFormat::DEPTH32:
			return VK_FORMAT_D32_SFLOAT;
		default:
			LOG_RHI_THROW("/!\\ Unsupported texture format!")
	}
}

void VulkanTexture::Create(IDevice* _device, ICommandPool* _commandPool, uint32_t _width, uint32_t _height, ETextureFormat _format, ETextureUsage _usage, const void* data)
{
	LOG_RHI_CLEAN("\n\n===== TEXTURE CREATION =====\n")

	VulkanCommandPool vkCmdPool = _commandPool->API_Vulkan();
	VulkanDevice& vulkanDevice = _device->API_Vulkan();
	VkDevice device = vulkanDevice.GetLogicalDeviceVkHandle();

	width = _width;
	height = _height;
	format = _format;
	usage = _usage;

	// Calculate size of the image according to its type. For now, only RGBA8 and BGRA8 are supported.
	uint32_t bytesPerPixel = (format == ETextureFormat::RGBA8 || format == ETextureFormat::BGRA8) ? 4 : 0;
	VkDeviceSize imageSize = width * height * bytesPerPixel;
	VkFormat vkFormat = ConvertFormat(format);

	CreateImage(vulkanDevice, width, height, vkFormat);
	AllocateMemory(vulkanDevice);
	CreateImageView(vulkanDevice, vkFormat);
	if (HasUsage(usage, ETextureUsage::SAMPLED))
		CreateSampler(vulkanDevice);
	currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	// Passing the data to the GPU via a staging buffer.
	if (data)
	{
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateStagingBuffer(vulkanDevice, imageSize, stagingBuffer, stagingBufferMemory);

		void* stagingData;
		vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &stagingData);
		memcpy(stagingData, data, static_cast<size_t>(imageSize));
		vkUnmapMemory(device, stagingBufferMemory);

		TransitionImageLayout(vulkanDevice, vkCmdPool, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		CopyBufferToImage(vulkanDevice, vkCmdPool, stagingBuffer, image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
		TransitionImageLayout(vulkanDevice, vkCmdPool, image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}
	else
	{
		if (HasUsage(usage, ETextureUsage::COLOR_ATTACHMENT))
			TransitionImageLayout(vulkanDevice, vkCmdPool, image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		if (HasUsage(usage, ETextureUsage::DEPTH_ATTACHMENT))
			TransitionImageLayout(vulkanDevice, vkCmdPool, image, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}

	LOG_RHI_CLEAN("")
}

void VulkanTexture::Destroy(IDevice* _device)
{
	LOG_RHI_CLEAN("\n\n===== TEXTURE DESTRUCTION =====\n")

	VulkanDevice& vulkanDevice = _device->API_Vulkan();
	VkDevice device = vulkanDevice.GetLogicalDeviceVkHandle();

	if (sampler) vkDestroySampler(device, sampler, nullptr);
	if (imageView) vkDestroyImageView(device, imageView, nullptr);
	if (image) vkDestroyImage(device, image, nullptr);
	if (memory) vkFreeMemory(device, memory, nullptr);

	LOG_RHI_CLEAN("")
}

void VulkanTexture::CreateImage(VulkanDevice& _device, uint32_t _width, uint32_t _height, VkFormat _vkFormat)
{
	VkImageCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.imageType = VK_IMAGE_TYPE_2D;
	info.extent.width = _width;
	info.extent.height = _height;
	info.extent.depth = 1;
	info.mipLevels = 1;
	info.arrayLayers = 1;
	info.format = _vkFormat;
	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	info.samples = VK_SAMPLE_COUNT_1_BIT;

	// Make the correct usage for the texture.
	info.usage = 0;
	if (HasUsage(usage, ETextureUsage::SAMPLED))
		info.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;

	if (HasUsage(usage, ETextureUsage::COLOR_ATTACHMENT))
		info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (HasUsage(usage, ETextureUsage::DEPTH_ATTACHMENT))
		info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	// VERY IMPORTANT if reused later
	if (HasUsage(usage, ETextureUsage::SAMPLED))
		info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	VkResult result = vkCreateImage(_device.GetLogicalDeviceVkHandle(), &info, nullptr, &image);
	if (result != VK_SUCCESS)
		LOG_RHI_THROW("/!\\ Failed to create texture image!")
	else
		LOG_RHI("Texture image created succesfully.")
}

void VulkanTexture::AllocateMemory(VulkanDevice& _device)
{
	VkMemoryRequirements memReq;
	vkGetImageMemoryRequirements(_device.GetLogicalDeviceVkHandle(), image, &memReq);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = FindMemoryType(_device, memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkResult result = vkAllocateMemory(_device.GetLogicalDeviceVkHandle(), &allocInfo, nullptr, &memory);
	if (result != VK_SUCCESS)
		LOG_RHI_THROW("/!\\ Failed to allocate texture image memory!")
	result = vkBindImageMemory(_device.GetLogicalDeviceVkHandle(), image, memory, 0);
	if (result != VK_SUCCESS)
		LOG_RHI_THROW("/!\\ Failed to bind texture image memory!")
}

void VulkanTexture::CreateImageView(VulkanDevice& _device, VkFormat _vkFormat)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = _vkFormat;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.layerCount = 1;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	if (HasUsage(usage, ETextureUsage::DEPTH_ATTACHMENT))
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	VkResult result = vkCreateImageView(_device.GetLogicalDeviceVkHandle(), &viewInfo, nullptr, &imageView);
	if (result != VK_SUCCESS)
		LOG_RHI_THROW("/!\\ Failed to create texture image view!")
	else
		LOG_RHI("Created texture image view successfully.")
}

void VulkanTexture::CreateSampler(VulkanDevice& _device)
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	VkResult result = vkCreateSampler(_device.GetLogicalDeviceVkHandle(), &samplerInfo, nullptr, &sampler);
	if (result != VK_SUCCESS)
		LOG_RHI_THROW("/!\\ Failed to create texture sampler!")
	else
		LOG_RHI("Created texture sampler successfully.")
}

void VulkanTexture::CreateStagingBuffer(VulkanDevice& _device, uint64_t _size, VkBuffer& _buffer, VkDeviceMemory& _bufferMemory)
{
	LOG_RHI("Creating empty staging buffer for texture loading...")

	// Create info about the buffer we want to create.
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = _size;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// Create the buffer and ensure it succeeded.
	VkResult result = vkCreateBuffer(_device.GetLogicalDeviceVkHandle(), &bufferInfo, nullptr, &_buffer);
	if (result != VK_SUCCESS)
		LOG_RHI_THROW("/!\\ Failed to create texture staging buffer!")
	else
		LOG_RHI("Texture staging buffer created successfully.")

	// Get the memory requirements for the buffer.
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(_device.GetLogicalDeviceVkHandle(), _buffer, &memRequirements);

	// Create info about the memory allocation for the buffer.
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(_device, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Allocate the memory and ensure it succeeded.
	result = vkAllocateMemory(_device.GetLogicalDeviceVkHandle(), &allocInfo, nullptr, &_bufferMemory);
	if (result != VK_SUCCESS)
		LOG_RHI_THROW("/!\\ Failed to allocate texture staging buffer memory!")
	else
		LOG_RHI("Texture staging buffer memory allocated successfully.\n")

	// Bind the buffer with the allocated memory.
	vkBindBufferMemory(_device.GetLogicalDeviceVkHandle(), _buffer, _bufferMemory, 0);
}

uint32_t VulkanTexture::FindMemoryType(VulkanDevice& _device, uint32_t _typeFilter, VkMemoryPropertyFlags _properties)
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
			LOG_RHI("Found a suitable memory type for texture (texture or staging buffer).")
			return i;
		}
	}
	LOG_RHI_THROW("/!\\ Failed to find suitable memory type for texture!")
}

void VulkanTexture::TransitionImageLayout(VulkanDevice& _device, VulkanCommandPool& _commandPool, VkImage _image, VkImageLayout _newLayout)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(_device, _commandPool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = currentLayout;
	barrier.newLayout = _newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = _image;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	if (HasUsage(usage, ETextureUsage::DEPTH_ATTACHMENT))
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;

	// From undefined to optimal-transfer.
	if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && _newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
	// From optimal-transfer to shader-readable.
    else if (currentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && _newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
	// From undefined to depth.
	else if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && _newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	// From depth to shader-readable.
	else if (currentLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && _newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	// From undefined to color attachment.
    else if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && _newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
	// From color attachment to shader-readable.
    else if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && _newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
	else
		LOG_RHI_THROW("/!\\ Unsupported layout transition!")

	vkCmdPipelineBarrier(
		commandBuffer,
		srcStage, dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	EndSingleTimeCommands(_device, _commandPool, commandBuffer);
	currentLayout = _newLayout;
}

void VulkanTexture::CopyBufferToImage(VulkanDevice& _device, VulkanCommandPool& _commandPool, VkBuffer _buffer, VkImage _image, uint32_t _width, uint32_t _height)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(_device, _commandPool);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { _width, _height, 1 };
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	if (HasUsage(usage, ETextureUsage::DEPTH_ATTACHMENT))
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	vkCmdCopyBufferToImage(commandBuffer, _buffer, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	EndSingleTimeCommands(_device, _commandPool, commandBuffer);
}

VkCommandBuffer VulkanTexture::BeginSingleTimeCommands(VulkanDevice& _device, VulkanCommandPool& _commandPool)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = _commandPool.GetVkHandle();
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(_device.GetLogicalDeviceVkHandle(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void VulkanTexture::EndSingleTimeCommands(VulkanDevice& _device, VulkanCommandPool& _commandPool, VkCommandBuffer _commandBuffer)
{
	vkEndCommandBuffer(_commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_commandBuffer;

	vkQueueSubmit(_device.GetGraphicsQueueVkHandle(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(_device.GetGraphicsQueueVkHandle());

	vkFreeCommandBuffers(_device.GetLogicalDeviceVkHandle(), _commandPool.GetVkHandle(), 1, &_commandBuffer);
}
