#include "vulkan_device.h"

#include "vulkan_instance.h"
#include "vulkan_surface.h"

#include <set>

void VulkanDevice::PickPhysicalDevice(VkInstance _instance, VkSurfaceKHR _surface)
{
	// Enumerate physical devices, throw an exception if there is none available.
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
	if (deviceCount == 0)
		LOG_THROW("/!\\ Failed to find GPUs with Vulkan support!")
	else
		LOG_RHI("Picking a GPU to use...")

	// Get all physical devices.
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

	// Pick the first suitable device.
	for (const VkPhysicalDevice& device : devices)
	{
		if (IsDeviceSuitable(device, _surface))
		{
			physicalDevice = device;
			if (enableValidationLayers)
			{
				VkPhysicalDeviceProperties deviceProperties;
				vkGetPhysicalDeviceProperties(device, &deviceProperties);
				std::string name = deviceProperties.deviceName;
				LOG_RHI("Selected GPU --> [%s]", name.c_str())
			}
			break;
		}
	}

	// If no suitable device was found, throw an exception.
	if (physicalDevice == VK_NULL_HANDLE)
		LOG_THROW("/!\\ Failed to find a suitable GPU!")
}

bool VulkanDevice::IsDeviceSuitable(VkPhysicalDevice _device, VkSurfaceKHR _surface)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(_device, &deviceProperties);

	// Checks if the device is a discrete GPU (dedicated graphics card).
	bool bIsDiscreteGPU = (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);

	// Find queue families in this physical device.
	QueueFamilyIndices familyIndices = FindRequiredQueueFamilies(_device, _surface);

	// Checks if the device supports the extensions we need (specified in the header file of this class).
	bool bHasAllTheRequiredExtensions = CheckDeviceExtensionSupport(_device);

	// Checks if the device's swap chain support is suitable. --> I'm not quite sure what "suitable" means here though, to be seen. Formats? PresentModes?
	bool bIsSwapChainSupportAdequate = false;
	if (bHasAllTheRequiredExtensions)
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(_device, _surface);
		bIsSwapChainSupportAdequate = (!swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty());
	}
	// The device is suitable if it is a discrete GPU, has the required queue families (GRAPHICS and PRESENT), and has all the required extensions (we only need the Swap Chain extension, and we need it to fit our needs).
	bool finalRes = (bIsDiscreteGPU && familyIndices.bIsValid && bHasAllTheRequiredExtensions && bIsSwapChainSupportAdequate);
	return finalRes;
}

QueueFamilyIndices VulkanDevice::FindRequiredQueueFamilies(VkPhysicalDevice _device, VkSurfaceKHR _surface) const
{
	QueueFamilyIndices familyIndices;

	// Get the number of queue families.
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamilyCount, nullptr);

	// Get all queue families.
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamilyCount, queueFamilies.data());

	// Temporary variables to know if the families were found.
	VkBool32 bGraphicsFamilyFound = false;
	VkBool32 bPresentFamilyFound = false;

	// Find at least one queue family that supports graphics and presentation.
	for (int i = 0; i < queueFamilies.size(); i++)
	{
		// Check for GRAPHICS support.
		bGraphicsFamilyFound = (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT);
		if (bGraphicsFamilyFound)
			familyIndices.graphicsFamily = i;

		// Check for PRESENT support.
		vkGetPhysicalDeviceSurfaceSupportKHR(_device, i, _surface, &bPresentFamilyFound);
		if (bPresentFamilyFound)
			familyIndices.presentFamily = i;

		// If both families are found, set the indices struct to valid and break the loop.
		if (bGraphicsFamilyFound && bPresentFamilyFound)
		{
			familyIndices.bIsValid = true;
			break;
		}
	}
	return familyIndices;
}

bool VulkanDevice::CheckDeviceExtensionSupport(VkPhysicalDevice _device)
{
	// Enumerate extensions available in the given device.
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(_device, nullptr, &extensionCount, nullptr);

	// Retrieve those device extensions.
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(_device, nullptr, &extensionCount, availableExtensions.data());

	// Set the required extensions to the ones we specified in the header file (this is a temporary list).
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	// Delete from the temporary list the extensions that we found in the device.
	for (const VkExtensionProperties& extension : availableExtensions)
		requiredExtensions.erase(extension.extensionName);

	// Returns true if all the requires extensions have been found.
	return requiredExtensions.empty();
}

void VulkanDevice::CreateLogicalDevice(VkSurfaceKHR _surface)
{
	// Store the queue families' indices.
	indices = FindRequiredQueueFamilies(physicalDevice, _surface);

	// Specify queue creation info.
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {
		indices.graphicsFamily,
		indices.presentFamily
	};

	// Specify queue priorities (1.0 = highest).
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Create logical device info and features.
	VkPhysicalDeviceFeatures deviceFeatures{};
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	// Modify VkDeviceCreateInfo to point to the vector.
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	// Specify device features.
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	// Enable validation layers if is in debug mode.
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
		createInfo.enabledLayerCount = 0;

	// Create logical device and verify.
	VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice);
	if (result != VK_SUCCESS)
		LOG_THROW("/!\\ Failed to create device!")
	else
		LOG_RHI("Device created successfully.")
	volkLoadDevice(logicalDevice);

	// Get queue handles (index 0 since we only requested one queue).
	vkGetDeviceQueue(logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(logicalDevice, indices.presentFamily, 0, &presentQueue);
}

SwapChainSupportDetails VulkanDevice::QuerySwapChainSupport(VkPhysicalDevice _device, VkSurfaceKHR _surface)
{
	SwapChainSupportDetails details;

	// Get the capabilities of the given device with the current surface.
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device, _surface, &details.capabilities);

	// Get the supported surface formats of the given device.
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(_device, _surface, &formatCount, nullptr);
	// Store the formats.
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(_device, _surface, &formatCount, details.formats.data());
	}

	// Do the same thing as above, but for the supported surface present modes.
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(_device, _surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(_device, _surface, &presentModeCount, details.presentModes.data());
	}

	// Return the result of all the queries.
	return details;
}


void VulkanDevice::Create(IInstance* _instance, ISurface* _surface)
{
	LOG_CLEAN("\n\n===== DEVICE CREATION =====\n")

	VulkanInstance& vulkanInstance = _instance->API_Vulkan();
	VulkanSurface& vulkanSurface = _surface->API_Vulkan();

	// Copy the validation layers of the instance to here.
	validationLayers = vulkanInstance.GetValidationLayers();

	// Choose the adequate physical device.
	PickPhysicalDevice(vulkanInstance.GetVkHandle(), vulkanSurface.GetVkHandle());
	// Use it to create a logical device.
	CreateLogicalDevice(vulkanSurface.GetVkHandle());
}

void VulkanDevice::Destroy()
{
	LOG_CLEAN("\n\n===== DEVICE DESTRUCTION =====\n")

	// Destroy logical device. Physical device is destroyed automatically on instance destruction.
	if (logicalDevice != VK_NULL_HANDLE)
	{
		vkDestroyDevice(logicalDevice, nullptr);
		LOG_RHI("Device destroyed successfully.")
	}
	else
		LOG_RHI("Something went wrong trying to destroy the device...")

	validationLayers.clear();
	validationLayers.shrink_to_fit();
	deviceExtensions.clear();
	deviceExtensions.shrink_to_fit();
}

void VulkanDevice::WaitIdle()
{
	if (logicalDevice != VK_NULL_HANDLE)
		vkDeviceWaitIdle(logicalDevice);
	else
		LOG_THROW("Trying to wait for unexistent device : object is VK_NULL_HANDLE.")
}
