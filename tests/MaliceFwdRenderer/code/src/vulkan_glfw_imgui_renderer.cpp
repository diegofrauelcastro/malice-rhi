#include "vulkan_glfw_imgui_renderer.h"

#define IMGUI_IMPL_VULKAN_USE_VOLK
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "Vulkan/vulkan_imgui_support.h"
#include "Vulkan/vulkan_instance.h"
#include "Vulkan/vulkan_device.h"
#include "Vulkan/vulkan_swapchain.h"
#include "Vulkan/vulkan_commandbuffers.h"
#include "Vulkan/vulkan_renderpass.h"
#include "Vulkan/vulkan_framebuffers.h"

// Offscreen render target (color-only) to show inside ImGui
struct Offscreen
{
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkExtent2D extent{ 0,0 };
    VkDescriptorSet imguiSet = VK_NULL_HANDLE; // Returned by ImGui_ImplVulkan_AddTexture
};

// Helper to check VkResult
static void CheckVk(VkResult err, const char* msg)
{
    if (err != VK_SUCCESS)
    {
        std::fprintf(stderr, "Vulkan error %d: %s\n", err, msg);
        std::fflush(stderr);
        throw std::runtime_error("Vulkan error");
    }
}

void Vulkan_GLFW_ImGuiRenderer::Create(IMaliceToImGuiBridge* _bridge, GLFWwindow* _window)
{
    bridge = _bridge;
    window = _window;
    VulkanMaliceToImGuiBridge& vulkanBridge = _bridge->API_Vulkan();
    
    VulkanSwapChain sc = vulkanBridge.GetLinkedSwapChain()->API_Vulkan();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.ApiVersion = VK_API_VERSION_1_2;
    init_info.Instance = vulkanBridge.GetLinkedInstance()->API_Vulkan().GetVkHandle();
    init_info.PhysicalDevice = vulkanBridge.GetLinkedDevice()->API_Vulkan().GetPhysicalDeviceVkHandle();
    init_info.Device = vulkanBridge.GetLinkedDevice()->API_Vulkan().GetLogicalDeviceVkHandle();
    init_info.QueueFamily = vulkanBridge.GetLinkedDevice()->API_Vulkan().GetQueueFamiliesIndices().graphicsFamily;
    init_info.Queue = vulkanBridge.GetLinkedDevice()->API_Vulkan().GetGraphicsQueueVkHandle();
    init_info.DescriptorPool = vulkanBridge.GetDescriptorPool();
    init_info.MinImageCount = sc.GetMinImageCount();
    init_info.ImageCount = sc.GetImageCount();
    init_info.PipelineInfoMain.RenderPass = vulkanBridge.GetLinkedRenderPass()->API_Vulkan().GetVkHandle(); // main viewport render pass
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&init_info);

    // Fonts: Recent backend versions auto-upload on first NewFrame(), so no explicit upload needed here.

    // Create offscreen target and register to ImGui
    off = new Offscreen{};
    CreateOffscreen(vulkanBridge.GetLinkedDevice(), vulkanBridge.GetLinkedSwapChain(), *off);
    off->imguiSet = ImGui_ImplVulkan_AddTexture(off->sampler, off->view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void Vulkan_GLFW_ImGuiRenderer::Destroy()
{
    DestroyOffscreen(bridge->GetLinkedDevice(), *off);
    // Delete offscreen if it exists.
    if (off)
    {
        delete off;
        off = nullptr;
    }

    // Shutdown ImGui.
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Vulkan_GLFW_ImGuiRenderer::RecordOffscreen(ICommandBuffers* _cmd)
{
    VkCommandBuffer cmd = _cmd->API_Vulkan().GetCurrentCommandBuffer();

    // Transition to COLOR_ATTACHMENT_OPTIMAL
    VkImageMemoryBarrier toColor{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    toColor.srcAccessMask = 0;
    toColor.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    toColor.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    toColor.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    toColor.image = off->image;
    toColor.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toColor.subresourceRange.levelCount = 1;
    toColor.subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0, 0, nullptr, 0, nullptr, 1, &toColor);

    // Begin pass
    VkClearValue clear; clear.color = { { 0.10f, 0.20f, 0.60f, 1.0f } }; // Nice blue
    VkRenderPassBeginInfo rpbi{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    rpbi.renderPass = off->renderPass;
    rpbi.framebuffer = off->framebuffer;
    rpbi.renderArea.extent = off->extent;
    rpbi.clearValueCount = 1; rpbi.pClearValues = &clear;
    vkCmdBeginRenderPass(cmd, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    // No draws: just clear color attachment
    vkCmdEndRenderPass(cmd);

    // Transition to SHADER_READ_ONLY for ImGui sampling
    VkImageMemoryBarrier toSample{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    toSample.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    toSample.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    toSample.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    toSample.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    toSample.image = off->image;
    toSample.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toSample.subresourceRange.levelCount = 1;
    toSample.subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &toSample);
}

void Vulkan_GLFW_ImGuiRenderer::RecordNewFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Offscreen View");
    ImVec2 sz((float)off->extent.width, (float)off->extent.height);
    ImGui::Text("Vulkan offscreen render (cleared color) below:");
    ImTextureRef tex_ref((ImTextureID)(intptr_t)off->imguiSet);
    ImGui::Image(tex_ref, sz);
    ImGui::End();
}

void Vulkan_GLFW_ImGuiRenderer::ShowDemoWindow()
{
    ImGui::ShowDemoWindow();
}

void Vulkan_GLFW_ImGuiRenderer::RenderFrame(ICommandBuffers* _cmd)
{
    VkCommandBuffer cmd = _cmd->API_Vulkan().GetCurrentCommandBuffer();
    
    VkClearValue clearMain; clearMain.color = { { 0.02f, 0.02f, 0.02f, 1.0f } };
    VkRenderPassBeginInfo rpbi{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    rpbi.renderPass = bridge->GetLinkedRenderPass()->API_Vulkan().GetVkHandle();
    rpbi.framebuffer = bridge->GetLinkedFramebuffers()->API_Vulkan().GetVkHandles()[_cmd->GetCurrentFrame()];
    rpbi.renderArea.extent = bridge->GetLinkedSwapChain()->API_Vulkan().GetImageExtent();
    rpbi.clearValueCount = 1; rpbi.pClearValues = &clearMain;
    vkCmdBeginRenderPass(cmd, &rpbi, VK_SUBPASS_CONTENTS_INLINE);

    // Render ImGui draw data into the current command buffer
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    vkCmdEndRenderPass(cmd);
}

void Vulkan_GLFW_ImGuiRenderer::CreateOffscreen(IDevice* _device, ISwapChain* _swapChain, Offscreen& _out)
{
    VkDevice device = _device->API_Vulkan().GetLogicalDeviceVkHandle();
    VkExtent2D extent = _swapChain->API_Vulkan().GetImageExtent();
    VkFormat colorFormat = _swapChain->API_Vulkan().GetImageFormat();
    _out.extent = extent;

    // Image
    VkImageCreateInfo ici{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.format = colorFormat; // match swapchain for simplicity
    ici.extent = { extent.width, extent.height, 1 };
    ici.mipLevels = 1; ici.arrayLayers = 1;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    CheckVk(vkCreateImage(device, &ici, nullptr, &_out.image), "vkCreateImage(offscreen)");

    VkMemoryRequirements mr{};
    vkGetImageMemoryRequirements(device, _out.image, &mr);
    VkMemoryAllocateInfo mai{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    mai.allocationSize = mr.size;
    mai.memoryTypeIndex = FindMemoryType(_device, mr.memoryTypeBits);
    CheckVk(vkAllocateMemory(device, &mai, nullptr, &_out.memory), "vkAllocateMemory(offscreen)");
    CheckVk(vkBindImageMemory(device, _out.image, _out.memory, 0), "vkBindImageMemory(offscreen)");

    // View
    VkImageViewCreateInfo iv{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    iv.image = _out.image;
    iv.viewType = VK_IMAGE_VIEW_TYPE_2D;
    iv.format = colorFormat;
    iv.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    iv.subresourceRange.levelCount = 1;
    iv.subresourceRange.layerCount = 1;
    CheckVk(vkCreateImageView(device, &iv, nullptr, &_out.view), "vkCreateImageView(offscreen)");

    // Sampler
    VkSamplerCreateInfo sci{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    sci.magFilter = VK_FILTER_LINEAR; sci.minFilter = VK_FILTER_LINEAR;
    sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sci.addressModeU = sci.addressModeV = sci.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    CheckVk(vkCreateSampler(device, &sci, nullptr, &_out.sampler), "vkCreateSampler(offscreen)");

    // Render pass (color-only offscreen)
    VkAttachmentDescription color{};
    color.format = colorFormat;
    color.samples = VK_SAMPLE_COUNT_1_BIT;
    color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // will transition to READ_ONLY after pass

    VkAttachmentReference colorRef{}; colorRef.attachment = 0; colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{}; subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; subpass.colorAttachmentCount = 1; subpass.pColorAttachments = &colorRef;

    VkRenderPassCreateInfo rpci{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    rpci.attachmentCount = 1; rpci.pAttachments = &color; rpci.subpassCount = 1; rpci.pSubpasses = &subpass;
    CheckVk(vkCreateRenderPass(device, &rpci, nullptr, &_out.renderPass), "vkCreateRenderPass(offscreen)");

    // Framebuffer
    VkImageView attachments[] = { _out.view };
    VkFramebufferCreateInfo fci{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    fci.renderPass = _out.renderPass; fci.attachmentCount = 1; fci.pAttachments = attachments; fci.width = extent.width; fci.height = extent.height; fci.layers = 1;
    CheckVk(vkCreateFramebuffer(device, &fci, nullptr, &_out.framebuffer), "vkCreateFramebuffer(offscreen)");
}

void Vulkan_GLFW_ImGuiRenderer::DestroyOffscreen(IDevice* _device, Offscreen& _o)
{
    VkDevice device = _device->API_Vulkan().GetLogicalDeviceVkHandle();

    if (_o.imguiSet != VK_NULL_HANDLE)
    {
        ImGui_ImplVulkan_RemoveTexture(_o.imguiSet);
        _o.imguiSet = VK_NULL_HANDLE;
    }
    if (_o.framebuffer) vkDestroyFramebuffer(device, _o.framebuffer, nullptr);
    if (_o.renderPass) vkDestroyRenderPass(device, _o.renderPass, nullptr);
    if (_o.sampler) vkDestroySampler(device, _o.sampler, nullptr);
    if (_o.view) vkDestroyImageView(device, _o.view, nullptr);
    if (_o.image) vkDestroyImage(device, _o.image, nullptr);
    if (_o.memory) vkFreeMemory(device, _o.memory, nullptr);
    _o = {};
}

uint32_t Vulkan_GLFW_ImGuiRenderer::FindMemoryType(IDevice* _device, uint32_t _typeBits)
{
    VkPhysicalDeviceMemoryProperties mp{};
    vkGetPhysicalDeviceMemoryProperties(_device->API_Vulkan().GetPhysicalDeviceVkHandle(), &mp);
    for (uint32_t i = 0; i < mp.memoryTypeCount; ++i)
    {
        if ((_typeBits & (1u << i)) && (mp.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            return i;
    }
    throw std::runtime_error("No suitable memory type found");
}