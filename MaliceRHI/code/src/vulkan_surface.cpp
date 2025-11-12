#include "vulkan_surface.h"

#include "vulkan_instance.h"


void VulkanSurface::CreateSurface(IInstance* _instance, GLFWwindow* _window)
{
	VkResult result = glfwCreateWindowSurface(_instance->API_Vulkan().GetVkHandle(), _window, nullptr, &surface);
	if (result != VK_SUCCESS)
		throw std::runtime_error("/!\\ Failed to create window surface!");

	if (enableValidationLayers)
		std::cout << "\n* Created window surface successfully." << std::endl;
}

void VulkanSurface::Create(IInstance* _instance, GLFWwindow* _window)
{
	// Simply create the surface.
	CreateSurface(_instance, _window);
}

void VulkanSurface::Destroy(IInstance* _instance)
{
	// Destroy surface.
	if (surface != VK_NULL_HANDLE)
		vkDestroySurfaceKHR(_instance->API_Vulkan().GetVkHandle(), surface, nullptr);
}
