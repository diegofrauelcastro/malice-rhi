#pragma once

#include <iostream>


// Forward declarations
class VulkanRenderPass;
class IDevice;
class ISwapChain;
enum class ETextureFormat;

struct RenderPassDesc
{
	std::vector<ETextureFormat> colorFormats;
	ETextureFormat depthFormat;
	bool hasDepth = false;
	bool forPresent = false;
};

// Render pass interface
class IRenderPass
{
public:
	// Class destructor
	virtual ~IRenderPass() = default;


	/// Lifetime methods ///

	virtual void Create(IDevice* _device, ISwapChain* _swapChain) = 0;
	virtual void Destroy(IDevice* _device) = 0;


	/// Retrieving the backend ///

	virtual VulkanRenderPass& API_Vulkan() { throw std::runtime_error("Bad API call : object is not a VulkanRenderPass."); }
};