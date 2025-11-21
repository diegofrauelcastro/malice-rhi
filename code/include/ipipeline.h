#pragma once

#include <iostream>


// Forward declarations
class VulkanPipeline;
class IDevice;
class IBuffer;
class IRenderPass;
class IShaderModules;
struct GLFWwindow;

// Pipeline interface
class IPipeline
{
public:
	// Class destructor
	virtual ~IPipeline() = default;


	/// Lifetime methods ///

	virtual void Create(IDevice* _device, IRenderPass* _renderPass, IShaderModules* _shaders) = 0;
	virtual void Destroy(IDevice* _device) = 0;


	/// Retrieving the backend ///

	virtual VulkanPipeline& API_Vulkan() { throw std::runtime_error("Bad API call : object is not a VulkanPipeline."); }
};