#pragma once

#include "Interface/malicerhi_masterheader.h"
#include "Interface/ishadermodules.h"
#include <fstream>

// Forward declarations
class VulkanDevice;

// Vulkan shader modules class
class VulkanShaderModules : public IShaderModules
{
private:
	/// Class properties ///

	// Vertex shader module
	VkShaderModule vertexShader;
	// Vertex shader module
	VkShaderModule fragmentShader;

	// "Total size" of a vertex in this shader (sum of the size of all inputs for a given vertex).
	uint32_t vertexInputTotalSize = 0;
	// Shader vertex input attributes bindings (locations).
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	// Map of "descriptor set layout bindings" per descriptor set index.
	std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> mapDescriptorSetLayoutDescsPerSetIndex;


	/// Helper functions ///

	// Read a file. Static function.
	static std::vector<char> ReadFile(const std::string& _filename);

	// Create shader modules.
	VkShaderModule CreateShaderModule(VulkanDevice& _device, const std::vector<char>& code);

	// Use given structs to build a list of VkVertexInputAttributeDescription and pass it to the graphics pipeline.
	void CreateInputAttributeDescriptions(std::vector<VertexInputLocationParams> _params);

	// Transform a data type from our enum to a VkFormat.
	VkFormat DataTypeToVkFormat(EShaderDataType _type);

public:
	// Class destructor
	virtual ~VulkanShaderModules() override = default;


	/// Lifetime methods ///

	void Create(IDevice* _device, const char* _vertPath, const char* _fragPath, uint32_t _vertexTotalSize, std::vector<VertexInputLocationParams> _vertexInputParams) override ;
	void Destroy(IDevice* _device) override;


	/// Retrieving the backend ///

	VulkanShaderModules& API_Vulkan() override { return (*this); }
	VkShaderModule GetVertexShaderModuleVkHandle() const { return vertexShader; }
	VkShaderModule GetFragmentShaderModuleVkHandle() const { return fragmentShader; }
	uint32_t GetVertexInputTotalSize() const { return vertexInputTotalSize; }
	std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptorsVkHandles() const { return attributeDescriptions; }
	std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> GetMapDescSetLayoutBindingsPerSetIndex() const { return mapDescriptorSetLayoutDescsPerSetIndex; }


	/// Class specfic methods ///

	// Create the parameters for a descriptor set layout binding for a specified set.
	void AddDescriptorSetBinding(uint32_t _setIndex, uint32_t _bindingIndex, uint32_t _descriptorCount, EShaderStage _shaderStage, bool _bIsTextureSampler = false) override;
};