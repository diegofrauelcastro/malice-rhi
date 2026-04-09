#pragma once

#include <iostream>
#include "MaliceRHI/malice_enums.h"


// Forward declarations
class VulkanPipeline;
class IDevice;
class IBuffer;
class IRenderPass;
class IShaderModules;
struct GLFWwindow;


// Param structs
struct PipelineParams
{
	ETopologyMode inputTopologyMode = TRIANGLE_LIST;
	EPolygonMode polygonMode = FILL;
	EFrontFace frontFace = COUNTER_CLOCKWISE;
	ECullMode cullingMode = CULL_BACK_FACE;
	float rasterizerLineWidth = 1.0f;
	bool enableRasterizerDiscard = false;
	bool enableDepthClamp = false;
	bool enableDepthBias = false;
	bool enableColorBlend = false;
	bool enablePrimitiveRestart = false;
};

// Pipeline interface
class IPipeline
{
protected:
	// Class properties

	bool hasDepth = false;

public:
	// Class destructor
	virtual ~IPipeline() = default;


	/// Public methods ///

	virtual void AddPushConstant(EShaderDataType _type, EShaderStage _shaderStage, uint32_t _offset) = 0;


	/// Lifetime methods ///

	virtual void Create(IDevice* _device, IRenderPass* _renderPass, IShaderModules* _shaders, PipelineParams& _params) = 0;
	virtual void Destroy(IDevice* _device) = 0;


	/// Retrieving the backend ///

	virtual VulkanPipeline& API_Vulkan() { throw std::runtime_error("Bad API call : object is not a VulkanPipeline."); }
};