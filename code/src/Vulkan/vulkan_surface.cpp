#include "Vulkan/vulkan_surface.h"

#include "Vulkan/vulkan_instance.h"


void VulkanSurface::CreateSurface(IInstance* _instance, GLFWwindow* _window)
{
	LOG_RHI_CLEAN("\n\n===== SURFACE CREATION =====\n")

	VkResult result = glfwCreateWindowSurface(_instance->API_Vulkan().GetVkHandle(), _window, nullptr, &surface);
	if (result != VK_SUCCESS)
		LOG_RHI_THROW("/!\\ Failed to create surface!")
	else
		LOG_RHI("Created surface successfully.")
}

void VulkanSurface::Create(IInstance* _instance, GLFWwindow* _window)
{
	// Simply create the surface.
	CreateSurface(_instance, _window);
}

void VulkanSurface::Destroy(IInstance* _instance)
{
	LOG_RHI_CLEAN("\n\n===== SURFACE DESTRUCTION =====\n")

	// Destroy surface.
	if (surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(_instance->API_Vulkan().GetVkHandle(), surface, nullptr);
		LOG_RHI("Destroyed surface successfully.")
	}
	else
		LOG_RHI("Something went wrong trying to destroy a surface...")
}
