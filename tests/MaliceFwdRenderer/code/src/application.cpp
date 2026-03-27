#include "application.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Application::Application(const char* _appName, int _width, int _height)
	: m_Width(_width), m_Height(_height)
{
	InitWindow(_appName);
	InitRHI();
	InitScene();
}

Application::~Application()
{
	Cleanup();
}

void Application::InitWindow(const char* _windowName)
{
	// GLFW window.
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_Window = glfwCreateWindow(m_Width, m_Height, _windowName, nullptr, nullptr);
	glfwSetWindowUserPointer(m_Window, this);
}

void Application::InitRHI()
{
	// RHI.
	m_RHI = new VulkanRenderInterface();

	// Instance.
	m_Instance = m_RHI->InstantiateInstance();
	m_Instance->Create("RHI App");

	// Surface.
	m_Surface = m_RHI->InstantiateSurface();
	m_Surface->Create(m_Instance, m_Window);

	// Device.
	m_Device = m_RHI->InstantiateDevice();
	m_Device->Create(m_Instance, m_Surface);

	// Command pool.
	m_CommandPool = m_RHI->InstantiateCommandPool();
	m_CommandPool->Create(m_Device);

	// Swap chain.
	m_SwapChain = m_RHI->InstantiateSwapChain();
	m_SwapChain->Create(m_Device, m_Surface, m_Window);

	// Main render pass.
	m_RenderPass = m_RHI->InstantiateRenderPass();
	m_RenderPass->Create(m_Device, m_SwapChain, true);

	// Depth texture
	m_DepthTex = m_RHI->InstantiateTexture();
	m_DepthTex->Create(m_Device, m_CommandPool, m_Width, m_Height, ETextureFormat::DEPTH32, ETextureUsage::DEPTH_ATTACHMENT);

	// Framebuffers.
	m_Framebuffers = m_RHI->InstantiateFramebuffers();
	m_Framebuffers->Create(m_Device, m_RenderPass, m_SwapChain, m_DepthTex);
}

