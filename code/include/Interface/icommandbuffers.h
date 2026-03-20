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


	/// Class specific methods ///

	// Get current background color.
	Color GetClearColor() const { return backgroundColor; }

	// Get current background color.
	void SetClearColor(Color _color) { backgroundColor = _color; }

	// Get Current Frame.
	uint32_t GetCurrentFrame() const { return currentFrame; }

	// Start the drawing cycle.
	virtual void BeginDraw(IRenderPass* _renderPass, ISwapChain* _swapChain, IFramebuffers* _framebuffers, uint32_t& _imageIndex) = 0;

	// End the drawing cycle.
	virtual void EndDraw() = 0;

	// Submit command buffer.
	virtual void SubmitAndPresent(IDevice* _device, ISwapChain* _swapChain, IFramebuffers* _framebuffers, uint32_t& _imageIndex) = 0;

	// Bind a pipeline to draw the next objects with.
	virtual void BindPipeline(IPipeline* _pipeline) = 0;

	// Bind descriptor sets.
	virtual void BindDescriptorSets(IPipeline* _pipeline, IDescriptorSetsGroup* _descriptorSets) = 0;

	// Draw a specified number of vertices from a given set of vertex/index buffers.
	virtual void DrawVerticesByIndices(uint32_t _vertexNumber, IBuffer* vertexBuffer, IBuffer* indexBuffer) = 0;

	// Update the uniform buffer descriptor in the given descriptor set, at the indicated set and binding indices.
	virtual void UpdateUniformBuffer(IDevice* _device, IDescriptorSetsGroup* _descSets, IUniformBuffers* _ubo, uint32_t _setIndex, uint32_t _binding, uint32_t _descriptorCount) = 0;

	// Update the bound texture in the given descriptor set, at the indicated set and binding indices.
	virtual void UpdateTexture(IDevice* _device, IDescriptorSetsGroup* _descSets, ITexture* _tex, uint32_t _setIndex, uint32_t _binding) = 0;
};