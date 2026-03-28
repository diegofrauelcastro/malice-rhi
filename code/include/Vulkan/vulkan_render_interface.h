#pragma once

#include "Interface/irender_interface.h"

class VulkanRenderInterface : public IRenderInterface
{
public:
	// Constructor
	VulkanRenderInterface();
	// Class destructor
	~VulkanRenderInterface() override;

	// Get Perspective Projection Matrix, in the form of a row-major vector of 16 floats (Matrix4x4).
	std::vector<float> GetPerspectiveProjectionMatrix(unsigned int _width, unsigned int _height, float _near, float _far, float _fovYDeg) override;

	// Get Orthographic Projection Matrix, in the form of a row-major vector of 16 floats (Matrix4x4).
	std::vector<float> GetOrthographicProjectionMatrix(unsigned int _width, unsigned int _height, float _near, float _far) override;

	// Instance
	IInstance* InstantiateInstance() override;
	// Device
	IDevice* InstantiateDevice() override;
	// Surface
	ISurface* InstantiateSurface() override;
	// Swap chain
	ISwapChain* InstantiateSwapChain() override;
	// Render pass
	IRenderPass* InstantiateRenderPass() override;
	// Framebuffers
	IFramebuffers* InstantiateFramebuffers() override;
	// Buffer
	IBuffer* InstantiateBuffer() override;
	// Command pool
	ICommandPool* InstantiateCommandPool() override;
	// Command buffers
	ICommandBuffers* InstantiateCommandBuffers() override;
	// Shader modules
	IShaderModules* InstantiateShaderModules() override;
	// Command buffers
	IPipeline* InstantiatePipeline() override;
	// Descriptor sets bundle
	IDescriptorSetsGroup* InstantiateDescriptorSetsBundle() override;
	// Uniform buffers
	IUniformBuffers* InstantiateUniformBuffers() override;
	// Texture
	ITexture* InstantiateTexture() override;
	// MaliceRHI to ImGui bridge class
	IMaliceToImGuiBridge* InstantiateMaliceToImGuiBridge() override;
};