void Application::InitScene()
{
	// Shader location params
    uint32_t vertexTotalSize = sizeof(UserVertex);
    VertexInputLocationParams posParams;
    posParams.location = 0;
    posParams.type = VEC3;
    posParams.memoryOffset = offsetof(UserVertex, UserVertex::pos);
    VertexInputLocationParams colorParams;
    colorParams.location = 1;
    colorParams.type = VEC3;
    colorParams.memoryOffset = offsetof(UserVertex, UserVertex::color);
	VertexInputLocationParams uvParams;
	uvParams.location = 2;
	uvParams.type = VEC2;
	uvParams.memoryOffset = offsetof(UserVertex, UserVertex::uv);
	std::vector<VertexInputLocationParams> params = { posParams, colorParams, uvParams };

	// Shader modules.
	m_Shaders = m_RHI->InstantiateShaderModules();
	m_Shaders->Create(m_Device, "resources/shaders/vert.spv", "resources/shaders/frag.spv", sizeof(UserVertex), params);

	// Descriptor set bindings.
	m_Shaders->AddDescriptorSetBinding(0, 0, 1, VERTEX_SHADER);
	m_Shaders->AddDescriptorSetBinding(0, 1, 1, VERTEX_SHADER);
	m_Shaders->AddDescriptorSetBinding(1, 0, 1, FRAGMENT_SHADER);
	m_Shaders->AddDescriptorSetBinding(1, 1, 1, FRAGMENT_SHADER, true);

	// Graphics pipeline.
	m_Pipeline = m_RHI->InstantiatePipeline();
	PipelineParams pipeline;
	pipeline.enableRasterizerDiscard = false;
	pipeline.enableDepthClamp = false;
	pipeline.inputTopologyMode = TRIANGLE_LIST;
	pipeline.polygonMode = FILL;
	pipeline.enablePrimitiveRestart = false;
	pipeline.rasterizerLineWidth = 1.0f;
	pipeline.frontFace = COUNTER_CLOCKWISE;
	pipeline.cullingMode = CULL_BACK_FACE;
	pipeline.enableDepthBias = false;
	pipeline.enableColorBlend = false;
	m_Pipeline->Create(m_Device, m_RenderPass, m_Shaders, pipeline);

	// Command buffers.
	m_Commands = m_RHI->InstantiateCommandBuffers();
	m_Commands->Create(m_Device, m_CommandPool, m_SwapChain);

	// Vertex and index buffers.
	m_VertexBuffer = m_RHI->InstantiateBuffer();
	m_IndexBuffer = m_RHI->InstantiateBuffer();

	m_VertexBuffer->Create(m_Device, m_CommandPool, VERTEX_BUFFER, sizeof(UserVertex) * userVertices.size(), userVertices.data());
	m_IndexBuffer->Create(m_Device, m_CommandPool, INDEX_BUFFER, sizeof(uint16_t) * userIndices.size(), userIndices.data());

	// Descriptor sets bundle
    m_DescriptorSets = m_RHI->InstantiateDescriptorSetsBundle();
	m_DescriptorSets->Create(m_Device, m_Pipeline, m_SwapChain);

	// Uniform buffers.
	m_CamBuffer = m_RHI->InstantiateUniformBuffers();
	m_ModelBuffer = m_RHI->InstantiateUniformBuffers();
	m_ColorBuffer = m_RHI->InstantiateUniformBuffers();

	m_CamBuffer->Create(m_Device, m_SwapChain, sizeof(glm::mat4) * 2);
	m_ModelBuffer->Create(m_Device, m_SwapChain, sizeof(glm::mat4));
	m_ColorBuffer->Create(m_Device, m_SwapChain, sizeof(glm::vec4));

	// Texture
	int texWidth, texHeight, texChannels;
	stbi_uc* imgData = stbi_load("resources/textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	m_Texture = m_RHI->InstantiateTexture();
	m_Texture->Create(m_Device, m_CommandPool, texWidth, texHeight, ETextureFormat::RGBA8, ETextureUsage::SAMPLED, imgData);
	stbi_image_free(imgData);
}

Application::UniformBufferObject Application::UpdateUniforms()
{
	static auto start = std::chrono::high_resolution_clock::now();
	float t = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count();

	// Update the screen size.
	glfwGetFramebufferSize(m_Window, &m_Width, &m_Height);

	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.f), t, glm::vec3(0.f, 0.f, 1.f));
	ubo.view = glm::lookAt(glm::vec3(0.f, 2.f, 2.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
	std::vector<float> projMatVec = m_RHI->GetPerspectiveProjectionMatrix(m_Width, m_Height, 0.1f, 100.f, 45.f);
	//std::vector<float> projMatVec = m_RHI->GetOrthographicProjectionMatrix(m_Width, m_Height, 0.1f, 100.f);
	glm::mat4 projMat;
	for (int i = 0; i < projMatVec.size() || i < 16; i++)
		projMat[i%4][i/4] = projMatVec[i];
	ubo.proj = projMat;
	ubo.proj[1][1] *= -1;

	return ubo;
}

void Application::Update()
{
	// Update uniform buffers.
	UniformBufferObject u = UpdateUniforms();
	// Camera matrices.
	m_CamBuffer->UploadData(m_Commands, sizeof(glm::mat4)*2, &u);
	m_ModelBuffer->UploadData(m_Commands, sizeof(glm::mat4), &u.model);
	// Changing color over time (cycling).
	glm::vec4 c = glm::vec4((sin(glfwGetTime()) + 1) / 2, (cos(glfwGetTime()) + 1) / 2, 0, 1);
	m_ColorBuffer->UploadData(m_Commands, sizeof(glm::vec4), &c);
}


void Application::Draw()
{
	// Recording commands.
	m_Commands->SetClearColor({ 1.0f, 0.0f, 0.0f, 0.0f });
	uint32_t img = 0;
	m_Commands->BeginFrame(m_Device, m_SwapChain, img);

	m_Commands->BeginRender(m_RenderPass, m_Framebuffers, img);
		m_Commands->BindPipeline(m_Pipeline);

		m_Commands->BindDescriptorSets(m_Pipeline, m_DescriptorSets);
		m_Commands->UpdateUniformBuffer(m_Device, m_DescriptorSets, m_ModelBuffer, 0, 0, 1);
		m_Commands->UpdateUniformBuffer(m_Device, m_DescriptorSets, m_CamBuffer, 0, 1, 1);
		m_Commands->UpdateUniformBuffer(m_Device, m_DescriptorSets, m_ColorBuffer, 1, 0, 1);
		m_Commands->UpdateTexture(m_Device, m_DescriptorSets, m_Texture, 1, 1);

		m_Commands->DrawVerticesByIndices((uint32_t)userIndices.size(), m_VertexBuffer, m_IndexBuffer);
	m_Commands->EndRender();

	m_Commands->EndFrame();
	// Submitting commands and presenting the image.
	m_Commands->SubmitAndPresent(m_Device, m_SwapChain, m_Framebuffers, img);
}

void Application::Run()
{
	// Main loop.
	while (!glfwWindowShouldClose(m_Window))
	{
		glfwPollEvents();
		Update();
		Draw();
	}
	// Wait for the device to be idle before cleanup.
	m_Device->WaitIdle();
}

void Application::Cleanup()
{
	// Cleanup in reverse order of creation.

	// Depth texture
	m_DepthTex->Destroy(m_Device);

	// Texture
	m_Texture->Destroy(m_Device);

	// Uniform buffers.
	m_CamBuffer->Destroy(m_Device);
	m_RHI->DeleteUniformBuffers(m_CamBuffer);
	m_ModelBuffer->Destroy(m_Device);
	m_RHI->DeleteUniformBuffers(m_ModelBuffer);
	m_ColorBuffer->Destroy(m_Device);
	m_RHI->DeleteUniformBuffers(m_ColorBuffer);

	// Descriptor sets.
	m_DescriptorSets->Destroy(m_Device);
	m_RHI->DeleteDescriptorSetsBundle(m_DescriptorSets);

	// Vertex and index buffers.
	m_VertexBuffer->Destroy(m_Device);
	m_RHI->DeleteBuffer(m_VertexBuffer);
	m_IndexBuffer->Destroy(m_Device);
	m_RHI->DeleteBuffer(m_IndexBuffer);

	// Command buffers and pool.
	m_Commands->Destroy(m_Device, m_CommandPool);
	m_RHI->DeleteCommandBuffers(m_Commands);

	// Command pool.
	m_CommandPool->Destroy(m_Device);
	m_RHI->DeleteCommandPool(m_CommandPool);

	// Graphics pipeline.
	m_Pipeline->Destroy(m_Device);
	m_RHI->DeletePipeline(m_Pipeline);

	// Shader modules.
	m_Shaders->Destroy(m_Device);
	m_RHI->DeleteShaderModules(m_Shaders);

	// Framebuffers.
	m_Framebuffers->Destroy(m_Device);
	m_RHI->DeleteFramebuffers(m_Framebuffers);

	// Swap chain.
	m_SwapChain->Destroy(m_Device);
	m_RHI->DeleteSwapChain(m_SwapChain);

	// Render pass.
	m_RenderPass->Destroy(m_Device);
	m_RHI->DeleteRenderPass(m_RenderPass);

	// Device.
	m_Device->Destroy();
	m_RHI->DeleteDevice(m_Device);

	// Surface.
	m_Surface->Destroy(m_Instance);
	m_RHI->DeleteSurface(m_Surface);

	// Instance.
	m_Instance->Destroy();
	m_RHI->DeleteInstance(m_Instance);

	// GLFW window and termination.
	glfwDestroyWindow(m_Window);
	glfwTerminate();

	// RHI.
	delete m_RHI;
}