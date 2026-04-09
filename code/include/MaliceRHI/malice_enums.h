#pragma once

// Buffer usage enum.
enum EBufferUsage
{
	VERTEX_BUFFER,
	INDEX_BUFFER
};

// Input topology mode.
enum ETopologyMode
{
	POINT_LIST,
	LINE_LIST,
	LINE_STRIP,
	TRIANGLE_LIST,
	TRIANGLE_STRIP,
	TRIANGLE_FAN
};
// Line drawing mode.
enum EPolygonMode
{
	FILL,
	LINE,
	POINT
};
// Culling mode.
enum ECullMode
{
	CULL_NONE,
	CULL_FRONT_FACE,
	CULL_BACK_FACE,
	CULL_FRONT_AND_BACK
};
// Culling face.
enum EFrontFace
{
	COUNTER_CLOCKWISE,
	CLOCKWISE
};

// Available rendering APIs.
enum ERenderingAPI
{
	VULKAN,
	OPENGL,
	DIRECTX12
};

// Data types supported for the shader (input location for vertex, or push constants).
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
	VEC4,
	MAT2,
	MAT3,
	MAT4
};

// Shader stage enum.
enum EShaderStage
{
	ALL,
	VERTEX_SHADER,
	FRAGMENT_SHADER
};

// Format of a texture/image
enum class ETextureFormat
{
	RGBA8,
	BGRA8,
	DEPTH32
};

// Usage of a texture.
enum class ETextureUsage : uint32_t
{
	NONE = 0,
	SAMPLED = 1 << 0,
	COLOR_ATTACHMENT = 1 << 1,
	DEPTH_ATTACHMENT = 1 << 2
};