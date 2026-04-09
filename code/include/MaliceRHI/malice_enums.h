#pragma once

// Buffer usage enum.
enum EBufferUsage
{
	MRHI_VERTEX_BUFFER,
	MRHI_INDEX_BUFFER
};

// Input topology mode.
enum ETopologyMode
{
	MRHI_POINT_LIST,
	MRHI_LINE_LIST,
	MRHI_LINE_STRIP,
	MRHI_TRIANGLE_LIST,
	MRHI_TRIANGLE_STRIP,
	MRHI_TRIANGLE_FAN
};
// Line drawing mode.
enum EPolygonMode
{
	MRHI_FILL,
	MRHI_LINE,
	MRHI_POINT
};
// Culling mode.
enum ECullMode
{
	MRHI_CULL_NONE,
	MRHI_CULL_FRONT_FACE,
	MRHI_CULL_BACK_FACE,
	MRHI_CULL_FRONT_AND_BACK
};
// Culling face.
enum EFrontFace
{
	MRHI_COUNTER_CLOCKWISE,
	MRHI_CLOCKWISE
};

// Available rendering APIs.
enum ERenderingAPI
{
	MRHI_VULKAN,
	MRHI_OPENGL,
	MRHI_DIRECTX12
};

// Data types supported for the shader (input location for vertex, or push constants).
enum EShaderDataType
{
	MRHI_NONE,
	MRHI_BOOL,
	MRHI_INT,
	MRHI_UINT,
	MRHI_FLOAT,
	MRHI_DOUBLE,
	MRHI_VEC2,
	MRHI_VEC3,
	MRHI_VEC4,
	MRHI_MAT2,
	MRHI_MAT3,
	MRHI_MAT4
};

// Shader stage enum.
enum EShaderStage
{
	MRHI_ALL,
	MRHI_VERTEX_SHADER,
	MRHI_FRAGMENT_SHADER
};

// Format of a texture/image
enum class ETextureFormat
{
	MRHI_RGBA8,
	MRHI_BGRA8,
	MRHI_DEPTH32
};

// Usage of a texture.
enum class ETextureUsage : uint32_t
{
	MRHI_NONE = 0,
	MRHI_SAMPLED = 1 << 0,
	MRHI_COLOR_ATTACHMENT = 1 << 1,
	MRHI_DEPTH_ATTACHMENT = 1 << 2
};