#include "vulkan_render_interface.h"

#include "vulkan_instance.h"
#include "vulkan_device.h"
#include "vulkan_surface.h"
#include "vulkan_swapchain.h"
#include "vulkan_renderpass.h"
#include "vulkan_framebuffers.h"
#include "vulkan_shadermodules.h"
#include "vulkan_pipeline.h"
#include "vulkan_commandpool.h"
#include "vulkan_commandbuffers.h"
#include "vulkan_buffer.h"
//#include "vulkan_descriptorsetsbundle.h"
#include "vulkan_descriptorsetsgroup.h"
#include "vulkan_uniformbuffers.h"


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
