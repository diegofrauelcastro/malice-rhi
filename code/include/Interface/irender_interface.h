#pragma once

#include <vector>

// Forward declarations.
class IInstance;
class IDevice;
class ISurface;
class ISwapChain;
class IRenderPass;
class IFramebuffers;
class IBuffer;
class ICommandPool;
class ICommandBuffers;
class IShaderModules;
class IPipeline;
class IUniformBuffers;
class IDescriptorSetsGroup;

// Available rendering APIs.
enum ERenderingAPI
{
	VULKAN,
	OPENGL,
	DIRECTX12
};

// Factory interface.
class IRenderInterface
{
protected:
	ERenderingAPI currentAPI = VULKAN;

public:
	// Class destructor
	virtual ~IRenderInterface() = default;

	// Get current rendering API.
	ERenderingAPI GetCurrentAPI() const { return currentAPI; }

	// Get Perspective Projection Matrix, in the form of a row-major vector of 16 floats (Matrix4x4).
	virtual std::vector<float> GetPerspectiveProjectionMatrix(unsigned int _width, unsigned int _height, float _near, float _far, float _fovYDeg) = 0;

	// Get Orthogonal Projection Matrix, in the form of a row-major vector of 16 floats (Matrix4x4).
	virtual std::vector<float> GetOrthogonalProjectionMatrix() = 0;

	// Instance
	virtual IInstance* InstantiateInstance() = 0;
	virtual void DeleteInstance(IInstance* _instance) { delete _instance; }

	// Device
	virtual IDevice* InstantiateDevice() = 0;
	virtual void DeleteDevice(IDevice* _device) { delete _device; }

	// Surface
	virtual ISurface* InstantiateSurface() = 0;
	virtual void DeleteSurface(ISurface* _surface) { delete _surface; }

	// Swap chain
	virtual ISwapChain* InstantiateSwapChain() = 0;
	virtual void DeleteSwapChain(ISwapChain* _swapChain) { delete _swapChain; }

	// Render pass
	virtual IRenderPass* InstantiateRenderPass() = 0;
	virtual void DeleteRenderPass(IRenderPass* _renderPass) { delete _renderPass; }

	// Framebuffers
	virtual IFramebuffers* InstantiateFramebuffers() = 0;
	virtual void DeleteFramebuffers(IFramebuffers* _frameBuffers) { delete _frameBuffers; }

	// Buffer
	virtual IBuffer* InstantiateBuffer() = 0;
	virtual void DeleteBuffer(IBuffer* _buffer) { delete _buffer; }

	// Command pool
	virtual ICommandPool* InstantiateCommandPool() = 0;
	virtual void DeleteCommandPool(ICommandPool* _commandPool) { delete _commandPool; }
	
	// Command buffers
	virtual ICommandBuffers* InstantiateCommandBuffers() = 0;
	virtual void DeleteCommandBuffers(ICommandBuffers* _commandBuffers) { delete _commandBuffers; }
	
	// Shader modules
	virtual IShaderModules* InstantiateShaderModules() = 0;
	virtual void DeleteShaderModules(IShaderModules* _shaderModules) { delete _shaderModules; }
	
	// Command buffers
	virtual IPipeline* InstantiatePipeline() = 0;
	virtual void DeletePipeline(IPipeline* _pipeline) { delete _pipeline; }

	// Descriptor sets bundle
	virtual IDescriptorSetsGroup* InstantiateDescriptorSetsBundle() = 0;
	virtual void DeleteDescriptorSetsBundle(IDescriptorSetsGroup* _descriptorSets) { delete _descriptorSets; }

	// Uniform buffer
	virtual IUniformBuffers* InstantiateUniformBuffers() = 0;
	virtual void DeleteUniformBuffers(IUniformBuffers* _uniformBuffer) { delete _uniformBuffer; }
};