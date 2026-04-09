#include "Vulkan/vulkan_pipeline.h"

#include "Vulkan/vulkan_device.h"
#include "Vulkan/vulkan_renderpass.h"
#include "Vulkan/vulkan_shadermodules.h"

void VulkanPipeline::CreateGraphicsPipeline(VulkanDevice& _device, VulkanRenderPass& _renderPass, VulkanShaderModules& _shaders, PipelineParams& _params)
{
	LOG_RHI_CLEAN("\n\n===== GRAPHICS PIPELINE CREATION =====\n")

	// Create info for the vertex shader.
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = _shaders.GetVertexShaderModuleVkHandle();
	vertShaderStageInfo.pName = "main";
	LOG_RHI("Vertex shader linked.")

	// Create info for the fragment shader.
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = _shaders.GetFragmentShaderModuleVkHandle();
	fragShaderStageInfo.pName = "main";
	LOG_RHI("Fragment shader linked.")

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

	// User parameters translated for vulkan use.
	VulkanTranslatedParams translatedParams = TranslateAbstractParameters(_params);

	// Create info about how the given vertices should be assembled.
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = translatedParams.inputTopologyMode;
	inputAssembly.primitiveRestartEnable = _params.enablePrimitiveRestart;

	// Create info about the viewport and the scissors.
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	// Setup the rasterizer.
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = _params.enableDepthClamp;
	rasterizer.rasterizerDiscardEnable = _params.enableRasterizerDiscard;
	rasterizer.polygonMode = translatedParams.polygonMode;
	rasterizer.lineWidth = _params.rasterizerLineWidth;
	rasterizer.cullMode = translatedParams.cullingMode;
	rasterizer.frontFace = translatedParams.frontFace;
	rasterizer.depthBiasEnable = _params.enableDepthBias;

	// Depth and stencil.
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	if (hasDepth)
	{
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {}; // Optional
		depthStencil.back = {}; // Optional
	}

	// Setup anti-aliasing.
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// Configure color blending.
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = _params.enableColorBlend;

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

	// Initialize info about the descriptor set layouts.
	CreateDescriptorSetLayouts(_device, _shaders);

	// Create info about our pipeline.
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
	pipelineLayoutInfo.pushConstantRangeCount = pushConstants.size();

	// Create the pipeline and ensure it was created successfully.
	VkResult pipelineLayoutResult = vkCreatePipelineLayout(_device.GetLogicalDeviceVkHandle(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
	if (pipelineLayoutResult != VK_SUCCESS)
		LOG_RHI_THROW("/!\\ Failed to create pipeline layout!")
	else
		LOG_RHI("Pipeline layout created successfully.")

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
	pipelineInfo.pDepthStencilState = hasDepth ? &depthStencil : nullptr; // Enable depth/stencil only when the given render pass has depth enabled.
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;

	// Pipeline layout.
	pipelineInfo.layout = pipelineLayout;

	// Render pass.
	pipelineInfo.renderPass = _renderPass.GetVkHandle();
	pipelineInfo.subpass = 0;

	LOG_RHI("Finished configuring graphics pipeline.")

	// Create the graphics pipeline and ensure it succeeded.
	VkResult pipelineResult = vkCreateGraphicsPipelines(_device.GetLogicalDeviceVkHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
	if (pipelineResult != VK_SUCCESS)
		LOG_RHI_THROW("/!\\ Failed to create graphics pipeline!")
	else
		LOG_RHI("Graphics pipeline created successfully.")
}

void VulkanPipeline::CreateDescriptorSetLayouts(VulkanDevice& _device, VulkanShaderModules& _shaders)
{
	// Retrieve all existing descriptor set indices and sort them in ascending order.
	std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> mapTemp = _shaders.GetMapDescSetLayoutBindingsPerSetIndex();
	std::vector<uint32_t> existingSetIndices;
	for (auto it = mapTemp.begin(); it != mapTemp.end(); it++)
		existingSetIndices.push_back(it->first);
	sort(existingSetIndices.begin(), existingSetIndices.end());	// Ascending order sort O(n log n).

	// Create all the descriptor set layouts.
	descriptorSetLayouts.resize(existingSetIndices.size());
	// Also prepare to store the descriptor set layout bindings per set for later use.
	descriptorSetLayoutBindingsPerSet.resize(existingSetIndices.size());

	LOG_RHI_CLEAN("")
	LOG_RHI("Creating descriptor set layouts for each descriptor set...")
	for (uint32_t i = 0; i < existingSetIndices.size(); i++)
	{
		// Throw an error if for example the user defined set 0 and 2 but forgot set 1.
		if (i != existingSetIndices[i])
			LOG_RHI_THROW("/!\\ There seems to be a missing index for the descriptor sets? Last recorded index is %d, current index %d <- fix this one please.\n/!\\ If last recorded index is -1, it means descriptor set 0 is missing.", (int)(i-1), (int)existingSetIndices[i])

		// Create info about the descriptor set layout.
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = (uint32_t)mapTemp[i].size();
		layoutInfo.pBindings = mapTemp[i].data();

		// Create the descriptor set layout and ensure it succeeded.
		VkResult result = vkCreateDescriptorSetLayout(_device.GetLogicalDeviceVkHandle(), &layoutInfo, nullptr, &descriptorSetLayouts[i]);
		if (result != VK_SUCCESS)
			LOG_RHI_THROW("/!\\ Failed to create descriptor set layout for descriptor set number %d!", (int)i)
		else
			LOG_RHI("Descriptor set %d's layout created successfully.", (int)i)
		
		// Store the descriptor set layout bindings for later use.
		descriptorSetLayoutBindingsPerSet[i] = mapTemp[i];
	}
	// Clear the temporary map.
	for (auto it = mapTemp.begin(); it != mapTemp.end(); it++)
	{
		it->second.clear();
		it->second.shrink_to_fit();
	}
	mapTemp.clear();

	LOG_RHI("Finished creating descriptor set layouts.")
	LOG_RHI_CLEAN("")
}

VulkanPipeline::VulkanTranslatedParams VulkanPipeline::TranslateAbstractParameters(PipelineParams _params)
{
	VulkanTranslatedParams newParams;
	// Primitive topology
	switch (_params.inputTopologyMode)
	{
	default:
	case MRHI_POINT_LIST:
		newParams.inputTopologyMode = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		break;
	case MRHI_LINE_LIST:
		newParams.inputTopologyMode = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		break;
	case MRHI_LINE_STRIP:
		newParams.inputTopologyMode = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		break;
	case MRHI_TRIANGLE_LIST:
		newParams.inputTopologyMode = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		break;
	case MRHI_TRIANGLE_STRIP:
		newParams.inputTopologyMode = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		break;
	case MRHI_TRIANGLE_FAN:
		newParams.inputTopologyMode = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		break;
	}
	// Polygon mode
	switch (_params.polygonMode)
	{
	default:
	case MRHI_FILL:
		newParams.polygonMode = VK_POLYGON_MODE_FILL;
		break;
	case MRHI_LINE:
		newParams.polygonMode = VK_POLYGON_MODE_LINE;
		break;
	case MRHI_POINT:
		newParams.polygonMode = VK_POLYGON_MODE_POINT;
		break;
	}
	// Front face
	switch (_params.frontFace)
	{
	default:
	case MRHI_COUNTER_CLOCKWISE:
		newParams.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		break;
	case MRHI_CLOCKWISE:
		newParams.frontFace = VK_FRONT_FACE_CLOCKWISE;
		break;
	}
	// Culling mode
	switch (_params.cullingMode)
	{
	default:
	case MRHI_CULL_NONE:
		newParams.cullingMode = VK_CULL_MODE_NONE;
		break;
	case MRHI_CULL_FRONT_FACE:
		newParams.cullingMode = VK_CULL_MODE_FRONT_BIT;
		break;
	case MRHI_CULL_BACK_FACE:
		newParams.cullingMode = VK_CULL_MODE_BACK_BIT;
		break;
	case MRHI_CULL_FRONT_AND_BACK:
		newParams.cullingMode = VK_CULL_MODE_FRONT_AND_BACK;
		break;
	}
	return newParams;
}

void VulkanPipeline::AddPushConstant(EShaderDataType _type, EShaderStage _shaderStage, uint32_t _offset)
{
	VkPushConstantRange newPushConstant{};
	newPushConstant.offset = _offset;
	switch (_type)
	{
	default:
	case MRHI_NONE:
		newPushConstant.size = 0;
		break;
	case MRHI_BOOL:
		newPushConstant.size = sizeof(bool);
		break;
	case MRHI_INT:
		newPushConstant.size = sizeof(int);
		break;
	case MRHI_UINT:
		newPushConstant.size = sizeof(uint32_t);
		break;
	case MRHI_FLOAT:
		newPushConstant.size = sizeof(float);
		break;
	case MRHI_DOUBLE:
		newPushConstant.size = sizeof(double);
		break;
	case MRHI_VEC2:
		newPushConstant.size = sizeof(float) * 2;
		break;
	case MRHI_VEC3:
		newPushConstant.size = sizeof(float) * 3;
		break;
	case MRHI_VEC4:
		newPushConstant.size = sizeof(float) * 4;
		break;
	case MRHI_MAT2:
		newPushConstant.size = sizeof(float) * 4;
		break;
	case MRHI_MAT3:
		newPushConstant.size = sizeof(float) * 9;
		break;
	case MRHI_MAT4:
		newPushConstant.size = sizeof(float) * 16;
		break;
	}
	switch (_shaderStage)
	{
	default:
	case MRHI_ALL:
		newPushConstant.stageFlags = VK_SHADER_STAGE_ALL;
		break;
	case MRHI_VERTEX_SHADER:
		newPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		break;
	case MRHI_FRAGMENT_SHADER:
		newPushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		break;
	}
	pushConstants.push_back(newPushConstant);
}

void VulkanPipeline::Create(IDevice* _device, IRenderPass* _renderPass, IShaderModules* _shaders, PipelineParams& _params)
{
	hasDepth = _renderPass->GetHasDepth();
	CreateGraphicsPipeline(_device->API_Vulkan(), _renderPass->API_Vulkan(), _shaders->API_Vulkan(), _params);
}

void VulkanPipeline::Destroy(IDevice* _device)
{
	LOG_RHI("\n\n===== GRAPHICS PIPELINE DESTRUCTION =====\n")

	// Destroy the descriptor set layouts.
	for (size_t i = 0; i < descriptorSetLayouts.size(); i++)
	{
		if (descriptorSetLayouts[i] != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(_device->API_Vulkan().GetLogicalDeviceVkHandle(), descriptorSetLayouts[i], nullptr);
			LOG_RHI("Descriptor set %d's layout destroyed successfully.", (int)i)
		}
		else
			LOG_RHI("Something went wrong trying to destroy descriptor set %d's layout...", (int)i)
	}

	// Clear vector.
	descriptorSetLayouts.clear();
	descriptorSetLayouts.shrink_to_fit();
	// Clear vector of descriptor set layout bindings per set.
	descriptorSetLayoutBindingsPerSet.clear();
	descriptorSetLayoutBindingsPerSet.shrink_to_fit();

	// Destroy the graphics pipeline.
	if (pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(_device->API_Vulkan().GetLogicalDeviceVkHandle(), pipeline, nullptr);
		LOG_RHI("Graphics pipeline destroyed successfully.")
	}
	else
		LOG_RHI("Something went wrong trying to destroy a graphics pipeline...")

	// Destroy the pipeline's layout.
	if (pipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(_device->API_Vulkan().GetLogicalDeviceVkHandle(), pipelineLayout, nullptr);
		LOG_RHI("Pipeline layout destroyed successfully.")
	}
	else
		LOG_RHI("Something went wrong trying to destroy a pipeline layout...")
}
