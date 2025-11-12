#include "vulkan_shadermodules.h"

#include "vulkan_device.h"


std::vector<char> VulkanShaderModules::ReadFile(const std::string& _filename)
{
	// Start reading the file from the end to get the file size easily. Also, open it in binary mode.
	std::ifstream file(_filename, std::ios::ate | std::ios::binary);
	std::cout << "Reading " << _filename << std::endl;

	if (!file.is_open())
		throw std::runtime_error("/!\\ Failed to open \"" + _filename + "\" file!");


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
		throw std::runtime_error("/!\\ Failed to create shader module!");
	return shaderModule;
}

void VulkanShaderModules::CreateDescriptorSetLayout(VulkanDevice& _device)
{
	// Create a layout binding for the future uniform buffer object (UBO).
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;		// Our UBO will be used in the vertex shader.

	// Create info about the descriptor set layout.
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	// Create the descriptor set layout and ensure it succeeded.
	VkResult result = vkCreateDescriptorSetLayout(_device.GetLogicalDeviceVkHandle(), &layoutInfo, nullptr, &descriptorSetLayout);
	if (result != VK_SUCCESS)
		throw std::runtime_error("/!\\ Failed to create descriptor set layout!");
}

std::vector<VkVertexInputAttributeDescription> VulkanShaderModules::CreateInputAttributeDescriptions(std::vector<VertexInputLocationParams> _params)
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	attributeDescriptions.resize(_params.size());

	for (int i = 0; i < _params.size(); i++)
	{
		attributeDescriptions[i].binding = 0;
		attributeDescriptions[i].location = _params[i].location;
		attributeDescriptions[i].format = DataTypeToVkFormat(_params[i].type);
		attributeDescriptions[i].offset = _params[i].offset;
	}

	return attributeDescriptions;
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

void VulkanShaderModules::Create(IDevice* _device, const char* _vertPath, const char* _fragPath, uint32_t _vertexTotalSize, std::vector<VertexInputLocationParams> _params)
{
	// Read both precompiled shader files.
	std::vector<char> vertexShaderCode = ReadFile(_vertPath);
	std::vector<char> fragmentShaderCode = ReadFile(_fragPath); // The path looks like this because the default directory is the one where the executable is.

	// Make a module for each of them.
	vertexShader = CreateShaderModule(_device->API_Vulkan(), vertexShaderCode);
	fragmentShader = CreateShaderModule(_device->API_Vulkan(), fragmentShaderCode);

	// Initialize vertex input locations.
	vertexInputTotalSize = _vertexTotalSize;
	attributeDescriptions = CreateInputAttributeDescriptions(_params);
}

void VulkanShaderModules::Destroy(IDevice* _device)
{
	// Destroy both modules.

	if (fragmentShader != VK_NULL_HANDLE)
		vkDestroyShaderModule(_device->API_Vulkan().GetLogicalDeviceVkHandle(), fragmentShader, nullptr);
	if (vertexShader != VK_NULL_HANDLE)
		vkDestroyShaderModule(_device->API_Vulkan().GetLogicalDeviceVkHandle(), vertexShader, nullptr);
}
