#pragma once

#include "malicerhi_masterheader.h"
#include "iswapchain.h"

// Forward declaration
class VulkanDevice;

// Vulkan swap chain class
class VulkanSwapChain : public ISwapChain
{
private:
	/// Class properties ///

	// Swap chain.
	VkSwapchainKHR swapChain;
	// Handles to the images in our swap chain.
	std::vector<VkImage> swapChainImages;
	// Image views of our swap chain's images.
	std::vector<VkImageView> swapChainImageViews;
	// Image format of our swap chain.
	VkFormat swapChainImageFormat;
	// Extent (resolution) of our swap chain.
	VkExtent2D swapChainExtent;

	VkSurfaceKHR surface;
	GLFWwindow* window;

	// Sync objects (semaphores and fences, one per frame).

	// Semaphores (GPU block) to know if an image is available to render.
	std::vector<VkSemaphore> imageAvailableSemaphores;
	// Semaphores (GPU block) to know if the rendering is finished and presentation can happen.
	std::vector<VkSemaphore> renderFinishedSemaphores;
	// Fences (CPU block) to make sure only one image is being rendered at a time.
	std::vector<VkFence> inFlightFences;


	/// Helper functions ///

	// Choose the preferred surface format from the available ones.
	VkSurfaceFormatKHR ChoosePreferredSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& _availableFormats);

	// Choose the preferred present mode from the available ones.
	VkPresentModeKHR ChoosePreferredSwapPresentMode(const std::vector<VkPresentModeKHR>& _availablePresentModes);

	// Choose the preferred swap extent settings for our swap chain.
	VkExtent2D ChoosePreferredSwapExtent(const VkSurfaceCapabilitiesKHR& _capabilities, GLFWwindow* _window);

	// Setup the swap chain with the best settings we can find.
	void SetupSwapChain(VulkanDevice& _device, VkSurfaceKHR _surface, GLFWwindow* _window);

	// Create image views for each of the swap chain images.
	void CreateImageViews(VulkanDevice& _device);

	// Clamp method for uint32_t because I don't know how to move to C++17 to get std::clamp.
	uint32_t Clamp(uint32_t value, uint32_t min, uint32_t max);

	// Cleanup swap chain.
	void CleanupSwapChain(VulkanDevice& _device);

	// Cleanup sync objects.
	void CleanupSyncObjects(VulkanDevice& _device);

	// Create sync objects (semaphores and fences).
	void CreateSyncObjects(VulkanDevice& _device);

public:
	// Class destructor
	virtual ~VulkanSwapChain() override = default;


	/// Lifetime methods ///

	void Create(IDevice* _device, ISurface* _surface, GLFWwindow* _window) override;
	void Destroy(IDevice* _device) override;


	/// Retrieving the backend ///

	VulkanSwapChain& API_Vulkan() override { return (*this); }
	VkSwapchainKHR GetSwapChainVkHandle() const { return swapChain; }
	std::vector<VkImage> GetImagesVkHandle() const { return swapChainImages; }
	std::vector<VkImageView> GetImageViewsVkHandle() const { return swapChainImageViews; }
	VkFormat GetImageFormat() const { return swapChainImageFormat; }
	VkExtent2D GetImageExtent() const { return swapChainExtent; }
	std::vector<VkSemaphore> GetImageAvailableSemaphoresVkHandles() { return imageAvailableSemaphores; }
	std::vector<VkSemaphore> GetRenderFinishedSemaphoresVkHandles() { return renderFinishedSemaphores; }
	std::vector<VkFence> GetInFlightFencesVkHandles() { return inFlightFences; }


	/// Class specific methods ///

	// Gets the next presentable image in the swap chain. Returns i, the index of the retrieved image : i is in range [0, maxFramesInFlight-1] .
	uint32_t AcquireNextImage(IDevice* _device, uint32_t currentFrameIndex) override;

	// Recreate swap chain (if it becomes suboptimal or obsolete).
	void RecreateSwapChain(IDevice* _device);
};