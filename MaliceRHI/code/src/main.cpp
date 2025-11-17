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
#include "idescriptorsetsgroup.h"
#include "iuniformbuffers.h"
#include "debug.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <chrono>

#define WIDTH 800
#define HEIGHT 600

struct UserVertex
{
    glm::vec2 pos;
    glm::vec3 color;
};

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
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
0, 1, 2, 2, 3, 0
};

static UniformBufferObject UpdateUniformBuffer()
{
    // Calculate the time since the start of the application.
    static auto startTime = std::chrono::high_resolution_clock::now();
    // Get the current time.
    auto currentTime = std::chrono::high_resolution_clock::now();
    // Calculate the time difference in seconds as a float.
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    std::cout << "Time: " << time << "s\r";

    // Create the model, view, and projection matrices.
    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), (float)800 / (float)600, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
	return ubo;
}

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
    posParams.type = VEC2;
    posParams.memoryOffset = offsetof(UserVertex, UserVertex::pos);
    VertexInputLocationParams colorParams;
    colorParams.location = 1;
    colorParams.type = VEC3;
    colorParams.memoryOffset = offsetof(UserVertex, UserVertex::color);

    // Shader modules
    IShaderModules* shaders = RHI->InstantiateShaderModules();
    shaders->Create(device, "resources/shaders/vert.spv", "resources/shaders/frag.spv", vertexTotalSize, { posParams, colorParams });
    // Descriptor sets params (optionnal)
    shaders->AddDescriptorSetBinding(0, 0, 1, VERTEX_SHADER);
    shaders->AddDescriptorSetBinding(1, 0, 1, FRAGMENT_SHADER);

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

	// Descriptor sets bundle
    IDescriptorSetsGroup* descriptorSets = RHI->InstantiateDescriptorSetsBundle();
	descriptorSets->Create(device, pipeline, swapChain);
    
    // Uniform buffer
	IUniformBuffers* mvpBuffer = RHI->InstantiateUniformBuffers();
    IUniformBuffers* colorBuffer = RHI->InstantiateUniformBuffers();
	mvpBuffer->Create(device, swapChain, sizeof(UniformBufferObject));
    colorBuffer->Create(device, swapChain, sizeof(glm::vec4));

    //// LOOP

    LOG_CLEAN("\n\n===== LOOP =====\n")
    LOG_DEBUG("User loop start...")
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

		// Update uniform buffer
		UniformBufferObject ubo = UpdateUniformBuffer();
		mvpBuffer->UploadData(commands, sizeof(glm::mat4)*3, &ubo);

        // Update color
		glm::vec4 newColor = glm::vec4((sin(glfwGetTime()) + 1.0f) / 2.0f, (cos(glfwGetTime()) + 1.0f) / 2.0f, 0.0f, 1.0f);
		colorBuffer->UploadData(commands, sizeof(glm::vec4), &newColor);

		// Drawing code
        uint32_t imageIndex = swapChain->AcquireNextImage(device, commands->GetCurrentFrame());
        commands->BeginDraw(renderPass, swapChain, framebuffers, imageIndex);

            commands->BindPipeline(pipeline);
            
			commands->BindDescriptorSets(pipeline, descriptorSets);
			commands->UpdateUniformBuffer(device, descriptorSets, mvpBuffer, 0, 0, 1);
			commands->UpdateUniformBuffer(device, descriptorSets, colorBuffer, 1, 0, 1);

            commands->DrawVerticesByIndices((uint32_t)userIndices.size(), vertexBuffer, indexBuffer);

        commands->EndDraw();
        commands->SubmitAndPresent(device, swapChain, imageIndex);
    }
    LOG_DEBUG("User loop end. Waiting for device to be idle...")
    device->WaitIdle();
    LOG_DEBUG("Device is idle. Resuming.")

    //// DESTROY

	// Uniform buffer
	mvpBuffer->Destroy(device);
	RHI->DeleteUniformBuffers(mvpBuffer);
	mvpBuffer = nullptr;
	colorBuffer->Destroy(device);
	RHI->DeleteUniformBuffers(colorBuffer);
	colorBuffer = nullptr;

	// Descriptor sets bundle
	descriptorSets->Destroy(device);
	RHI->DeleteDescriptorSetsBundle(descriptorSets);
	descriptorSets = nullptr;

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
