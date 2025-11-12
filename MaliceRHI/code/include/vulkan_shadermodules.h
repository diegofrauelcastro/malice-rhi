#pragma once

#include "malicerhi_masterheader.h"
#include "ishadermodules.h"
#include <fstream>

// Forward declarations
class VulkanDevice;

// Shader modules interface
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

	// TODO
	VkDescriptorSetLayout descriptorSetLayout;


	/// Helper function ///

	// Read a file. Static function.
	static std::vector<char> ReadFile(const std::string& _filename);

	// Create shader modules.
	VkShaderModule CreateShaderModule(VulkanDevice& _device, const std::vector<char>& code);

	// Create descriptor set layout.
	void CreateDescriptorSetLayout(VulkanDevice& _device); // TODO Adjust to give the user more control.

	// Use given structs to build a VkVertexInputAttributeDescription and pass it to the graphics pipeline.
	std::vector<VkVertexInputAttributeDescription> CreateInputAttributeDescriptions(std::vector<VertexInputLocationParams> _params);

	// Transform a data type from our enum to a VkFormat.
	VkFormat DataTypeToVkFormat(EShaderDataType _type);

public:
	/// Lifetime methods ///

	void Create(IDevice* _device, const char* _vertPath, const char* _fragPath, uint32_t _vertexTotalSize, std::vector<VertexInputLocationParams> _params) override ;
	void Destroy(IDevice* _device) override;


	/// Retrieving the backend ///

	VulkanShaderModules& API_Vulkan() override { return (*this); }
	VkShaderModule GetVertexShaderModuleVkHandle() const { return vertexShader; }
	VkShaderModule GetFragmentShaderModuleVkHandle() const { return fragmentShader; }
	VkDescriptorSetLayout GetDescriptorSetLayoutVkHandle() const { return descriptorSetLayout; }
	uint32_t GetVertexInputTotalSize() const { return vertexInputTotalSize; }
	std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptorsVkHandles() const { return attributeDescriptions; }
};