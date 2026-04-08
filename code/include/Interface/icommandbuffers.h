#pragma once

#include "color.h"
#include <iostream>


// Forward declarations
class VulkanCommandBuffers;
class IDevice;
class IRenderPass;
class IPipeline;
class ICommandPool;
class IBuffer;
class IFramebuffers;
class ISwapChain;
class IDescriptorSetsGroup;
class IUniformBuffers;
class ITexture;

// Swap chain interface
class ICommandBuffers
{
protected:
	/// Class properties ///

	// Index of the current image of the swap chain that is being presented.
	uint32_t currentFrame = 0;
	Color backgroundColor;

public:
	// Class destructor
	virtual ~ICommandBuffers() = default;


	/// Lifetime methods ///

	virtual void Create(IDevice* _device, ICommandPool* _commandPool, ISwapChain* _swapChain) = 0;
	virtual void Destroy(IDevice* _device, ICommandPool* _commandPool) = 0;


	/// Retrieving the backend ///

	virtual VulkanCommandBuffers& API_Vulkan() { throw std::runtime_error("Bad API call : object is not a ICommandBuffers."); }


	/// Command methods ///

	// Bind a pipeline to draw the next objects with. This method should be called within the scope of a BeginFrame(...)/EndFrame().
	virtual void BindPipeline(IPipeline* _pipeline) = 0;

	// Bind descriptor sets.
	virtual void BindDescriptorSets(IPipeline* _pipeline, IDescriptorSetsGroup* _descriptorSets) = 0;

	// Send push constants. This method should be called within the scope of a BeginFrame(...)/EndFrame().
	virtual void SendPushConstants(IPipeline* _pipeline, const void* _data, uint32_t _dataSize, uint32_t _dataOffset) = 0;

	// Update the uniform buffer descriptor in the given descriptor set, at the indicated set and binding indices. This method should be called within the scope of a BeginFrame(...)/EndFrame().
	virtual void UpdateUniformBuffer(IDevice* _device, IDescriptorSetsGroup* _descSets, IUniformBuffers* _ubo, uint32_t _setIndex, uint32_t _binding, uint32_t _descriptorCount) = 0;

	// Update the bound texture in the given descriptor set, at the indicated set and binding indices. This method should be called within the scope of a BeginFrame(...)/EndFrame().
	virtual void UpdateTexture(IDevice* _device, IDescriptorSetsGroup* _descSets, ITexture* _tex, uint32_t _setIndex, uint32_t _binding) = 0;

	// Begin rendering with a render pass. This works for both on-screen and offscreen rendering. For offscreen rendering, _framebufferIndex should be equal to 0. Otherwise, please use the _imageIndex given by the BeginFrame(...) method. This call MUST be followed by an EndRender() call once all the draw commands are done. Needless to say this method should be called inside the scope of a BeginFrame(...)/EndFrame().
	virtual void BeginRender(IRenderPass* _renderPass, IFramebuffers* _framebuffers, uint32_t _framebufferIndex = 0) = 0;

	// End rendering with a render pass (any that was active until now). This call comes AFTER a BeginRender(...) method call.
	virtual void EndRender() = 0;

	// Submit command buffer and present the image to the screen. This method should be called AFTER an EndFrame() method call.
	virtual void SubmitAndPresent(IDevice* _device, ISwapChain* _swapChain, IFramebuffers* _framebuffers, uint32_t& _imageIndex) = 0;

	// Draw a specified number of vertices from a given set of vertex/index buffers. This method should be called within the scope of a BeginRender(...)/EndRender().
	virtual void DrawVerticesByIndices(uint32_t _vertexNumber, IBuffer* vertexBuffer, IBuffer* indexBuffer) = 0;


	/// Class specific methods ///

	// Get current background color.
	Color GetClearColor() const { return backgroundColor; }

	// Get current background color.
	void SetClearColor(Color _color) { backgroundColor = _color; }

	// Get Current Frame.
	uint32_t GetCurrentFrame() const { return currentFrame; }

	// Begin recording commands for a frame's rendering process. After all rendering commands are done, the user MUST call EndFrame() before submitting the frame.
	virtual bool BeginFrame(IDevice* _device, ISwapChain* _swapChain, uint32_t& _imageIndex) = 0;

	// End recording commands for a frame's rendering process. Should be called AFTER a BeginFrame(...).
	virtual void EndFrame() = 0;
};