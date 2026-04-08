#pragma once

#include "Interface/malicerhi_masterheader.h"
#include "Interface/ipipeline.h"


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
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	std::vector<std::vector<VkDescriptorSetLayoutBinding>> descriptorSetLayoutBindingsPerSet; // Initialized in CreateDescriptorSetLayouts.
	std::vector<VkPushConstantRange> pushConstants;

	struct VulkanTranslatedParams
	{
		VkPrimitiveTopology inputTopologyMode;
		VkPolygonMode polygonMode;
		VkFrontFace frontFace;
		VkCullModeFlagBits cullingMode;
	};


	/// Helper functions ///

	// Create graphics pipeline.
	void CreateGraphicsPipeline(VulkanDevice& _device, VulkanRenderPass& _renderPass, VulkanShaderModules& _shaders, PipelineParams& _params);
	
	// Create descriptor set layout.
	void CreateDescriptorSetLayouts(VulkanDevice& _device, VulkanShaderModules& _shaders);

	// Translate abstract parameters into Vk parameters for the pipeline.
	VulkanTranslatedParams TranslateAbstractParameters(PipelineParams _params);

public:
	// Class destructor
	virtual ~VulkanPipeline() override = default;


	/// Public methods ///

	void AddPushConstant(EShaderDataType _type, EShaderStage _shaderStage, uint32_t _offset) override;


	/// Lifetime methods ///

	void Create(IDevice* _device, IRenderPass* _renderPass, IShaderModules* _shaders, PipelineParams& _params) override;
	void Destroy(IDevice* _device) override;


	/// Retrieving the backend ///

	VulkanPipeline& API_Vulkan() override { return (*this); }
	VkPipeline GetVkHandle() const { return pipeline; }
	VkPipelineLayout GetPipelineLayoutVkHandle() const { return pipelineLayout; }
	std::vector<VkDescriptorSetLayout> GetDescriptorSetLayoutsVkHandles() const { return descriptorSetLayouts; }
	std::vector<std::vector<VkDescriptorSetLayoutBinding>> GetDescriptorSetLayoutBindingsPerSet() const { return descriptorSetLayoutBindingsPerSet; }
	std::vector<VkPushConstantRange> GetPushConstantsVkHandles() const { return pushConstants; }
};