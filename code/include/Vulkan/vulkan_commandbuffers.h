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
	VkExtent2D renderExtent;

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


	/// Command methods ///
	
	// Bind a pipeline to draw the next objects with. This method should be called within the scope of a BeginFrame(...)/EndFrame().
	void BindPipeline(IPipeline* _pipeline) override;

	// Bind descriptor sets. This method should be called within the scope of a BeginFrame(...)/EndFrame().
	void BindDescriptorSets(IPipeline* _pipeline, IDescriptorSetsGroup* _descriptorSets) override;

	// Update the uniform buffer descriptor in the given descriptor set, at the indicated set and binding indices. This method should be called within the scope of a BeginFrame(...)/EndFrame().
	void UpdateUniformBuffer(IDevice* _device, IDescriptorSetsGroup* _descSets, IUniformBuffers* _ubo, uint32_t _setIndex, uint32_t _binding, uint32_t _descriptorCount) override;

	// Update the bound texture in the given descriptor set, at the indicated set and binding indices. This method should be called within the scope of a BeginFrame(...)/EndFrame().
	void UpdateTexture(IDevice* _device, IDescriptorSetsGroup* _descSets, ITexture* _tex, uint32_t _setIndex, uint32_t _binding) override;

	// Begin rendering with a render pass. This works for both on-screen and offscreen rendering. For offscreen rendering, _framebufferIndex should be equal to 0. Otherwise, please use the _imageIndex given by the BeginFrame(...) method. This call MUST be followed by an EndRender() call once all the draw commands are done. Needless to say this method should be called inside the scope of a BeginFrame(...)/EndFrame().
	void BeginRender(IRenderPass* _renderPass, IFramebuffers* _framebuffers, uint32_t _framebufferIndex = 0) override;

	// End rendering with a render pass (any that was active until now). This call comes AFTER a BeginRender(...) method call.
	void EndRender() override;

	// Submit command buffer and present the image to the screen. This method should be called AFTER an EndFrame() method call.
	void SubmitAndPresent(IDevice* _device, ISwapChain* _swapChain, IFramebuffers* _framebuffers, uint32_t& _imageIndex) override;

	// Draw a specified number of vertices from a given set of vertex/index buffers. This method should be called within the scope of a BeginRender(...)/EndRender().
	void DrawVerticesByIndices(uint32_t _vertexNumber, IBuffer* _vertexBuffer, IBuffer* _indexBuffer) override;


	/// Class specific methods ///

	// Get handle to the current command buffer.
	VkCommandBuffer GetCurrentCommandBuffer() const { return commandBuffers[currentFrame]; }

	// Begin recording commands for a frame's rendering process. After all rendering commands are done, the user MUST call EndFrame() before submitting the frame.
	virtual bool BeginFrame(IDevice* _device, ISwapChain* _swapChain, uint32_t& _imageIndex);

	// End recording commands for a frame's rendering process. Should be called AFTER a BeginFrame(...).
	virtual void EndFrame();

};