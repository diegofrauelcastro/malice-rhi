#include "Vulkan/vulkan_framebuffers.h"

#include "Vulkan/vulkan_device.h"
#include "Vulkan/vulkan_renderpass.h"
#include "Vulkan/vulkan_swapchain.h"
#include "Vulkan/vulkan_texture.h"


void VulkanFramebuffers::CreateFramebuffers(VulkanDevice& _device, VkRenderPass _renderPass)
{
	LOG_RHI_CLEAN("\n\n===== FRAMEBUFFERS CREATION =====\n")

	renderPass = _renderPass;

	// Resize to hold all framebuffers (one for each ImageView in our swap chain images), OR our color attachments.
	if (sc) framebuffers.resize(sc->GetImageViewsVkHandle().size());
	else framebuffers.resize(1);

	LOG_RHI("Creating a framebuffer per image in the swapchain...")

	// Iterate through every image view and create a framebuffer for each.
	size_t imageCount = 0;
	if (sc) imageCount = sc->GetImageViewsVkHandle().size();
	else imageCount = params.colorAttachments.size();

	for (size_t i = 0; i < imageCount; i++)
	{
		// All attachments.
		std::vector<VkImageView> attachments;
		if (sc) attachments.push_back(sc->GetImageViewsVkHandle()[i]);
		else attachments.push_back(params.colorAttachments[i]->API_Vulkan().GetImageView());

		// Enable depth if there was a depth texture given.
		if (params.depthAttachment)
			attachments.push_back(params.depthAttachment->API_Vulkan().GetImageView());

		// Info for this framebuffer.
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = _renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = params.width;
		framebufferInfo.height = params.height;
		framebufferInfo.layers = 1;

		// Create the framebuffer.
		VkResult result = vkCreateFramebuffer(_device.GetLogicalDeviceVkHandle(), &framebufferInfo, nullptr, &framebuffers[i]);
		if (result != VK_SUCCESS)
			LOG_RHI_THROW("/!\\ Failed to create framebuffer!")
		else
			LOG_RHI("Framebuffer %d created successfully.", (int)i)
	}
}

void VulkanFramebuffers::Create(IDevice* _device, IRenderPass* _renderPass, ISwapChain* _swapChain, ITexture* _depthTex)
{
	// Create parameters from SwapChain.
	params = {};
	params.depthAttachment = _depthTex;
	params.width = _swapChain->API_Vulkan().GetImageExtent().width;
	params.height = _swapChain->API_Vulkan().GetImageExtent().height;
	sc = &_swapChain->API_Vulkan();

	CreateFramebuffers(_device->API_Vulkan(), _renderPass->API_Vulkan().GetVkHandle());

	// Bind resize callback method to the swapchain.
	_swapChain->BindResizeCallback([&](IDevice* _d, ISwapChain* _sc) { return Recreate(_d, _sc); });
}

void VulkanFramebuffers::Create(IDevice* _device, IRenderPass* _renderPass, const FramebufferParams& _params)
{
	// Store the given params.
	params = {};
	params.colorAttachments = _params.colorAttachments;
	params.depthAttachment = _params.depthAttachment;
	params.height = _params.height;
	params.width = _params.width;

	CreateFramebuffers(_device->API_Vulkan(), _renderPass->API_Vulkan().GetVkHandle());
}

void VulkanFramebuffers::Destroy(IDevice* _device)
{
	LOG_RHI_CLEAN("\n\n===== FRAMEBUFFERS DESTRUCTION =====\n")

	// Destroy framebuffers.
	for (size_t i = 0; i < framebuffers.size(); i++)
	{
		vkDestroyFramebuffer(_device->API_Vulkan().GetLogicalDeviceVkHandle(), framebuffers[i], nullptr);
		LOG_RHI("Framebuffer %d destroyed successfully.", (int)i)
	}

	framebuffers.clear();
	framebuffers.shrink_to_fit();
	sc = nullptr;
}

void VulkanFramebuffers::Recreate(IDevice* _device, ISwapChain* _swapChain)
{
	LOG_RHI("Recreating framebuffers...")
	Destroy(_device);
	sc = &_swapChain->API_Vulkan();
	params.width = sc->GetImageExtent().width;
	params.height = sc->GetImageExtent().height;
	
	CreateFramebuffers(_device->API_Vulkan(), renderPass);
}
