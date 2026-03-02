#include "Vulkan/vulkan_instance.h"


void VulkanInstance::CreateInstance()
{
	LOG_CLEAN("\n\n===== INSTANCE CREATION =====\n")

	LOG_RHI("Creating Vulkan instance... Warning: this may take several seconds and freeze the application.")
	// Check validation layers if is in debug mode.
	if (enableValidationLayers && !CheckValidationLayerSupport())
	{
		if (!CheckValidationLayerSupport())
			LOG_THROW("/!\\ Validation layers requested, but not available!")
		else
			LOG_RHI("Validation layers enabled successfully.")
	}

	// Application info.
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = instanceName.c_str();
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "RHI";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// Instance create info.
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	// Get required extensions.
	std::vector<const char*> extensions = GetRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	// Enable validation layers if is in debug mode.
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
		createInfo.enabledLayerCount = 0;

	// Create and verify instance.
	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
	if (result != VK_SUCCESS)
		LOG_THROW("/!\\ Failed to create Vulkan instance!")
	else
		LOG_RHI("Vulkan instance created successfully.")
	volkLoadInstance(instance);
}

std::vector<const char*> VulkanInstance::GetRequiredExtensions()
{
	// Get GLFW required extensions.
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	// Add debug utils extension if is in debug mode.
	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		extensions.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);

		// List all available extensions.
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensionsProperties(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionsProperties.data());
		LOG_RHI("Available extensions:")
		for (const VkExtensionProperties& extension : extensionsProperties)
		{
			std::string name = extension.extensionName;
			LOG_CLEAN("\t%s", name.c_str())
		}
		// List all required extensions.
		LOG_RHI("Required extensions:")
		for (const char* extension : extensions)
		{
			std::string name = extension;
			LOG_CLEAN("\t%s", name.c_str())
		}
	}
	LOG_CLEAN("")
	return extensions;
}

bool VulkanInstance::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool bLayerFound = false;

		for (const VkLayerProperties& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				bLayerFound = true;
				break;
			}
		}

		if (!bLayerFound)
			return false;
	}

	return true;
}

void VulkanInstance::Create(const char* _instanceName)
{
	// Rename the instance.
	instanceName = _instanceName;
	// Create the instance.
	CreateInstance();
	// Initialize the debug messenger (validation layers).
	SetupDebugMessenger();
	LOG_RHI("Vulkan instance and debug messenger initialized successfully.")
}

void VulkanInstance::Destroy()
{
	LOG_CLEAN("\n\n===== INSTANCE DESTRUCTION =====\n")

	// Destroy debug utils if necessary.
	if (enableValidationLayers && debugMessenger != VK_NULL_HANDLE)
	{
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		LOG_RHI("Debug messenger destroyed successfully.")
	}
	else
		LOG_RHI("Something went wrong trying to destroy the debug messenger...")

	// Destroy Vulkan instance.
	if (instance != VK_NULL_HANDLE)
	{
		vkDestroyInstance(instance, nullptr);
		LOG_RHI("Vulkan instance destroyed successfully.")
	}
	else
		LOG_RHI("Something went wrong trying to destroy the Vulkan instance...")

	validationLayers.clear();
	validationLayers.shrink_to_fit();
}

void VulkanInstance::SetupDebugMessenger()
{
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		LOG_THROW("/!\\ Failed to set up debug messenger!")
	else
		LOG_RHI("Debug messenger created successfully.")
}

void VulkanInstance::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = VulkanInstance::DebugCallback;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstance::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	LOG_CLEAN("")
	LOG_RHI("\n\n* /!\\ Validation layer --> %s\n", pCallbackData->pMessage)
	return VK_FALSE;
}

VkResult VulkanInstance::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void VulkanInstance::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
		func(instance, debugMessenger, pAllocator);
}


