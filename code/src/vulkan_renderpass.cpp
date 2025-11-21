#include "vulkan_renderpass.h"

#include "vulkan_swapchain.h"
#include "vulkan_device.h"


void VulkanRenderPass::CreateRenderPass(VulkanDevice& _device, VulkanSwapChain& _swapChain)
{
	LOG_CLEAN("\n\n===== RENDER PASS CREATION =====\n")

	// Single color buffer, same format as the images in our swap chain.
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = _swapChain.GetImageFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	// We clear the color and depth data before rendering.
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	// We aren't really using stencil data so we ignore it.
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	// The layout of images has to be changed depending on what we plan to do with it.
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;	// "Images to be presented in the swap chain" -> Vulkan Tutorial.

	// Create an attachment that we'll link to a subpass.
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	// Create a single subpass (we won't need more for now).
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	// Define a subpass dependency to handle the transition of the image layout.
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	// Specify the pipeline stages and access masks.
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	// Specify that the destination is the color attachment output stage.
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	// Create info about the render pass, linking the color attachment and the subpass we created before.
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	// Finally, create the render pass and ensure it succeeded.
	VkResult result = vkCreateRenderPass(_device.GetLogicalDeviceVkHandle(), &renderPassInfo, nullptr, &renderPass);
	if (result != VK_SUCCESS)
		LOG_THROW("/!\\ Failed to create render pass!")
	else
		LOG_RHI("Render pass created successfully.")
}

void VulkanRenderPass::Create(IDevice* _device, ISwapChain* _swapChain)
{
	CreateRenderPass(_device->API_Vulkan(), _swapChain->API_Vulkan());
}

void VulkanRenderPass::Destroy(IDevice* _device)
{
	LOG_CLEAN("\n\n===== RENDER PASS DESTRUCTION =====\n")

	// Destroy the render pass.
	if (renderPass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(_device->API_Vulkan().GetLogicalDeviceVkHandle(), renderPass, nullptr);
		LOG_RHI("Render pass destroyed successfully.")
	}
	else
		LOG_RHI("Something went wrong trying to destroy a render pass...")
}
