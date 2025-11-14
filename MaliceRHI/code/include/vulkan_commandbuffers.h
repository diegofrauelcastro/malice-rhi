#pragma once

#include "malicerhi_masterheader.h"
#include "icommandbuffers.h"


// Forward declarations
class VulkanDevice;
class VulkanCommandPool;
class VulkanSwapChain;

// Swap chain interface
class VulkanCommandBuffers : public ICommandBuffers
{
private:
	/// Class properties ///
	std::vector<VkCommandBuffer> commandBuffers;

	/// Helper functions ///

	// Create command buffer.
	void CreateCommandBuffers(VulkanDevice& _device, VulkanCommandPool& _commandPool, VulkanSwapChain& _swapChain);

public:
	/// Lifetime methods ///

	void Create(IDevice* _device, ICommandPool* _commandPool, ISwapChain* _swapChain) override;
	void Destroy(IDevice* _device, ICommandPool* _commandPool) override;


	/// Retrieving the backend ///

	VulkanCommandBuffers& API_Vulkan() override { return (*this); }


	/// Class specific methods ///

	// Start the drawing cycle.
	void BeginDraw(IRenderPass* _renderPass, ISwapChain* _swapChain, IFramebuffers* _framebuffers, uint32_t& _imageIndex) override;

	// End the drawing cycle.
	void EndDraw() override;

	// Submit command buffer.
	void SubmitAndPresent(IDevice* _device, ISwapChain* _swapChain, uint32_t& _imageIndex) override;

	// Bind a pipeline to draw the next objects with.
	void BindPipeline(IPipeline* _pipeline) override;

	// Draw a specified number of vertices from a given set of vertex/index buffers.
	void DrawVerticesByIndices(uint32_t _vertexNumber, IBuffer* _vertexBuffer, IBuffer* _indexBuffer) override;
};