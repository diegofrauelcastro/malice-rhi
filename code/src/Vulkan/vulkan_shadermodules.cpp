#include "Vulkan/vulkan_shadermodules.h"

#include "Vulkan/vulkan_device.h"


VkShaderModule VulkanShaderModules::CreateShaderModule(VulkanDevice& _device, const std::vector<char>& _code)
{
	// Create info for the shader module. We only need a pointer to the bytecode, and the size of it.
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = _code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(_code.data());

	// Create the bytecode and ensure it was a success.
	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(_device.GetLogicalDeviceVkHandle(), &createInfo, nullptr, &shaderModule);
	if (result != VK_SUCCESS)
		LOG_RHI_THROW("/!\\ Failed to create shader module!")
	else
		LOG_RHI("Shader module created successfully.")
	return shaderModule;
}

void VulkanShaderModules::CreateInputAttributeDescriptions(std::vector<VertexInputLocationParams> _params)
{
	attributeDescriptions.resize(_params.size());

	for (int i = 0; i < _params.size(); i++)
	{
		attributeDescriptions[i].binding = 0;
		attributeDescriptions[i].location = _params[i].location;
		attributeDescriptions[i].format = DataTypeToVkFormat(_params[i].type);
		attributeDescriptions[i].offset = _params[i].memoryOffset;
		LOG_RHI("Initialized shader vertex input at location %d", (int)_params[i].location)
	}
}

VkFormat VulkanShaderModules::DataTypeToVkFormat(EShaderDataType _type)
{
	switch (_type)
	{
	case MRHI_BOOL:
	case MRHI_UINT:
		return VK_FORMAT_R32_UINT;
	case MRHI_INT:
		return VK_FORMAT_R32_SINT;
	case MRHI_FLOAT:
		return VK_FORMAT_R32_SFLOAT;
	case MRHI_DOUBLE:
		return VK_FORMAT_R64_SFLOAT;
	case MRHI_MAT2:
	case MRHI_VEC2:
		return VK_FORMAT_R32G32_SFLOAT;
	case MRHI_MAT3:
	case MRHI_VEC3:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case MRHI_MAT4:
	case MRHI_VEC4:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case MRHI_NONE:
	default:
		return VK_FORMAT_UNDEFINED;
	}
}

void VulkanShaderModules::AddDescriptorSetBinding(uint32_t _setIndex, uint32_t _bindingIndex, uint32_t _descriptorCount, EShaderStage _shaderStage, bool _bIsTextureSampler)
{
	VkShaderStageFlags shaderStageVkEquivalent;
	switch (_shaderStage)
	{
	case MRHI_ALL:
		shaderStageVkEquivalent = VK_SHADER_STAGE_ALL;
		break;
	case MRHI_FRAGMENT_SHADER:
		shaderStageVkEquivalent = VK_SHADER_STAGE_FRAGMENT_BIT;
		break;
	case MRHI_VERTEX_SHADER:
	default:
		shaderStageVkEquivalent = VK_SHADER_STAGE_VERTEX_BIT;
		break;
	}

	VkDescriptorSetLayoutBinding newLayoutBinding{};
	newLayoutBinding.binding = _bindingIndex;
	newLayoutBinding.descriptorType = _bIsTextureSampler ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	newLayoutBinding.descriptorCount = _descriptorCount;
	newLayoutBinding.stageFlags = shaderStageVkEquivalent;
	mapDescriptorSetLayoutDescsPerSetIndex[_setIndex].push_back(newLayoutBinding);
	LOG_RHI("Registered shader's descriptor set %d (binding %d) with %d descriptors.", (int)_setIndex, (int)_bindingIndex, (int)_descriptorCount)
}

void VulkanShaderModules::Create(IDevice* _device, const std::vector<char>& _vertSrc, const std::vector<char>& _fragSrc, uint32_t _vertexTotalSize, std::vector<VertexInputLocationParams> _vertexInputParams)
{
	LOG_RHI_CLEAN("\n\n===== SHADER MODULES CREATION =====\n")

	// Make a module for each of them.
	LOG_RHI("Creating vertex shader module...")
	vertexShader = CreateShaderModule(_device->API_Vulkan(), _vertSrc);
	LOG_RHI("Creating fragment shader module...")
	fragmentShader = CreateShaderModule(_device->API_Vulkan(), _fragSrc);

	// Initialize vertex input locations.
	vertexInputTotalSize = _vertexTotalSize;
	LOG_RHI_CLEAN("")
	LOG_RHI("Initializing vertex input...")
	CreateInputAttributeDescriptions(_vertexInputParams);
	LOG_RHI_CLEAN("")
}

void VulkanShaderModules::Destroy(IDevice* _device)
{
	LOG_RHI_CLEAN("\n\n===== SHADER MODULES DESTRUCTION =====\n")

	// Destroy both modules.

	if (fragmentShader != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(_device->API_Vulkan().GetLogicalDeviceVkHandle(), fragmentShader, nullptr);
		LOG_RHI("Fragment shader destroyed successfully.")
	}
	else
		LOG_RHI("Something went wrong trying to destroy fragment shader...")
	if (vertexShader != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(_device->API_Vulkan().GetLogicalDeviceVkHandle(), vertexShader, nullptr);
		LOG_RHI("Vertex shader destroyed successfully.")
	}
	else
		LOG_RHI("Something went wrong trying to destroy vertex shader...")

	// Clear attributeDescriptions vectors.
	attributeDescriptions.clear();
	attributeDescriptions.shrink_to_fit();

	// Clear vectors of descriptor set layout bindings for each descriptor set index in the map.
	for (auto it = mapDescriptorSetLayoutDescsPerSetIndex.begin(); it != mapDescriptorSetLayoutDescsPerSetIndex.end(); it++)
	{
		it->second.clear();
		it->second.shrink_to_fit();
	}
	// Clear the map itself.
	mapDescriptorSetLayoutDescsPerSetIndex.clear();
}