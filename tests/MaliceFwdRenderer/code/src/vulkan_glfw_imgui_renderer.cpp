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
#include "Vulkan/vulkan_texture.h"

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

    // Create offscreen target and register to ImGui
    off = new Offscreen();
    *off = bridge->GetOffscreenParams();
    // ImGui Render Texture
    VulkanTexture& vkTex = off->colorTex->API_Vulkan();
    imguiSet = ImGui_ImplVulkan_AddTexture(vkTex.GetSampler(), vkTex.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void Vulkan_GLFW_ImGuiRenderer::Destroy()
{
    // Shutdown ImGui.
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Destroy ImGui descriptor set.
    if (imguiSet)
    {
        ImGui_ImplVulkan_RemoveTexture(imguiSet);
        imguiSet = VK_NULL_HANDLE;
    }
    // Delete offscreen pointer.
    if (off)
    {
        delete off;
        off = nullptr;
    }
}

void Vulkan_GLFW_ImGuiRenderer::RecordNewFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Vulkan_GLFW_ImGuiRenderer::ShowOffscreenRenderInWindow()
{
    ImGui::Begin("Offscreen View");
    ImVec2 sz((float)off->framebuffers->GetWidth(), (float)off->framebuffers->GetHeight());
    ImGui::Text("Vulkan offscreen render (cleared color) below:");
    ImTextureRef tex_ref((ImTextureID)(intptr_t)imguiSet);
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
    
    ImGui::Render();

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
