#pragma once

#include <cstdint>

// Forward declarations
class IDevice;
class ICommandPool;
class VulkanTexture;

enum class ETextureFormat
{
	RGBA8,
	BGRA8,
	DEPTH32
};

enum class ETextureUsage : uint32_t
{
	NONE = 0,
	SAMPLED = 1 << 0,
	COLOR_ATTACHMENT = 1 << 1,
	DEPTH_ATTACHMENT = 1 << 2
};

inline ETextureUsage operator|(ETextureUsage a, ETextureUsage b)
{
	return static_cast<ETextureUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline bool HasUsage(ETextureUsage value, ETextureUsage flag)
{
	return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
}

class ITexture
{
protected:
	/// Texture properties ///

	uint32_t width = 0;
	uint32_t height = 0;
	ETextureFormat format;
	ETextureUsage usage;

public:
	/// Lifetime methods ///

	virtual void Create(IDevice* _device, ICommandPool* _commandPool, uint32_t _width, uint32_t _height, ETextureFormat _format, ETextureUsage _usage, const void* _data = nullptr) = 0;
	virtual void Destroy(IDevice* _device) = 0;
	virtual ~ITexture() = default;


	/// Retrieving the backend ///

	virtual VulkanTexture& API_Vulkan() { throw std::runtime_error("Bad API call : object is not a VulkanTexture."); }


	/// Class specific methods ///

	uint32_t GetWidth() const { return width; }
	uint32_t GetHeight() const { return height; }
	ETextureFormat GetFormat() const { return format; }
	ETextureUsage GetUsage() const { return usage; }
};