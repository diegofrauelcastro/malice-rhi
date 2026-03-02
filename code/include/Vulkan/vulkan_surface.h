#pragma once

#include "Interface/malicerhi_masterheader.h"
#include "Interface/isurface.h"

// Vulkan Surface class
class VulkanSurface : public ISurface
{
private:
	/// Class properties ///

	// Handle of the surface.
	VkSurfaceKHR surface;

	// Enable checking layers on debug mode.
	#ifdef NDEBUG
		const bool enableValidationLayers = false;
	#else
		const bool enableValidationLayers = true;
	#endif


	/// Helper functions ///

	void CreateSurface(IInstance* _instance, GLFWwindow* _window);

public:
	// Class destructor
	virtual ~VulkanSurface() override = default;


	/// Lifetime methods ///

	void Create(IInstance* _instance, GLFWwindow* _window) override;
	void Destroy(IInstance* _instance) override ;


	/// Retrieving the backend ///

	VulkanSurface& API_Vulkan() override { return (*this); }
	VkSurfaceKHR GetVkHandle() const { return surface; }
};