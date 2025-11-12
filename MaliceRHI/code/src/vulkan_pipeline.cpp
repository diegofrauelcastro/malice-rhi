#include "vulkan_pipeline.h"

#include "vulkan_device.h"
#include "vulkan_renderpass.h"
#include "vulkan_shadermodules.h"


void VulkanPipeline::CreateGraphicsPipeline(VulkanDevice& _device, VulkanRenderPass& _renderPass, VulkanShaderModules& _shaders)
{
	// Create info for the vertex shader.
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = _shaders.GetVertexShaderModuleVkHandle();
	vertShaderStageInfo.pName = "main";

	// Create info for the fragment shader.
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = _shaders.GetFragmentShaderModuleVkHandle();
	fragShaderStageInfo.pName = "main";

	// Store these two shader stages in an array for later use in the pipeline creation.
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// Get binding and attribute descriptions from the Vertex struct.
	VkVertexInputBindingDescription bindingDescription {};
	bindingDescription.binding = 0;
	bindingDescription.stride = _shaders.GetVertexInputTotalSize();
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = _shaders.GetAttributeDescriptorsVkHandles();

	// Create info about the vertex input memory alignment and space.
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	// Create info about the vertex input memory alignment and space.	
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	/*VkVertexInputBindingDescription bindingDescription = Vertex::GetBindingDescription();	// TODO delete if the struct solution works
	auto attributeDescriptions = Vertex::GetAttributeDescriptions();*/

	// Get binding and attribute descriptions from the Vertex struct.
	/*VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;*/

	// Create info about how the given vertices should be assembled.
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Create info about the viewport and the scissors.
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	// Setup the rasterizer.
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	// Setup anti-aliasing.
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// Configure color blending.
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	// Set window to be resizable at runtime (dynamic).
	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	// Create info about our pipeline.
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	/*pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;*/ // TODO Implement layout count (for uniform buffers).
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	// Create the pipeline and ensure it was created successfully.
	VkResult pipelineLayoutResult = vkCreatePipelineLayout(_device.GetLogicalDeviceVkHandle(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
	if (pipelineLayoutResult != VK_SUCCESS)
		LOG_THROW("/!\\ Failed to create pipeline layout!")

	// Finally we create the info about our pipeline.
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	// Link all info created above.

	// Shader stages
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.stageCount = 2;

	// Fixed-function stages
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;

	// Pipeline layout.
	pipelineInfo.layout = pipelineLayout;

	// Render pass.
	pipelineInfo.renderPass = _renderPass.GetVkHandle();
	pipelineInfo.subpass = 0;

	// Create the graphics pipeline and ensure it succeeded.
	VkResult pipelineResult = vkCreateGraphicsPipelines(_device.GetLogicalDeviceVkHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
	if (pipelineResult != VK_SUCCESS)
		LOG_THROW("/!\\ Failed to create graphics pipeline!")
}

void VulkanPipeline::Create(IDevice* _device, IRenderPass* _renderPass, IShaderModules* _shaders)
{
	CreateGraphicsPipeline(_device->API_Vulkan(), _renderPass->API_Vulkan(), _shaders->API_Vulkan());
}

void VulkanPipeline::Destroy(IDevice* _device)
{
	// Destroy the graphics pipeline.
	if (pipeline != VK_NULL_HANDLE)
		vkDestroyPipeline(_device->API_Vulkan().GetLogicalDeviceVkHandle(), pipeline, nullptr);

	// Destroy the pipeline's layout.
	if (pipelineLayout != VK_NULL_HANDLE)
		vkDestroyPipelineLayout(_device->API_Vulkan().GetLogicalDeviceVkHandle(), pipelineLayout, nullptr);
}
