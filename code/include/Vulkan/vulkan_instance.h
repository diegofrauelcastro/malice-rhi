#pragma once

#include "Interface/malicerhi_masterheader.h"
#include "Interface/iinstance.h"


// Vulkan Instance class
class VulkanInstance : public IInstance
{
private:
	/// Class properties ///

	// Handle to the vulkan instance.
	VkInstance instance;
	// Handle to the debug messenger.
	VkDebugUtilsMessengerEXT debugMessenger;

	// Enable checking layers on debug mode.
	#ifdef NDEBUG
		const bool enableValidationLayers = false;
	#else
		const bool enableValidationLayers = true;
	#endif

	// Validation layers that we want to enable.
	std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};


	/// Helper methods ///

	void CreateInstance();								// Create instance.
	std::vector<const char*> GetRequiredExtensions();	// Get required extensions.
	bool CheckValidationLayerSupport();					// Check layers.
	void SetupDebugMessenger();							// Setup debug messenger.

	// Populate debug messenger create info.
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	// Debug callback function.
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	// Create debug messenger.
	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	// Destroy debug messenger.
	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

public:
	// Class destructor
	virtual ~VulkanInstance() override = default;


	/// Lifetime methods ///

	void Create(const char* _instanceName) override;
	void Destroy() override;

	/// Retrieving the backend ///

	VulkanInstance& API_Vulkan() override { return (*this); }
	VkInstance GetVkHandle() const { return instance; }										// Get handle of the instance.
	std::vector<const char*> GetValidationLayers() const { return validationLayers; }		// Get all validation layers that we want to enable.
};