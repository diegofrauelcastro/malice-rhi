#pragma once

#include "Interface/malicerhi_masterheader.h"
#include "Interface/idevice.h"

// Structure to hold queue family indices.
struct QueueFamilyIndices
{
	uint32_t graphicsFamily = 0;
	uint32_t presentFamily = 0;
	bool bIsValid = false;
};

// Structure to know more details about the type of swap chain support a physical device holds.
struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};


// Vulkan Device class
class VulkanDevice : public IDevice
{
private:
	/// Class properties ///

	// Handle to the physical device.
	VkPhysicalDevice physicalDevice;
	// Handle to the logical device.
	VkDevice logicalDevice;
	// Handle to the chosen graphics queue.
	VkQueue graphicsQueue;
	// Handle to the chosen present queue.
	VkQueue presentQueue;
	// Indices of the queue families;
	QueueFamilyIndices indices;

	// Enable checking layers on debug mode.
	#ifdef NDEBUG
		const bool enableValidationLayers = false;
	#else
		const bool enableValidationLayers = true;
	#endif

	// Device extensions that we want to check for, and enable.
	std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	// Validation layers that we want to enable. This vector is initialized empty, and only copies the one inside the instance passed as parameter during creation.
	std::vector<const char*> validationLayers;

	/// Helper methods ///

	void PickPhysicalDevice(VkInstance _instance, VkSurfaceKHR _surface);									// Pick physical device.
	bool IsDeviceSuitable(VkPhysicalDevice _device, VkSurfaceKHR _surface, bool _bShouldBeDiscrete);		// Check if the given physical device is suitable.
	bool CheckDeviceExtensionSupport(VkPhysicalDevice _device);												// Check if the given physical device has all the extensions that we require.
	void CreateLogicalDevice(VkSurfaceKHR _surface);														// Create logical device.
	QueueFamilyIndices FindRequiredQueueFamilies(VkPhysicalDevice _device, VkSurfaceKHR _surface) const;	// Find queue families in the given physical device.
	
public:
	// Class destructor
	virtual ~VulkanDevice() override = default;


	/// Lifetime methods ///

	void Create(IInstance* _instance, ISurface* _surface) override;
	void Destroy() override;

	/// Retrieving the backend ///

	VulkanDevice& API_Vulkan() { return (*this); }
	VkPhysicalDevice GetPhysicalDeviceVkHandle() const { return physicalDevice; }							// Get the handle to the physical device.
	VkDevice GetLogicalDeviceVkHandle() const { return logicalDevice; }										// Get the handle to the logical device.
	VkQueue GetGraphicsQueueVkHandle() const { return graphicsQueue; }										// Get the handle to the chosen graphics queue.
	VkQueue GetPresentQueueVkHandle() const { return presentQueue; }										// Get the handle to the chosen present queue.
	QueueFamilyIndices GetQueueFamiliesIndices() const { return indices; }									// Get indices of the queue families.

	/// Class specific methods ///

	// Wait until the device is idle.
	void WaitIdle() override;

	// Get the details of the swap chain support for the given physical device.
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice _device, VkSurfaceKHR _surface);
};