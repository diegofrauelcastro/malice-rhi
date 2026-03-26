#include "Vulkan/vulkan_renderpass.h"

#include "Vulkan/vulkan_swapchain.h"
#include "Vulkan/vulkan_device.h"
#include "Vulkan/vulkan_texture.h"

static VkFormat ConvertFormat(ETextureFormat format)
{
	switch (format)
	{
	case ETextureFormat::RGBA8:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case ETextureFormat::BGRA8:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case ETextureFormat::DEPTH32:
		return VK_FORMAT_D32_SFLOAT;
	default:
		LOG_RHI_THROW("/!\\ Unsupported texture format!")
	}
}

void VulkanRenderPass::CreateRenderPass(VulkanDevice& _device)
{
	LOG_RHI_CLEAN("\n\n===== RENDER PASS CREATION =====\n")

	std::vector<VkAttachmentDescription> attachments;
	uint32_t colorCount = sc ? 1 : static_cast<uint32_t>(params.colorFormats.size());
	for (size_t i = 0; i < colorCount; i++)
	{
		VkAttachmentDescription attachment{};
		attachment.format = sc ? sc->GetImageFormat() : ConvertFormat(params.colorFormats[i]);
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		// We clear the color and depth data before rendering.
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		// We aren't really using stencil data so we ignore it.
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout = sc ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		attachments.push_back(attachment);
	}
	VkAttachmentReference depthRef{};
	if (params.hasDepth)
	{
		VkAttachmentDescription attachment{};
		attachment.format = ConvertFormat(params.depthFormat);
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		// We clear the color and depth data before rendering.
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments.push_back(attachment);

		depthRef.attachment = (uint32_t)(attachments.size() - 1);
		depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	// Create attachment references for all color attachments.
	std::vector<VkAttachmentReference> colorAttachmentRefs;
	for (size_t i = 0; i < colorCount; i++)
	{
		VkAttachmentReference ref{};
		ref.attachment = i;
		ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachmentRefs.push_back(ref);
	}

	// Create a single subpass (we won't need more for now).
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = (uint32_t)colorAttachmentRefs.size();
	subpass.pColorAttachments = colorAttachmentRefs.data();
	if (params.hasDepth)
		subpass.pDepthStencilAttachment = &depthRef;
	else
		subpass.pDepthStencilAttachment = nullptr;

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

	if (params.hasDepth)
	{
		dependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

	// Create info about the render pass, linking the color attachment and the subpass we created before.
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = (uint32_t)attachments.size();
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	// Finally, create the render pass and ensure it succeeded.
	VkResult result = vkCreateRenderPass(_device.GetLogicalDeviceVkHandle(), &renderPassInfo, nullptr, &renderPass);
	if (result != VK_SUCCESS)
		LOG_RHI_THROW("/!\\ Failed to create render pass!")
	else
		LOG_RHI("Render pass created successfully.")
}

void VulkanRenderPass::Create(IDevice* _device, ISwapChain* _swapChain, bool _hasDepth)
{
	sc = &_swapChain->API_Vulkan();
	params = {};
	params.hasDepth = _hasDepth;

	CreateRenderPass(_device->API_Vulkan());
}

void VulkanRenderPass::Create(IDevice* _device, const RenderPassParams& _params)
{
	params = {};
	params.colorFormats = _params.colorFormats;
	params.depthFormat = _params.depthFormat;
	params.hasDepth = _params.hasDepth;

	CreateRenderPass(_device->API_Vulkan());
}

void VulkanRenderPass::Destroy(IDevice* _device)
{
	LOG_RHI_CLEAN("\n\n===== RENDER PASS DESTRUCTION =====\n")

	params = {};
	sc = nullptr;

	// Destroy the render pass.
	if (renderPass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(_device->API_Vulkan().GetLogicalDeviceVkHandle(), renderPass, nullptr);
		LOG_RHI("Render pass destroyed successfully.")
	}
	else
		LOG_RHI("Something went wrong trying to destroy a render pass...")
}
