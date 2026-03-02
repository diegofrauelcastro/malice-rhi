#include "Vulkan/vulkan_render_interface.h"

#include "Vulkan/vulkan_instance.h"
#include "Vulkan/vulkan_device.h"
#include "Vulkan/vulkan_surface.h"
#include "Vulkan/vulkan_swapchain.h"
#include "Vulkan/vulkan_renderpass.h"
#include "Vulkan/vulkan_framebuffers.h"
#include "Vulkan/vulkan_shadermodules.h"
#include "Vulkan/vulkan_pipeline.h"
#include "Vulkan/vulkan_commandpool.h"
#include "Vulkan/vulkan_commandbuffers.h"
#include "Vulkan/vulkan_buffer.h"
#include "Vulkan/vulkan_descriptorsetsgroup.h"
#include "Vulkan/vulkan_uniformbuffers.h"


// Class constructor
VulkanRenderInterface::VulkanRenderInterface()
{
	LOG_CLEAN("\n\n===== VULKAN RENDER INTERFACE =====\n")
	LOG_RHI("Initializing Volk for Vulkan RHI...")
	currentAPI = VULKAN;
	volkInitialize();
	LOG_RHI("Vulkan RHI initialized.\n")
}

// Class destructor
VulkanRenderInterface::~VulkanRenderInterface()
{
	LOG_CLEAN("\n\n===== TERMINATION =====\n")
	LOG_RHI("Finalizing Volk for Vulkan RHI...")
	volkFinalize();
	LOG_RHI("Vulkan RHI cleaned up.\n")
	// Log destruction.
	MaliceRHI::Debug::Log::GetInstance()->Destroy();
}

// Instance
IInstance* VulkanRenderInterface::InstantiateInstance()
{
	return new VulkanInstance();
}

// Device
IDevice* VulkanRenderInterface::InstantiateDevice()
{
	return new VulkanDevice();
}

// Surface
ISurface* VulkanRenderInterface::InstantiateSurface()
{
	return new VulkanSurface();
}

// Swap chain
ISwapChain* VulkanRenderInterface::InstantiateSwapChain()
{
	return new VulkanSwapChain();
}

// Render pass
IRenderPass* VulkanRenderInterface::InstantiateRenderPass()
{
	return new VulkanRenderPass();
}

IFramebuffers* VulkanRenderInterface::InstantiateFramebuffers()
{
	return new VulkanFramebuffers();
}

// Buffer
IBuffer* VulkanRenderInterface::InstantiateBuffer()
{
	return new VulkanBuffer();
}

// Command pool
ICommandPool* VulkanRenderInterface::InstantiateCommandPool()
{
	return new VulkanCommandPool();
}

// Command buffers
ICommandBuffers* VulkanRenderInterface::InstantiateCommandBuffers()
{
	return new VulkanCommandBuffers();
}

// Shader modules
IShaderModules* VulkanRenderInterface::InstantiateShaderModules()
{
	return new VulkanShaderModules();
}

// Command buffers
IPipeline* VulkanRenderInterface::InstantiatePipeline()
{
	return new VulkanPipeline();
}

// Descriptor sets bundle
IDescriptorSetsGroup* VulkanRenderInterface::InstantiateDescriptorSetsBundle()
{
	return new VulkanDescriptorSetsGroup();
}

// Uniform buffer
IUniformBuffers* VulkanRenderInterface::InstantiateUniformBuffers()
{
	return new VulkanUniformBuffers();
}
