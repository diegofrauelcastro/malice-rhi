#pragma once

#include <iostream>


// Forward declarations
class VulkanFramebuffers;
class IDevice;
class ITexture;
class ISwapChain;
class IRenderPass;

struct FramebufferParams
{
	std::vector<ITexture*> colorAttachments;
	ITexture* depthAttachment = nullptr;
	uint32_t width;
	uint32_t height;
};

// Framebuffers interface
class IFramebuffers
{
protected:
	// Class properties

	FramebufferParams params;

public:
	// Class destructor
	virtual ~IFramebuffers() = default;


	/// Lifetime methods ///

	// Direct screen rendering. For offscreen rendering, use Create() with FramebufferParams instead of an ISwapChain.
	virtual void Create(IDevice* _device, IRenderPass* _renderPass, ISwapChain* _swapChain, ITexture* _depthTex) = 0;
	// Flexible creation of framebuffers, can be used for offscreen rendering.
	virtual void Create(IDevice* _device, IRenderPass* _renderPass, const FramebufferParams& _params) = 0;
	virtual void Destroy(IDevice* _device) = 0;


	/// Retrieving the backend ///

	virtual VulkanFramebuffers& API_Vulkan() { throw std::runtime_error("Bad API call : object is not a VulkanFramebuffers."); }
	uint32_t GetWidth() const { return params.width; }
	uint32_t GetHeight() const { return params.height; }
};