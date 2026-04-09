#pragma once

#include <iostream>
#include <vector>
#include "MaliceRHI/malice_enums.h"


// Forward declarations
class VulkanShaderModules;
class IDevice;

// Struct for vertex input location params.
struct VertexInputLocationParams
{
	uint32_t location = 0;
	uint32_t memoryOffset = 0;
	EShaderDataType type = NONE;
};

// Shader modules interface
class IShaderModules
{
public:
	// Class destructor
	virtual ~IShaderModules() = default;


	/// Lifetime methods ///

	virtual void Create(IDevice* _device, const std::vector<char>& _vertSrc, const std::vector<char>& _fragSrc, uint32_t _vertexTotalSize, std::vector<VertexInputLocationParams> _vertexInputParams) = 0;
	virtual void Destroy(IDevice* _device) = 0;


	/// Retrieving the backend ///

	virtual VulkanShaderModules& API_Vulkan() { throw std::runtime_error("Bad API call : object is not a VulkanShaderModules."); }


	/// Class specific methods ///

	// Create the parameters for a descriptor set layout binding for a specified set.
	virtual void AddDescriptorSetBinding(uint32_t _setIndex, uint32_t _bindingIndex, uint32_t _descriptorCount, EShaderStage _shaderStage, bool _bIsTextureSampler = false) = 0;
};