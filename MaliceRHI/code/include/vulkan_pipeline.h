#pragma once

#include "malicerhi_masterheader.h"
#include "ipipeline.h"


// Forward declarations
class VulkanRenderPass;
class VulkanDevice;
class VulkanShaderModules;

// Vulkan pipeline class
class VulkanPipeline : public IPipeline
{
private:
	/// Class properties ///

	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;

	/// Helper functions ///

	// Create graphics pipeline.
	void CreateGraphicsPipeline(VulkanDevice& _device, VulkanRenderPass& _renderPass, VulkanShaderModules& _shaders);
	
public:
	/// Lifetime methods ///

	void Create(IDevice* _device, IRenderPass* _renderPass, IShaderModules* _shaders) override;
	void Destroy(IDevice* _device) override;


	/// Retrieving the backend ///

	VulkanPipeline& API_Vulkan() override { return (*this); }
	VkPipeline GetVkHandle() const { return pipeline; }
};