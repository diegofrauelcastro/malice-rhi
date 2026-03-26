#include <cstdio>
#include <cstdlib>
#include <vector>
#include <array>
#include <optional>
#include <stdexcept>
#include <iostream>
#include <cstring>

// Dear ImGui
#define IMGUI_IMPL_VULKAN_USE_VOLK
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

// GLFW for window/surface and Vulkan surface creation helper
#include <GLFW/glfw3.h>

#include "application.h"

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

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphics;
    std::optional<uint32_t> present;
    bool complete() const { return graphics.has_value() && present.has_value(); }
};

static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;

    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, nullptr);
    std::vector<VkQueueFamilyProperties> props(count);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, props.data());

    for (uint32_t i = 0; i < count; ++i)
    {
        if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphics = i;

        VkBool32 supportsPresent = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &supportsPresent);
        if (supportsPresent)
            indices.present = i;

        if (indices.complete()) break;
    }
    return indices;
}

struct Swapchain
{
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkExtent2D extent{ 0,0 };
    std::vector<VkImage> images;
    std::vector<VkImageView> views;
};

static void CreateSwapchain(VkPhysicalDevice gpu, VkDevice device, VkSurfaceKHR surface, const QueueFamilyIndices& q, GLFWwindow* window, Swapchain& out)
{
    // Query surface capabilities
    VkSurfaceCapabilitiesKHR caps{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &caps);

    uint32_t formatCount = 0; vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, formats.data());

    uint32_t presentModeCount = 0; vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, presentModes.data());

    // Choose format
    VkSurfaceFormatKHR chosenFormat = formats[0];
    for (auto& f : formats)
    {
        if (f.format == VK_FORMAT_B8G8R8A8_UNORM || f.format == VK_FORMAT_B8G8R8A8_SRGB)
        {
            chosenFormat = f; break;
        }
    }

    // Choose present mode (mailbox if available, else fifo)
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (auto pm : presentModes) if (pm == VK_PRESENT_MODE_MAILBOX_KHR) { presentMode = pm; break; }

    // Extent
    int w, h; glfwGetFramebufferSize(window, &w, &h);
    VkExtent2D extent{ (uint32_t)std::max(1, w), (uint32_t)std::max(1, h) };
    if (caps.currentExtent.width != UINT32_MAX)
        extent = caps.currentExtent;

    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) imageCount = caps.maxImageCount;

    uint32_t queueFamilyIndices[2] = { q.graphics.value(), q.present.value() };

    VkSwapchainCreateInfoKHR ci{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    ci.surface = surface;
    ci.minImageCount = imageCount;
    ci.imageFormat = chosenFormat.format;
    ci.imageColorSpace = chosenFormat.colorSpace;
    ci.imageExtent = extent;
    ci.imageArrayLayers = 1;
    ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (q.graphics != q.present)
    {
        ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        ci.queueFamilyIndexCount = 2;
        ci.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    ci.preTransform = caps.currentTransform;
    ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    ci.presentMode = presentMode;
    ci.clipped = VK_TRUE;

    CheckVk(vkCreateSwapchainKHR(device, &ci, nullptr, &out.swapchain), "vkCreateSwapchainKHR");

    uint32_t count = 0; vkGetSwapchainImagesKHR(device, out.swapchain, &count, nullptr);
    out.images.resize(count);
    vkGetSwapchainImagesKHR(device, out.swapchain, &count, out.images.data());

    out.format = chosenFormat.format;
    out.extent = extent;

    out.views.resize(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        VkImageViewCreateInfo iv{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        iv.image = out.images[i];
        iv.viewType = VK_IMAGE_VIEW_TYPE_2D;
        iv.format = out.format;
        iv.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        iv.subresourceRange.levelCount = 1;
        iv.subresourceRange.layerCount = 1;
        CheckVk(vkCreateImageView(device, &iv, nullptr, &out.views[i]), "vkCreateImageView(swapchain)");
    }
}

static void DestroySwapchain(VkDevice device, Swapchain& sc)
{
    for (auto v : sc.views) if (v) vkDestroyImageView(device, v, nullptr);
    sc.views.clear();
    if (sc.swapchain) vkDestroySwapchainKHR(device, sc.swapchain, nullptr);
    sc.swapchain = VK_NULL_HANDLE;
}

// Create a simple render pass for the main presentation
static VkRenderPass CreateRenderPass(VkDevice device, VkFormat colorFormat)
{
    VkAttachmentDescription color{};
    color.format = colorFormat;
    color.samples = VK_SAMPLE_COUNT_1_BIT;
    color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorRef{};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;

    VkSubpassDependency dep{};
    dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    dep.dstSubpass = 0;
    dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.srcAccessMask = 0;
    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo ci{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    ci.attachmentCount = 1;
    ci.pAttachments = &color;
    ci.subpassCount = 1;
    ci.pSubpasses = &subpass;
    ci.dependencyCount = 1;
    ci.pDependencies = &dep;

    VkRenderPass rp{};
    CheckVk(vkCreateRenderPass(device, &ci, nullptr, &rp), "vkCreateRenderPass(main)");
    return rp;
}

struct FramebufferBundle
{
    std::vector<VkFramebuffer> framebuffers;
};

static void CreateFramebuffers(VkDevice device, const Swapchain& sc, VkRenderPass rp, FramebufferBundle& out)
{
    out.framebuffers.resize(sc.views.size());
    for (size_t i = 0; i < sc.views.size(); ++i)
    {
        VkImageView attachments[] = { sc.views[i] };
        VkFramebufferCreateInfo fci{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        fci.renderPass = rp;
        fci.attachmentCount = 1;
        fci.pAttachments = attachments;
        fci.width = sc.extent.width;
        fci.height = sc.extent.height;
        fci.layers = 1;
        CheckVk(vkCreateFramebuffer(device, &fci, nullptr, &out.framebuffers[i]), "vkCreateFramebuffer");
    }
}

static void DestroyFramebuffers(VkDevice device, FramebufferBundle& fb)
{
    for (auto f : fb.framebuffers) if (f) vkDestroyFramebuffer(device, f, nullptr);
    fb.framebuffers.clear();
}

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

static uint32_t FindMemoryType(VkPhysicalDevice gpu, uint32_t typeBits, VkMemoryPropertyFlags props)
{
    VkPhysicalDeviceMemoryProperties mp{};
    vkGetPhysicalDeviceMemoryProperties(gpu, &mp);
    for (uint32_t i = 0; i < mp.memoryTypeCount; ++i)
    {
        if ((typeBits & (1u << i)) && (mp.memoryTypes[i].propertyFlags & props) == props)
            return i;
    }
    throw std::runtime_error("No suitable memory type found");
}

static void CreateOffscreen(VkPhysicalDevice gpu, VkDevice device, VkFormat colorFormat, VkExtent2D extent, Offscreen& out)
{
    out.extent = extent;

    // Image
    VkImageCreateInfo ici{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.format = colorFormat; // match swapchain for simplicity
    ici.extent = { extent.width, extent.height, 1 };
    ici.mipLevels = 1; ici.arrayLayers = 1;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    CheckVk(vkCreateImage(device, &ici, nullptr, &out.image), "vkCreateImage(offscreen)");

    VkMemoryRequirements mr{};
    vkGetImageMemoryRequirements(device, out.image, &mr);
    VkMemoryAllocateInfo mai{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    mai.allocationSize = mr.size;
    mai.memoryTypeIndex = FindMemoryType(gpu, mr.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    CheckVk(vkAllocateMemory(device, &mai, nullptr, &out.memory), "vkAllocateMemory(offscreen)");
    CheckVk(vkBindImageMemory(device, out.image, out.memory, 0), "vkBindImageMemory(offscreen)");

    // View
    VkImageViewCreateInfo iv{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    iv.image = out.image;
    iv.viewType = VK_IMAGE_VIEW_TYPE_2D;
    iv.format = colorFormat;
    iv.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    iv.subresourceRange.levelCount = 1;
    iv.subresourceRange.layerCount = 1;
    CheckVk(vkCreateImageView(device, &iv, nullptr, &out.view), "vkCreateImageView(offscreen)");

    // Sampler
    VkSamplerCreateInfo sci{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    sci.magFilter = VK_FILTER_LINEAR; sci.minFilter = VK_FILTER_LINEAR;
    sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sci.addressModeU = sci.addressModeV = sci.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    CheckVk(vkCreateSampler(device, &sci, nullptr, &out.sampler), "vkCreateSampler(offscreen)");

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
    CheckVk(vkCreateRenderPass(device, &rpci, nullptr, &out.renderPass), "vkCreateRenderPass(offscreen)");

    // Framebuffer
    VkImageView attachments[] = { out.view };
    VkFramebufferCreateInfo fci{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    fci.renderPass = out.renderPass; fci.attachmentCount = 1; fci.pAttachments = attachments; fci.width = extent.width; fci.height = extent.height; fci.layers = 1;
    CheckVk(vkCreateFramebuffer(device, &fci, nullptr, &out.framebuffer), "vkCreateFramebuffer(offscreen)");
}

static void DestroyOffscreen(VkDevice device, Offscreen& o)
{
    if (o.imguiSet != VK_NULL_HANDLE)
    {
        ImGui_ImplVulkan_RemoveTexture(o.imguiSet);
        o.imguiSet = VK_NULL_HANDLE;
    }
    if (o.framebuffer) vkDestroyFramebuffer(device, o.framebuffer, nullptr);
    if (o.renderPass) vkDestroyRenderPass(device, o.renderPass, nullptr);
    if (o.sampler) vkDestroySampler(device, o.sampler, nullptr);
    if (o.view) vkDestroyImageView(device, o.view, nullptr);
    if (o.image) vkDestroyImage(device, o.image, nullptr);
    if (o.memory) vkFreeMemory(device, o.memory, nullptr);
    o = {};
}

int main()
{
    // Init GLFW
    if (!glfwInit())
    {
        std::fprintf(stderr, "Failed to init GLFW\n");
        return 1;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "ImGui Vulkan Test (offscreen in window)", nullptr, nullptr);
    if (!window) { std::fprintf(stderr, "Failed to create GLFW window\n"); return 1; }

    // Init Volk
    if (volkInitialize() != VK_SUCCESS)
    {
        std::fprintf(stderr, "Failed to initialize Volk\n");
        return 1;
    }

    // Create Vulkan instance
    std::vector<const char*> extensions;
    {
        uint32_t glfwExtCount = 0;
        const char** glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);
        extensions.assign(glfwExts, glfwExts + glfwExtCount);
    }

    VkApplicationInfo app{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    app.pApplicationName = "TestImGuiVulkan";
    app.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app.pEngineName = "NGine";
    app.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo ici{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    ici.pApplicationInfo = &app;
    ici.enabledExtensionCount = (uint32_t)extensions.size();
    ici.ppEnabledExtensionNames = extensions.data();

    VkInstance instance{};
    CheckVk(vkCreateInstance(&ici, nullptr, &instance), "vkCreateInstance");
    volkLoadInstance(instance);

    // Surface
    VkSurfaceKHR surface{};
    CheckVk((VkResult)glfwCreateWindowSurface(instance, window, nullptr, &surface), "glfwCreateWindowSurface");

    // Physical device
    uint32_t gpuCount = 0; vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
    if (gpuCount == 0) { std::fprintf(stderr, "No Vulkan devices found\n"); return 1; }
    std::vector<VkPhysicalDevice> gpus(gpuCount); vkEnumeratePhysicalDevices(instance, &gpuCount, gpus.data());
    VkPhysicalDevice gpu = gpus[0];

    // Queues
    QueueFamilyIndices q = FindQueueFamilies(gpu, surface);
    if (!q.complete()) { std::fprintf(stderr, "No suitable queue families\n"); return 1; }

    float prio = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> qcis;
    std::vector<uint32_t> uniqueFamilies;
    uniqueFamilies.push_back(q.graphics.value());
    if (q.present.value() != q.graphics.value()) uniqueFamilies.push_back(q.present.value());
    for (uint32_t fam : uniqueFamilies)
    {
        VkDeviceQueueCreateInfo qci{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
        qci.queueFamilyIndex = fam;
        qci.queueCount = 1;
        qci.pQueuePriorities = &prio;
        qcis.push_back(qci);
    }

    const char* devExts[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo dci{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    dci.queueCreateInfoCount = (uint32_t)qcis.size();
    dci.pQueueCreateInfos = qcis.data();
    dci.enabledExtensionCount = 1;
    dci.ppEnabledExtensionNames = devExts;

    VkDevice device{};
    CheckVk(vkCreateDevice(gpu, &dci, nullptr, &device), "vkCreateDevice");
    volkLoadDevice(device);

    VkQueue graphicsQ = VK_NULL_HANDLE;
    VkQueue presentQ = VK_NULL_HANDLE;
    vkGetDeviceQueue(device, q.graphics.value(), 0, &graphicsQ);
    vkGetDeviceQueue(device, q.present.value(), 0, &presentQ);

    // Swapchain
    Swapchain sc{};
    CreateSwapchain(gpu, device, surface, q, window, sc);

    // Render pass for presentation
    VkRenderPass mainRenderPass = CreateRenderPass(device, sc.format);

    // Framebuffers for swapchain images
    FramebufferBundle fb{}; CreateFramebuffers(device, sc, mainRenderPass, fb);

    // Command pool & buffers
    VkCommandPool cmdPool{};
    {
        VkCommandPoolCreateInfo cpci{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        cpci.queueFamilyIndex = q.graphics.value();
        cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        CheckVk(vkCreateCommandPool(device, &cpci, nullptr, &cmdPool), "vkCreateCommandPool");
    }

    const int MaxFramesInFlight = 2;
    std::vector<VkCommandBuffer> cmdBufs(MaxFramesInFlight);
    {
        VkCommandBufferAllocateInfo ai{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        ai.commandPool = cmdPool;
        ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        ai.commandBufferCount = (uint32_t)cmdBufs.size();
        CheckVk(vkAllocateCommandBuffers(device, &ai, cmdBufs.data()), "vkAllocateCommandBuffers");
    }

    // Sync objects
    std::vector<VkSemaphore> imageAvailable(MaxFramesInFlight);
    std::vector<VkSemaphore> renderFinished(MaxFramesInFlight);
    std::vector<VkFence> inFlight(MaxFramesInFlight);
    {
        VkSemaphoreCreateInfo si{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        VkFenceCreateInfo fi{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO }; fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        for (int i = 0; i < MaxFramesInFlight; ++i)
        {
            CheckVk(vkCreateSemaphore(device, &si, nullptr, &imageAvailable[i]), "vkCreateSemaphore");
            CheckVk(vkCreateSemaphore(device, &si, nullptr, &renderFinished[i]), "vkCreateSemaphore");
            CheckVk(vkCreateFence(device, &fi, nullptr, &inFlight[i]), "vkCreateFence");
        }
    }

    // Descriptor pool for ImGui
    VkDescriptorPool imguiPool{};
    {
        std::array<VkDescriptorPoolSize, 11> poolSizes = {
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
        };
        VkDescriptorPoolCreateInfo dpci{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
        dpci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        dpci.maxSets = 1000;
        dpci.poolSizeCount = (uint32_t)poolSizes.size();
        dpci.pPoolSizes = poolSizes.data();
        CheckVk(vkCreateDescriptorPool(device, &dpci, nullptr, &imguiPool), "vkCreateDescriptorPool(imgui)");
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.ApiVersion = VK_API_VERSION_1_2;
    init_info.Instance = instance;
    init_info.PhysicalDevice = gpu;
    init_info.Device = device;
    init_info.QueueFamily = q.graphics.value();
    init_info.Queue = graphicsQ;
    init_info.DescriptorPool = imguiPool;
    init_info.MinImageCount = (uint32_t)sc.images.size();
    init_info.ImageCount = (uint32_t)sc.images.size();
    init_info.PipelineInfoMain.RenderPass = mainRenderPass; // main viewport render pass
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&init_info);

    // Fonts: Recent backend versions auto-upload on first NewFrame(), so no explicit upload needed here.

    // Create offscreen target and register to ImGui
    Offscreen off{};
    CreateOffscreen(gpu, device, sc.format, VkExtent2D{ std::max(1u, sc.extent.width / 2), std::max(1u, sc.extent.height / 2) }, off);
    off.imguiSet = ImGui_ImplVulkan_AddTexture(off.sampler, off.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    auto record_offscreen = [&](VkCommandBuffer cmd)
        {
            // Transition to COLOR_ATTACHMENT_OPTIMAL
            VkImageMemoryBarrier toColor{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
            toColor.srcAccessMask = 0;
            toColor.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            toColor.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            toColor.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            toColor.image = off.image;
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
            rpbi.renderPass = off.renderPass;
            rpbi.framebuffer = off.framebuffer;
            rpbi.renderArea.extent = off.extent;
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
            toSample.image = off.image;
            toSample.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            toSample.subresourceRange.levelCount = 1;
            toSample.subresourceRange.layerCount = 1;
            vkCmdPipelineBarrier(cmd,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0, 0, nullptr, 0, nullptr, 1, &toSample);
        };

    uint32_t frameIndex = 0;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Acquire next image
        vkWaitForFences(device, 1, &inFlight[frameIndex], VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &inFlight[frameIndex]);

        uint32_t imageIndex = 0;
        VkResult acq = vkAcquireNextImageKHR(device, sc.swapchain, UINT64_MAX, imageAvailable[frameIndex], VK_NULL_HANDLE, &imageIndex);
        if (acq == VK_ERROR_OUT_OF_DATE_KHR)
        {
            // Handle resize (omitted for brevity): recreate swapchain/framebuffers/offscreen
            // For now, break to simplify sample
            break;
        }
        else if (acq != VK_SUCCESS && acq != VK_SUBOPTIMAL_KHR)
        {
            std::fprintf(stderr, "Failed to acquire image (%d)\n", acq);
            break;
        }

        // Record command buffer
        VkCommandBuffer cmd = cmdBufs[frameIndex];
        VkCommandBufferBeginInfo bi{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        CheckVk(vkBeginCommandBuffer(cmd, &bi), "vkBeginCommandBuffer");

        // 1) Render into offscreen (clear-only)
        record_offscreen(cmd);

        // 2) Start ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Offscreen View");
        ImVec2 sz((float)off.extent.width, (float)off.extent.height);
        ImGui::Text("Vulkan offscreen render (cleared color) below:");
        ImTextureRef tex_ref((ImTextureID)(intptr_t)off.imguiSet);
        ImGui::Image(tex_ref, sz);
        ImGui::End();

        ImGui::ShowDemoWindow();

        ImGui::Render();

        // 3) Main render pass to present ImGui
        VkClearValue clearMain; clearMain.color = { { 0.02f, 0.02f, 0.02f, 1.0f } };
        VkRenderPassBeginInfo rpbi{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        rpbi.renderPass = mainRenderPass;
        rpbi.framebuffer = fb.framebuffers[imageIndex];
        rpbi.renderArea.extent = sc.extent;
        rpbi.clearValueCount = 1; rpbi.pClearValues = &clearMain;
        vkCmdBeginRenderPass(cmd, &rpbi, VK_SUBPASS_CONTENTS_INLINE);

        // Render ImGui draw data into the current command buffer
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

        vkCmdEndRenderPass(cmd);

        CheckVk(vkEndCommandBuffer(cmd), "vkEndCommandBuffer");

        // Submit and present
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo si{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        si.waitSemaphoreCount = 1; si.pWaitSemaphores = &imageAvailable[frameIndex]; si.pWaitDstStageMask = &waitStage;
        si.commandBufferCount = 1; si.pCommandBuffers = &cmd;
        si.signalSemaphoreCount = 1; si.pSignalSemaphores = &renderFinished[frameIndex];
        CheckVk(vkQueueSubmit(graphicsQ, 1, &si, inFlight[frameIndex]), "vkQueueSubmit");

        VkPresentInfoKHR pi{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        pi.waitSemaphoreCount = 1; pi.pWaitSemaphores = &renderFinished[frameIndex];
        pi.swapchainCount = 1; pi.pSwapchains = &sc.swapchain; pi.pImageIndices = &imageIndex;
        VkResult prez = vkQueuePresentKHR(presentQ, &pi);
        if (prez == VK_ERROR_OUT_OF_DATE_KHR || prez == VK_SUBOPTIMAL_KHR)
        {
            // Handle resize if needed (omitted for brevity)
            break;
        }
        else
        {
            CheckVk(prez, "vkQueuePresentKHR");
        }

        frameIndex = (frameIndex + 1) % MaxFramesInFlight;
    }

    vkDeviceWaitIdle(device);

    // Cleanup
    DestroyOffscreen(device, off);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    for (int i = 0; i < (int)inFlight.size(); ++i)
    {
        if (inFlight[i]) vkDestroyFence(device, inFlight[i], nullptr);
        if (renderFinished[i]) vkDestroySemaphore(device, renderFinished[i], nullptr);
        if (imageAvailable[i]) vkDestroySemaphore(device, imageAvailable[i], nullptr);
    }

    if (imguiPool) vkDestroyDescriptorPool(device, imguiPool, nullptr);

    vkDestroyCommandPool(device, cmdPool, nullptr);

    DestroyFramebuffers(device, fb);
    vkDestroyRenderPass(device, mainRenderPass, nullptr);
    DestroySwapchain(device, sc);

    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "Test ImGui Vulkan app finished." << std::endl;
    return 0;
}
