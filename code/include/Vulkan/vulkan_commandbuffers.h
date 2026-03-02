#pragma once

#include "Interface/malicerhi_masterheader.h"
#include "Interface/icommandbuffers.h"


// Forward declarations
class VulkanDevice;
class VulkanCommandPool;
class VulkanSwapChain;

// Vulkan command buffers class
class VulkanCommandBuffers : public ICommandBuffers
{
private:
	/// Class properties ///
	std::vector<VkCommandBuffer> commandBuffers;
	VkExtent2D swapChainExtent;

	/// Helper functions ///

	// Create command buffer.
	void CreateCommandBuffers(VulkanDevice& _device, VulkanCommandPool& _commandPool, VulkanSwapChain& _swapChain);

public:
	// Class destructor
	virtual ~VulkanCommandBuffers() override = default;


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
	void SubmitAndPresent(IDevice* _device, ISwapChain* _swapChain, IFramebuffers* _framebuffers, uint32_t& _imageIndex) override;

	// Bind a pipeline to draw the next objects with.
	void BindPipeline(IPipeline* _pipeline) override;

	// Draw a specified number of vertices from a given set of vertex/index buffers.
	void DrawVerticesByIndices(uint32_t _vertexNumber, IBuffer* _vertexBuffer, IBuffer* _indexBuffer) override;

	// Bind descriptor sets.
	void BindDescriptorSets(IPipeline* _pipeline, IDescriptorSetsGroup* _descriptorSets) override;

	// Update the uniform buffer descriptor in the given descriptor set.
	void UpdateUniformBuffer(IDevice* _device, IDescriptorSetsGroup* _descSets, IUniformBuffers* _ubo, uint32_t _setIndex, uint32_t _binding, uint32_t _descriptorCount) override;
};