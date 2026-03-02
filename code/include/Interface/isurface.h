#pragma once

#include <iostream>


// Forward declarations
class VulkanSurface;
class IInstance;
struct GLFWwindow;

// Surface interface
class ISurface
{
public:
	// Class destructor
	virtual ~ISurface() = default;


	/// Lifetime methods ///

	virtual void Create(IInstance* _instance, GLFWwindow* _window) = 0;
	virtual void Destroy(IInstance* _instance) = 0;


	/// Retrieving the backend ///

	virtual VulkanSurface& API_Vulkan() { throw std::runtime_error("Bad API call : object is not a VulkanSurface."); }
};