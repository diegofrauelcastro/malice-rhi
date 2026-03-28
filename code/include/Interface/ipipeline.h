#pragma once

#include <iostream>


// Forward declarations
class VulkanPipeline;
class IDevice;
class IBuffer;
class IRenderPass;
class IShaderModules;
struct GLFWwindow;

// Abstraction enums
enum ETopologyMode
{
	POINT_LIST,
	LINE_LIST,
	LINE_STRIP,
	TRIANGLE_LIST,
	TRIANGLE_STRIP,
	TRIANGLE_FAN
};

enum EPolygonMode
{
	FILL,
	LINE,
	POINT
};

enum ECullMode
{
	CULL_NONE,
	CULL_FRONT_FACE,
	CULL_BACK_FACE,
	CULL_FRONT_AND_BACK
};

enum EFrontFace
{
	COUNTER_CLOCKWISE,
	CLOCKWISE
};


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


	/// Lifetime methods ///

	virtual void Create(IDevice* _device, IRenderPass* _renderPass, IShaderModules* _shaders, PipelineParams& _params) = 0;
	virtual void Destroy(IDevice* _device) = 0;


	/// Retrieving the backend ///

	virtual VulkanPipeline& API_Vulkan() { throw std::runtime_error("Bad API call : object is not a VulkanPipeline."); }
};