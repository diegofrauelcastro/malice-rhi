#pragma once

#include <iostream>


// Forward declarations
class VulkanBuffer;
class IDevice;
class ICommandPool;

// Buffer usage enum
enum EBufferUsage
{
	VERTEX_BUFFER,
	INDEX_BUFFER
};

// General buffer interface
class IBuffer
{
protected:
	/// Class properties ///

	// The type of the buffer.
	EBufferUsage bufferType;
	// Size of the buffer.
	uint64_t size;

public:
	// Class destructor
	virtual ~IBuffer() = default;


	/// Lifetime methods ///

	virtual void Create(IDevice* _device, ICommandPool* _commandPool, EBufferUsage _usage, uint64_t _size, void* _data) = 0;
	virtual void Destroy(IDevice* _device) = 0;


	/// Retrieving the backend ///

	virtual VulkanBuffer& API_Vulkan() { throw std::runtime_error("Bad API call : object is not a VulkanBuffer."); }


	/// Class specific methods ///

	// Returns the type of the buffer.
	EBufferUsage GetBufferType() const { return bufferType; }
	// Returns the size of the buffer.
	uint64_t GetSize() const { return size; }
};