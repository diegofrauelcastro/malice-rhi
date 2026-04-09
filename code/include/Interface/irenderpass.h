#pragma once

#include <iostream>
#include "MaliceRHI/malice_enums.h"


// Forward declarations
class VulkanRenderPass;
class IDevice;
class ISwapChain;

struct RenderPassParams
{
	std::vector<ETextureFormat> colorFormats;
	ETextureFormat depthFormat;
	bool hasDepth = false;
};

// Render pass interface
class IRenderPass
{
protected:
	// Class properties

	RenderPassParams params;

public:
	// Class destructor
	virtual ~IRenderPass() = default;


	/// Lifetime methods ///

	// Create a RenderPass with a SwapChain to present on screen. To create an offscreen RenderPass, use Create() with RenderPassParams instead of a SwapChain.
	virtual void Create(IDevice* _device, ISwapChain* _swapChain, bool _hasDepth) = 0;
	// Create a RenderPass manually to be able to render offscreen.
	virtual void Create(IDevice* _device, const RenderPassParams& _params) = 0;
	virtual void Destroy(IDevice* _device) = 0;


	/// Retrieving the backend ///

	virtual VulkanRenderPass& API_Vulkan() { throw std::runtime_error("Bad API call : object is not a VulkanRenderPass."); }
	bool GetHasDepth() const { return params.hasDepth; }
};