#include "vulkan_shadermodules.h"

#include "vulkan_device.h"


std::vector<char> VulkanShaderModules::ReadFile(const std::string& _filename)
{
	// Start reading the file from the end to get the file size easily. Also, open it in binary mode.
	std::ifstream file(_filename, std::ios::ate | std::ios::binary);
	LOG_RHI("Reading %s", _filename.c_str())

	if (!file.is_open())
		LOG_THROW("/!\\ Failed to open \"%s\" file!", _filename.c_str())


	// Get the file size and allocate a buffer accordingly.
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	// Go back to the beginning of the file and read all its content into the buffer.
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	// Close the file and return it.
	file.close();
	return buffer;
}

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
		LOG_THROW("/!\\ Failed to create shader module!")
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
	case BOOL:
	case UINT:
		return VK_FORMAT_R32_UINT;
	case INT:
		return VK_FORMAT_R32_SINT;
	case FLOAT:
		return VK_FORMAT_R32_SFLOAT;
	case DOUBLE:
		return VK_FORMAT_R64_SFLOAT;
	case VEC2:
		return VK_FORMAT_R32G32_SFLOAT;
	case VEC3:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case VEC4:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case NONE:
	default:
		return VK_FORMAT_UNDEFINED;
	}
}

void VulkanShaderModules::AddDescriptorSetBinding(uint32_t _setIndex, uint32_t _bindingIndex, uint32_t _descriptorCount, EShaderStage _shaderStage)
{
	VkShaderStageFlags shaderStageVkEquivalent;
	switch (_shaderStage)
	{
	case ALL:
		shaderStageVkEquivalent = VK_SHADER_STAGE_ALL;
		break;
	case FRAGMENT_SHADER:
		shaderStageVkEquivalent = VK_SHADER_STAGE_FRAGMENT_BIT;
		break;
	case VERTEX_SHADER:
	default:
		shaderStageVkEquivalent = VK_SHADER_STAGE_VERTEX_BIT;
		break;
	}

	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = _bindingIndex;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = _descriptorCount;
	uboLayoutBinding.stageFlags = shaderStageVkEquivalent;
	mapDescriptorSetLayoutDescsPerSetIndex[_setIndex].push_back(uboLayoutBinding);
	LOG_RHI("Registered shader's descriptor set %d (binding %d) with %d descriptors.", (int)_setIndex, (int)_bindingIndex, (int)_descriptorCount)
}

void VulkanShaderModules::Create(IDevice* _device, const char* _vertPath, const char* _fragPath, uint32_t _vertexTotalSize, std::vector<VertexInputLocationParams> _vertexInputParams)
{
	LOG_CLEAN("\n\n===== SHADER MODULES CREATION =====\n")

	// Read both precompiled shader files.
	std::vector<char> vertexShaderCode = ReadFile(_vertPath);
	std::vector<char> fragmentShaderCode = ReadFile(_fragPath); // The path looks like this because the default directory is the one where the executable is.

	// Make a module for each of them.
	LOG_RHI("Creating vertex shader module...")
	vertexShader = CreateShaderModule(_device->API_Vulkan(), vertexShaderCode);
	LOG_RHI("Creating fragment shader module...")
	fragmentShader = CreateShaderModule(_device->API_Vulkan(), fragmentShaderCode);

	// Initialize vertex input locations.
	vertexInputTotalSize = _vertexTotalSize;
	LOG_CLEAN("")
	LOG_RHI("Initializing vertex input...")
	CreateInputAttributeDescriptions(_vertexInputParams);
	LOG_CLEAN("")
}

void VulkanShaderModules::Destroy(IDevice* _device)
{
	LOG_CLEAN("\n\n===== SHADER MODULES DESTRUCTION =====\n")

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