#pragma once

#include "irender_interface.h"

class VulkanRenderInterface : public IRenderInterface
{
public:
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
};