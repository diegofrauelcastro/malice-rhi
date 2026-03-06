#include "Vulkan/vulkan_swapchain.h"

#include "Vulkan/vulkan_surface.h"
#include "Vulkan/vulkan_device.h"

#include <limits>


VkSurfaceFormatKHR VulkanSwapChain::ChoosePreferredSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& _availableFormats)
{
	if (_availableFormats.size() == 0)
		LOG_RHI_THROW("/!\\ There are no available surface formats to choose from.")

	// We prefer the 32 bits BGR format, with sRGB color space.
	for (const VkSurfaceFormatKHR& availableFormat : _availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	// If the preferred format is not found, return the first available one, which should be just fine anyways.
	return _availableFormats[0];
}

VkPresentModeKHR VulkanSwapChain::ChoosePreferredSwapPresentMode(const std::vector<VkPresentModeKHR>& _availablePresentModes)
{
	// We prefer the Mailbox mode, which replaces older images with the newest ones (triple buffering) when the queue is full, resulting in low latency and no tearing.
	for (const VkPresentModeKHR& availablePresentMode : _availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return availablePresentMode;
	}

	// If Mailbox is not available, we fallback to FIFO mode, which is guaranteed to be available, and gives litle to no tearing either.
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapChain::ChoosePreferredSwapExtent(const VkSurfaceCapabilitiesKHR& _capabilities, GLFWwindow* _window)
{
	// If the current extent is not the special value (max uint32_t), we must use it as the swap extent. --> I'm not sure of when exactly the currentExtent is equal to the maximum value.
	if (_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return _capabilities.currentExtent;

	else
	{
		if (!_window)
			LOG_RHI_THROW("/!\\ Window is null when trying to get the preferred swap extent.")

		// Fetch the glfw's framebuffer width and height for the current window.
		int width, height;
		glfwGetFramebufferSize(_window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		// Clamp the resulting width and height to be within the allowed min and max extents of the physical device capbilities.
		actualExtent.width = Clamp(actualExtent.width, _capabilities.minImageExtent.width, _capabilities.maxImageExtent.width);
		actualExtent.height = Clamp(actualExtent.height, _capabilities.minImageExtent.height, _capabilities.maxImageExtent.height);

		return actualExtent;
	}
}
void VulkanSwapChain::SetupSwapChain(VulkanDevice& _device, VkSurfaceKHR _surface, GLFWwindow* _window)
{
	LOG_RHI_CLEAN("\n\n===== SWAP CHAIN SETUP =====\n")

	surface = _surface;
	window = _window;

	LOG_RHI("Configuring the swap chain...")
	SwapChainSupportDetails swapChainSupport = _device.QuerySwapChainSupport(_device.GetPhysicalDeviceVkHandle(), _surface);

	// Choose the best settings we can find in the physical device swap chain support.
	VkSurfaceFormatKHR surfaceFormat = ChoosePreferredSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = ChoosePreferredSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = ChoosePreferredSwapExtent(swapChainSupport.capabilities, _window);

	// Get the maximum number of images we are going to store in the swap chain queue. By default, we take the minimum number of images required to function, plus one.
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	// However, if that number is too high (for whatever reason), we set it directly to the maximum number of images that our swap chain supports.
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		imageCount = swapChainSupport.capabilities.maxImageCount;

	// Create info for our new swap chain with every setting we've chosen.
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = _surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.presentMode = presentMode;
	createInfo.oldSwapchain = VK_NULL_HANDLE;									// We suppose here we will never recreate the swap chain, so we won't have an old one to deal with.
	createInfo.clipped = VK_TRUE;												// More performance if some pixels are blocked by another window, or off-screen, etc.. (unless I got this wrong).
	createInfo.imageArrayLayers = 1;											// 1 by default, unless we build a 3D stereoscopic application. Which we won't.
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;				// This flag defines what we're going to do with the images in our swap chain (here, we are going to display them directly to the window).

	QueueFamilyIndices indices = _device.GetQueueFamiliesIndices();
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

	// Images can be used across multiple queue families without explicit ownership transfers.
	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	// An image is owned by one queue family at a time and ownership must be explicitly transferred before using it in another queue family.This option offers the best performance.
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;	// We do not want any transform applied to our images in the swap chain.
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;				// Opaque alpha unless we want to blend our window with other windows.

	// Create the swapchain and ensure it succeeded.
	VkResult result = vkCreateSwapchainKHR(_device.GetLogicalDeviceVkHandle(), &createInfo, nullptr, &swapChain);
	if (result != VK_SUCCESS)
		LOG_RHI_THROW("/!\\ Failed to create swap chain!")
	else
		LOG_RHI("Created swap chain successfully.")

	// Get the handles of the images in our swap chain.
	vkGetSwapchainImagesKHR(_device.GetLogicalDeviceVkHandle(), swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(_device.GetLogicalDeviceVkHandle(), swapChain, &imageCount, swapChainImages.data());
	// Store the format and the extent of our swap chain for later.
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void VulkanSwapChain::CreateImageViews(VulkanDevice& _device)
{
	swapChainImageViews.resize(swapChainImages.size());
	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		// Create the info for the image view at index i in our list of swap chain images.
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		// Set this image to be used directly to display.
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		// Define our image with 1 layer and no mipmaps.
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		// Create the image view and ensure it succeeded.
		VkResult result = vkCreateImageView(_device.GetLogicalDeviceVkHandle(), &createInfo, nullptr, &swapChainImageViews[i]);
		if (result != VK_SUCCESS)
			LOG_RHI_THROW("/!\\ Failed to create one of the image views. Image number %d", (int)i)
		else
			LOG_RHI("Successfully created image view %d", (int)i)
	}
}

uint32_t VulkanSwapChain::Clamp(uint32_t value, uint32_t min, uint32_t max)
{
	return std::max(min, std::min(value, max));
}

void VulkanSwapChain::CleanupSwapChain(VulkanDevice& _device)
{
	LOG_RHI_CLEAN("\n\n===== SWAP CHAIN CLEANUP =====\n")

	VkDevice device = _device.GetLogicalDeviceVkHandle();

	// Destroy all image views of our swap chain. The images themselves, however, will be deleted automatically on destroyal of the swap chain.
	for (VkImageView imageView : swapChainImageViews)
		vkDestroyImageView(device, imageView, nullptr);
	LOG_RHI("Successfully destroyed all image views.")

	// Destroy swap chain.
	if (swapChain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(device, swapChain, nullptr);
		LOG_RHI("Swap chain destroyed successfully.")
	}
	else
		LOG_RHI("Something went wrong trying to destroy a swap chain...")
}

void VulkanSwapChain::CleanupSyncObjects(VulkanDevice& _device)
{
	// Destroy sync objects.
	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		if (imageAvailableSemaphores[i] != VK_NULL_HANDLE)
			vkDestroySemaphore(_device.GetLogicalDeviceVkHandle(), imageAvailableSemaphores[i], nullptr);
		if (renderFinishedSemaphores[i] != VK_NULL_HANDLE)
			vkDestroySemaphore(_device.GetLogicalDeviceVkHandle(), renderFinishedSemaphores[i], nullptr);
		if (inFlightFences[i] != VK_NULL_HANDLE)
			vkDestroyFence(_device.GetLogicalDeviceVkHandle(), inFlightFences[i], nullptr);
	}
}

uint32_t VulkanSwapChain::AcquireNextImage(IDevice* _device, uint32_t currentFrameIndex)
{
	// Wait until the previous frame has finished.
	vkWaitForFences(_device->API_Vulkan().GetLogicalDeviceVkHandle(), 1, &inFlightFences[currentFrameIndex], VK_TRUE, UINT64_MAX);

	// Fetch the next image from the swap chain.
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(_device->API_Vulkan().GetLogicalDeviceVkHandle(), swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrameIndex], VK_NULL_HANDLE, &imageIndex);

	// Handle swap chain recreation if it's out of date (when window resized for example).
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		//RecreateSwapChain();
		LOG_RHI_THROW("/!\\ Swap chain is obsolete/suboptimal!")
	else if (result != VK_SUCCESS)
		LOG_RHI_THROW("/!\\ Failed to acquire swap chain image!")

	// Only reset the fence if we are submitting work.
	vkResetFences(_device->API_Vulkan().GetLogicalDeviceVkHandle(), 1, &inFlightFences[currentFrameIndex]);

	return imageIndex;
}

void VulkanSwapChain::RecreateSwapChain(IDevice* _device)
{
	// Handle window minimization.
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	// Wait until the device is idle before recreating the swap chain.
	_device->WaitIdle();

	// Cleanup the old swap chain and all its related resources.
	CleanupSwapChain(_device->API_Vulkan());
	SetupSwapChain(_device->API_Vulkan(), surface, window);
	CreateImageViews(_device->API_Vulkan());
}

void VulkanSwapChain::CreateSyncObjects(VulkanDevice& _device)
{
	size_t numberOfImages = swapChainImages.size();
	imageAvailableSemaphores.resize(numberOfImages);
	renderFinishedSemaphores.resize(numberOfImages);
	inFlightFences.resize(numberOfImages);

	// Infos for both semaphores.
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// Info for the fence.
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	// Create the semaphores and fences (one per frame).
	for (size_t i = 0; i < numberOfImages; i++)
	{
		// Create two semaphores and one fence for each frame, and ensure everything worked out well.
		VkResult semaphoreImageAvailableResult = vkCreateSemaphore(_device.GetLogicalDeviceVkHandle(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
		VkResult semaphoreRenderFinishedResult = vkCreateSemaphore(_device.GetLogicalDeviceVkHandle(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]);
		VkResult fenceInFlightResult = vkCreateFence(_device.GetLogicalDeviceVkHandle(), &fenceInfo, nullptr, &inFlightFences[i]);
		if (semaphoreImageAvailableResult != VK_SUCCESS || semaphoreRenderFinishedResult != VK_SUCCESS || fenceInFlightResult != VK_SUCCESS)
			LOG_RHI_THROW("/!\\ Failed to create semaphores/fences!")
	}
}


void VulkanSwapChain::Create(IDevice* _device, ISurface* _surface, GLFWwindow* _window)
{
	// Setup Swap Chain.
	SetupSwapChain(_device->API_Vulkan(), _surface->API_Vulkan().GetVkHandle(), _window);
	// Create image views from our swap chain.
	CreateImageViews(_device->API_Vulkan());
	// Create sync objects.
	CreateSyncObjects(_device->API_Vulkan());
}

void VulkanSwapChain::Destroy(IDevice* _device)
{
	CleanupSwapChain(_device->API_Vulkan());
	CleanupSyncObjects(_device->API_Vulkan());

	swapChainImages.clear();
	swapChainImages.shrink_to_fit();
	swapChainImageViews.clear();
	swapChainImageViews.shrink_to_fit();

	imageAvailableSemaphores.clear();
	imageAvailableSemaphores.shrink_to_fit();
	renderFinishedSemaphores.clear();
	renderFinishedSemaphores.shrink_to_fit();
	inFlightFences.clear();
	inFlightFences.shrink_to_fit();
}


