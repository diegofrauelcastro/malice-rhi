#pragma once

#include <iostream>
#include <vector>


// Forward declarations
class VulkanShaderModules;
class IDevice;

// Data types supported for the shader.
enum EShaderDataType
{
	NONE,
	BOOL,
	INT,
	UINT,
	FLOAT,
	DOUBLE,
	VEC2,
	VEC3,
	VEC4
};

// Struct vertex input location params.
struct VertexInputLocationParams
{
	uint32_t location = 0;
	EShaderDataType type = NONE;
	uint32_t offset = 0;
};

// Shader modules interface
class IShaderModules
{
public:
	// Class destructor
	virtual ~IShaderModules() = default;


	/// Lifetime methods ///

	virtual void Create(IDevice* _device, const char* _vertPath, const char* _fragPath, uint32_t _vertexTotalSize, std::vector<VertexInputLocationParams> _params) = 0;
	virtual void Destroy(IDevice* _device) = 0;


	/// Retrieving the backend ///

	virtual VulkanShaderModules& API_Vulkan() { throw std::runtime_error("Bad API call : object is not a VulkanShaderModules."); }
};