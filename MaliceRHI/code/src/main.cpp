#include "vulkan_render_interface.h"

#include "iinstance.h"
#include "idevice.h"
#include "isurface.h"
#include "iswapchain.h"
#include "irenderpass.h"
#include "iframebuffers.h"
#include "ishadermodules.h"
#include "ipipeline.h"
#include "icommandpool.h"
#include "icommandbuffers.h"
#include "ibuffer.h"
#include "debug.h"

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vector>

#define WIDTH 800
#define HEIGHT 600

struct UserVertex
{
    glm::vec2 pos;
    glm::vec3 color;
};

// Vertex data for a simple square.
std::vector<UserVertex> userVertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};
// Index data for the square (two triangles).
std::vector<uint16_t> userIndices = {
0, 3, 2, 2, 1, 0
};


int main()
{
    ///////////////////////////

    //// INIT
    
    // Window
    glfwInit();
    // Change window parameters.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // Create window.
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "RHI App", nullptr, nullptr);
    //glfwSetWindowUserPointer(window, this);
    
    // Factory class
    IRenderInterface* RHI = new VulkanRenderInterface();

    // Instance
    IInstance* instance = RHI->InstantiateInstance();
    instance->Create("RHI App");
    
    // Surface
    ISurface* surface = RHI->InstantiateSurface();
    surface->Create(instance, window);

    // Device (GPU)
    IDevice* device = RHI->InstantiateDevice();
    device->Create(instance, surface);

    // Swap chain
    ISwapChain* swapChain = RHI->InstantiateSwapChain();
    swapChain->Create(device, surface, window);

    // Render pass
    IRenderPass* renderPass = RHI->InstantiateRenderPass();
    renderPass->Create(device, swapChain);

    // Framebuffers
    IFramebuffers* framebuffers = RHI->InstantiateFramebuffers();
    framebuffers->Create(device, swapChain, renderPass);

    // Shader location params
    uint32_t vertexTotalSize = sizeof(UserVertex);
    VertexInputLocationParams posParams;
    posParams.location = 0;
    posParams.type = EShaderDataType::VEC2;
    posParams.offset = offsetof(UserVertex, UserVertex::pos);
    VertexInputLocationParams colorParams;
    colorParams.location = 1;
    colorParams.type = EShaderDataType::VEC3;
    colorParams.offset = offsetof(UserVertex, UserVertex::color);

    // Shader modules
    IShaderModules* shaders = RHI->InstantiateShaderModules();
    shaders->Create(device, "resources/shaders/vert.spv", "resources/shaders/frag.spv", vertexTotalSize, { posParams, colorParams });

    // Graphics pipeline
    IPipeline* pipeline = RHI->InstantiatePipeline();
    pipeline->Create(device, renderPass, shaders);

    // Command pool
    ICommandPool* commandPool = RHI->InstantiateCommandPool();
    commandPool->Create(device);

    // Command buffer
    ICommandBuffers* commands = RHI->InstantiateCommandBuffers();
    commands->Create(device, commandPool, swapChain);

    // Vertex and index buffers
    IBuffer* vertexBuffer = RHI->InstantiateBuffer();
    IBuffer* indexBuffer = RHI->InstantiateBuffer();
    vertexBuffer->Create(device, commandPool, VERTEX_BUFFER, sizeof(userVertices[0]) * userVertices.size(), (void*)userVertices.data());
    indexBuffer->Create(device, commandPool, INDEX_BUFFER, sizeof(userIndices[0]) * userIndices.size(), (void*)userIndices.data());

    //// LOOP

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        uint32_t imageIndex = swapChain->AcquireNextImage(device, commands->GetCurrentFrame());
        commands->BeginDraw(renderPass, swapChain, framebuffers, imageIndex);

            commands->BindPipeline(pipeline);
            commands->DrawVerticesByIndices(userIndices.size(), vertexBuffer, indexBuffer);

        commands->EndDraw();
        commands->SubmitAndPresent(device, swapChain, imageIndex);
    }
    device->WaitIdle();

    //// DESTROY

    // Buffers
    vertexBuffer->Destroy(device);
    RHI->DeleteBuffer(vertexBuffer);
    indexBuffer->Destroy(device);
    RHI->DeleteBuffer(indexBuffer);
    vertexBuffer = nullptr;
    indexBuffer = nullptr;

    // Command buffer
    commands->Destroy(device, commandPool);
    RHI->DeleteCommandBuffers(commands);
    commands = nullptr;

    // Command pool
    commandPool->Destroy(device);
    RHI->DeleteCommandPool(commandPool);
    commandPool = nullptr;

    // Graphics pipeline
    pipeline->Destroy(device);
    RHI->DeletePipeline(pipeline);
    pipeline = nullptr;

    // Shader modules
    shaders->Destroy(device);
    RHI->DeleteShaderModules(shaders);
    shaders = nullptr;

    // Framebuffers
    framebuffers->Destroy(device);
    RHI->DeleteFramebuffers(framebuffers);
    framebuffers = nullptr;

    // Swap chain
    swapChain->Destroy(device);
    RHI->DeleteSwapChain(swapChain);
    swapChain = nullptr;

    // Render Pass
    renderPass->Destroy(device);
    RHI->DeleteRenderPass(renderPass);
    renderPass = nullptr;

    // Device (GPU)
    device->Destroy();
    RHI->DeleteDevice(device);
    device = nullptr;

    // Surface
    surface->Destroy(instance);
    RHI->DeleteSurface(surface);
    surface = nullptr;

    // Instance
    instance->Destroy();
    RHI->DeleteInstance(instance);
    instance = nullptr;

    // Window
    glfwDestroyWindow(window);
    glfwTerminate();

    userVertices.clear();
    userIndices.clear();
    userVertices.shrink_to_fit();
    userIndices.shrink_to_fit();

    // Free factory class pointer.
    delete RHI;
    MaliceRHI::Debug::Log::GetInstance()->Destroy();

    ///////////////////////////
    return 0;
}
