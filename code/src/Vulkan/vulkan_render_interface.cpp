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
	LOG_RHI_CLEAN("\n\n===== VULKAN RENDER INTERFACE =====\n")
	LOG_RHI("Initializing Volk for Vulkan RHI...")
	currentAPI = VULKAN;
	volkInitialize();
	LOG_RHI("Vulkan RHI initialized.\n")
}

// Class destructor
VulkanRenderInterface::~VulkanRenderInterface()
{
	LOG_RHI_CLEAN("\n\n===== TERMINATION =====\n")
	LOG_RHI("Finalizing Volk for Vulkan RHI...")
	volkFinalize();
	LOG_RHI("Vulkan RHI cleaned up.\n")
	// Log destruction.
	MaliceRHI::Debug::Log::GetInstance()->Destroy();
}

// Get Perspective Projection Matrix, in the form of a row-major vector of 16 floats (Matrix4x4).
std::vector<float> VulkanRenderInterface::GetPerspectiveProjectionMatrix(unsigned int _width, unsigned int _height, float _near, float _far, float _fovYDeg)
{
	float q = 1.0f / tan(0.5f * _fovYDeg * 0.01745329251f);
	float a = q / ((float)_width / _height);

	float b = (_near + _far) / (_near - _far);
	float c = (2.0f * _near * _far) / (_near - _far);

	std::vector<float> result(16);
	result[0] = a;
	result[5] = q;
	result[10] = b;
	result[11] = c;
	result[14] = -1.0f;

	return result;
}

// Get Orthographic Projection Matrix, in the form of a row-major vector of 16 floats (Matrix4x4).
std::vector<float> VulkanRenderInterface::GetOrthographicProjectionMatrix(unsigned int _width, unsigned int _height, float _near, float _far)
{
	float ratio = (float)_width / _height;
	float inverseFarMinusMin = 1.f / (_far - _near);

	std::vector<float> result(16);
	result[0] = 1.f;
	result[5] = ratio;
	result[10] = -inverseFarMinusMin;
	result[11] = -_near * inverseFarMinusMin;
	result[15] = 1.f;

	return result;
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
