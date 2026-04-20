#include "application.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "vulkan_glfw_imgui_renderer.h"

Application::Application(const char* _appName, int _width, int _height)
	: m_Width(_width), m_Height(_height)
{
	InitWindow(_appName);
	InitRHI();
	InitScreenRendering();
	InitOffscreenRendering();
	InitImGuiRenderer();
	InitScene();
}

Application::~Application()
{
	Cleanup();
}

std::vector<char> Application::ReadFile(const std::string& _filename)
{
	// Start reading the file from the end to get the file size easily. Also, open it in binary mode.
	std::ifstream file(_filename, std::ios::ate | std::ios::binary);
	if (!file.is_open())
		return {};

	// Get the file size and allocate a buffer accordingly.
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	// Go back to the beginning of the file and read all its content into the buffer.
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	// Close the file and return it.
	file.close();
	return buffer;
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

}

void Application::InitScreenRendering()
{
	// Swap chain.
	m_SwapChain = m_RHI->InstantiateSwapChain();
	m_SwapChain->Create(m_Device, m_Surface, m_Window, true);

	// Depth texture
	m_DepthTex = m_RHI->InstantiateTexture();
	m_DepthTex->Create(m_Device, m_CommandPool, m_Width, m_Height, ETextureFormat::MRHI_DEPTH32, ETextureUsage::MRHI_DEPTH_ATTACHMENT);

	// Main render pass.
	m_RenderPass = m_RHI->InstantiateRenderPass();
	m_RenderPass->Create(m_Device, m_SwapChain, true);

	// Framebuffers.
	m_Framebuffers = m_RHI->InstantiateFramebuffers();
	m_Framebuffers->Create(m_Device, m_RenderPass, m_SwapChain, m_DepthTex);

	// Shader location params
	uint32_t vertexTotalSize = sizeof(ScreenVertex);
	VertexInputLocationParams posParams;
	posParams.location = 0;
	posParams.type = MRHI_VEC2;
	posParams.memoryOffset = offsetof(ScreenVertex, ScreenVertex::pos);
	VertexInputLocationParams uvParams;
	uvParams.location = 1;
	uvParams.type = MRHI_VEC2;
	uvParams.memoryOffset = offsetof(ScreenVertex, ScreenVertex::uv);
	std::vector<VertexInputLocationParams> params = { posParams, uvParams };

	// Shader modules.
	std::vector<char> vertexShaderCode = ReadFile("resources/shaders/vert.spv");
	std::vector<char> fragmentShaderCode = ReadFile("resources/shaders/frag.spv");
	m_Shaders = m_RHI->InstantiateShaderModules();
	m_Shaders->Create(m_Device, vertexShaderCode, fragmentShaderCode, vertexTotalSize, params);

	// Descriptor set bindings.
	m_Shaders->AddDescriptorSetBinding(0, 0, 1, MRHI_FRAGMENT_SHADER, MRHI_COMBINED_IMAGE_SAMPLER);

	// Graphics pipeline.
	m_Pipeline = m_RHI->InstantiatePipeline();
	PipelineParams pipeline;
	pipeline.enableRasterizerDiscard = false;
	pipeline.enableDepthClamp = false;
	pipeline.inputTopologyMode = MRHI_TRIANGLE_LIST;
	pipeline.polygonMode = MRHI_FILL;
	pipeline.enablePrimitiveRestart = false;
	pipeline.rasterizerLineWidth = 1.0f;
	pipeline.frontFace = MRHI_COUNTER_CLOCKWISE;
	pipeline.cullingMode = MRHI_CULL_BACK_FACE;
	pipeline.enableDepthBias = false;
	pipeline.enableColorBlend = false;
	m_Pipeline->Create(m_Device, m_RenderPass, m_Shaders, pipeline);

	// Descriptor sets bundle
	m_DescriptorSets = m_RHI->InstantiateDescriptorSetsBundle();
	m_DescriptorSets->Create(m_Device, m_Pipeline);

	// Command buffers.
	m_Commands = m_RHI->InstantiateCommandBuffers();
	m_Commands->Create(m_Device, m_CommandPool, m_SwapChain);

	// Screen index and vertex buffers.
	m_ScreenVertexBuffer = m_RHI->InstantiateBuffer();
	m_ScreenIndexBuffer = m_RHI->InstantiateBuffer();

	m_ScreenVertexBuffer->Create(m_Device, m_CommandPool, MRHI_VERTEX_BUFFER, sizeof(ScreenVertex) * 3, screenTriangle.data());
	m_ScreenIndexBuffer->Create(m_Device, m_CommandPool, MRHI_INDEX_BUFFER, sizeof(uint16_t) * 3, screenIndices.data());
}

void Application::InitOffscreenRendering()
{
	// Color texture.
	m_OffscreenColor = m_RHI->InstantiateTexture();
	m_OffscreenColor->Create(m_Device, m_CommandPool, m_Width, m_Height, ETextureFormat::MRHI_RGBA8, ETextureUsage::MRHI_COLOR_ATTACHMENT | ETextureUsage::MRHI_SAMPLED);

	// Depth texture.
	m_OffscreenDepthTex = m_RHI->InstantiateTexture();
	m_OffscreenDepthTex->Create(m_Device, m_CommandPool, m_Width, m_Height, ETextureFormat::MRHI_DEPTH32, ETextureUsage::MRHI_DEPTH_ATTACHMENT);

	// Main render pass.
	m_OffscreenRenderPass = m_RHI->InstantiateRenderPass();
	RenderPassParams rpp{};
	rpp.colorFormats = { ETextureFormat::MRHI_RGBA8 };
	rpp.depthFormat = ETextureFormat::MRHI_DEPTH32;
	rpp.hasDepth = true;
	m_OffscreenRenderPass->Create(m_Device, rpp);

	// Framebuffers.
	m_OffscreenFramebuffers = m_RHI->InstantiateFramebuffers();
	FramebufferParams fbp{};
	fbp.colorAttachments = { m_OffscreenColor };
	fbp.depthAttachment = m_OffscreenDepthTex;
	fbp.width = m_Width;
	fbp.height = m_Height;
	m_OffscreenFramebuffers->Create(m_Device, m_OffscreenRenderPass, fbp);

	// Shader location params
	uint32_t vertexTotalSize = sizeof(UserVertex);
	VertexInputLocationParams posParams;
	posParams.location = 0;
	posParams.type = MRHI_VEC3;
	posParams.memoryOffset = offsetof(UserVertex, UserVertex::pos);
	VertexInputLocationParams colorParams;
	colorParams.location = 1;
	colorParams.type = MRHI_VEC3;
	colorParams.memoryOffset = offsetof(UserVertex, UserVertex::color);
	VertexInputLocationParams uvParams;
	uvParams.location = 2;
	uvParams.type = MRHI_VEC2;
	uvParams.memoryOffset = offsetof(UserVertex, UserVertex::uv);
	std::vector<VertexInputLocationParams> params = { posParams, colorParams, uvParams };

	// Shader modules.
	std::vector<char> vertexShaderCode = ReadFile("resources/shaders/offscreenVert.spv");
	std::vector<char> fragmentShaderCode = ReadFile("resources/shaders/offscreenFrag.spv");
	m_OffscreenShaders = m_RHI->InstantiateShaderModules();
	m_OffscreenShaders->Create(m_Device, vertexShaderCode, fragmentShaderCode, vertexTotalSize, params);

	// Descriptor set bindings.
	m_OffscreenShaders->AddDescriptorSetBinding(0, 0, 1, MRHI_VERTEX_SHADER, MRHI_UNIFORM_BUFFER_DYNAMIC);
	m_OffscreenShaders->AddDescriptorSetBinding(0, 1, 1, MRHI_VERTEX_SHADER, MRHI_UNIFORM_BUFFER);
	m_OffscreenShaders->AddDescriptorSetBinding(1, 0, 1, MRHI_FRAGMENT_SHADER, MRHI_UNIFORM_BUFFER);
	m_OffscreenShaders->AddDescriptorSetBinding(1, 1, 1, MRHI_FRAGMENT_SHADER, MRHI_COMBINED_IMAGE_SAMPLER);

	// Graphics pipeline.
	m_OffscreenPipeline = m_RHI->InstantiatePipeline();
	PipelineParams pipeline;
	pipeline.enableRasterizerDiscard = false;
	pipeline.enableDepthClamp = false;
	pipeline.inputTopologyMode = MRHI_TRIANGLE_LIST;
	pipeline.polygonMode = MRHI_FILL;
	pipeline.enablePrimitiveRestart = false;
	pipeline.rasterizerLineWidth = 1.0f;
	pipeline.frontFace = MRHI_COUNTER_CLOCKWISE;
	pipeline.cullingMode = MRHI_CULL_BACK_FACE;
	pipeline.enableDepthBias = false;
	pipeline.enableColorBlend = false;

	// Push constants layout.
	m_OffscreenPipeline->AddPushConstant(sizeof(glm::vec3), MRHI_VERTEX_SHADER, 0);
	m_OffscreenPipeline->AddPushConstant(sizeof(float), MRHI_VERTEX_SHADER, sizeof(float) * 3);

	m_OffscreenPipeline->Create(m_Device, m_OffscreenRenderPass, m_OffscreenShaders, pipeline);

	// Descriptor sets bundle
	m_OffscreenDescriptorSets = m_RHI->InstantiateDescriptorSetsBundle();
	m_OffscreenDescriptorSets->Create(m_Device, m_OffscreenPipeline);
}

void Application::InitImGuiRenderer()
{
	Offscreen offscreenParams{};
	offscreenParams.colorTex = m_OffscreenColor;
	offscreenParams.framebuffers = m_OffscreenFramebuffers;
	offscreenParams.renderPass = m_OffscreenRenderPass;

	m_Bridge = m_RHI->InstantiateMaliceToImGuiBridge();
	m_Bridge->Create(m_Instance, m_Device, m_SwapChain, m_RenderPass, m_Framebuffers);
	m_Bridge->AddOffscreenTarget("MainTarget", offscreenParams);

	m_ImGuiRenderer = new Vulkan_GLFW_ImGuiRenderer();
	m_ImGuiRenderer->Create(m_Bridge, m_Window);
}

void Application::InitScene()
{
	// Vertex and index buffers.
	m_VertexBuffer = m_RHI->InstantiateBuffer();
	m_IndexBuffer = m_RHI->InstantiateBuffer();

	m_VertexBuffer->Create(m_Device, m_CommandPool, MRHI_VERTEX_BUFFER, sizeof(UserVertex) * userVertices.size(), userVertices.data());
	m_IndexBuffer->Create(m_Device, m_CommandPool, MRHI_INDEX_BUFFER, sizeof(uint16_t) * userIndices.size(), userIndices.data());

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
	m_Texture->Create(m_Device, m_CommandPool, texWidth, texHeight, ETextureFormat::MRHI_RGBA8, ETextureUsage::MRHI_SAMPLED, imgData);
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
	// White color.
	glm::vec4 c = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_ColorBuffer->UploadData(m_Commands, sizeof(glm::vec4), &c);
}


void Application::Draw()
{
	// Recording commands.
	uint32_t img = 0;

	bool isValid = m_Commands->BeginFrame(m_Device, m_SwapChain, img);
	if (!isValid) return;

	// Render scene in offscreen texture.
	m_Commands->SetClearColor({ 1.0f, 0.0f, 0.0f, 1.0f });
	m_Commands->BeginRender(m_OffscreenRenderPass, m_OffscreenFramebuffers, 0);
		m_Commands->BindPipeline(m_OffscreenPipeline);

		m_Commands->UpdateUniformBuffer(m_Device, m_OffscreenDescriptorSets, m_ModelBuffer, 0, 0, 1, true);
		m_Commands->UpdateUniformBuffer(m_Device, m_OffscreenDescriptorSets, m_CamBuffer, 0, 1, 1, false);
		m_Commands->UpdateUniformBuffer(m_Device, m_OffscreenDescriptorSets, m_ColorBuffer, 1, 0, 1, false);
		uint32_t dynamicOffset = 0; // We have only one model, so the offset is 0.
		m_Commands->BindDescriptorSetsDynamically(m_OffscreenPipeline, m_OffscreenDescriptorSets, 1, &dynamicOffset);
		// Changing color over time (cycling).
		glm::vec3 c = glm::vec3((sin(glfwGetTime()) + 1) / 2, (cos(glfwGetTime()) + 1) / 2, 0);
		m_Commands->SendPushConstants(m_OffscreenPipeline, &c, sizeof(glm::vec3), 0);
		m_Commands->SendPushConstants(m_OffscreenPipeline, &c, sizeof(float), sizeof(glm::vec3));

		m_Commands->UpdateTexture(m_Device, m_OffscreenDescriptorSets, m_Texture, 1, 1);

		m_Commands->DrawVerticesByIndices((uint32_t)userIndices.size(), m_VertexBuffer, m_IndexBuffer);
	m_Commands->EndRender();

	// Render the texture on a screen triangle.
	//m_Commands->BeginRender(m_RenderPass, m_Framebuffers, img);
	//	m_Commands->BindPipeline(m_Pipeline);

	//	m_Commands->BindDescriptorSets(m_Pipeline, m_DescriptorSets);
	//	m_Commands->UpdateTexture(m_Device, m_DescriptorSets, m_OffscreenColor, 0, 0);
	//	m_Commands->DrawVerticesByIndices(3, m_ScreenVertexBuffer, m_ScreenIndexBuffer);
	//m_Commands->EndRender();

	m_ImGuiRenderer->RecordNewFrame();
	m_ImGuiRenderer->ShowOffscreenRenderInWindow();
	m_ImGuiRenderer->ShowDemoWindow();

	// Render on screen the ImGui windows.
	m_Commands->SetClearColor({ 0.2f, 0.2f, 0.2f, 1.0f });
	m_Commands->BeginRender(m_RenderPass, m_Framebuffers, img);
		m_ImGuiRenderer->DrawImGuiData(m_Commands);
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

	// Destroy imgui renderer and brige.
	m_ImGuiRenderer->Destroy();
	delete m_ImGuiRenderer;
	m_Bridge->Destroy(m_Device);
	m_RHI->DeleteMaliceToImGuiBridge(m_Bridge);

	m_OffscreenDescriptorSets->Destroy(m_Device);
	m_RHI->DeleteDescriptorSetsBundle(m_OffscreenDescriptorSets);
	m_ScreenVertexBuffer->Destroy(m_Device);
	m_RHI->DeleteBuffer(m_ScreenVertexBuffer);
	m_ScreenIndexBuffer->Destroy(m_Device);
	m_RHI->DeleteBuffer(m_ScreenIndexBuffer);
	m_OffscreenFramebuffers->Destroy(m_Device);
	m_RHI->DeleteFramebuffers(m_OffscreenFramebuffers);
	m_OffscreenRenderPass->Destroy(m_Device);
	m_RHI->DeleteRenderPass(m_OffscreenRenderPass);
	m_OffscreenColor->Destroy(m_Device);
	m_RHI->DeleteTexture(m_OffscreenColor);
	m_OffscreenDepthTex->Destroy(m_Device);
	m_RHI->DeleteTexture(m_OffscreenDepthTex);
	m_OffscreenPipeline->Destroy(m_Device);
	m_RHI->DeletePipeline(m_OffscreenPipeline);
	m_OffscreenShaders->Destroy(m_Device);
	m_RHI->DeleteShaderModules(m_OffscreenShaders);

	// Command buffers and pool.
	m_Commands->Destroy(m_Device, m_CommandPool);
	m_RHI->DeleteCommandBuffers(m_Commands);

	// Command pool.
	m_CommandPool->Destroy(m_Device);
	m_RHI->DeleteCommandPool(m_CommandPool);

	m_DepthTex->Destroy(m_Device);
	m_RHI->DeleteTexture(m_DepthTex);
	m_Pipeline->Destroy(m_Device);
	m_RHI->DeletePipeline(m_Pipeline);
	m_Shaders->Destroy(m_Device);
	m_RHI->DeleteShaderModules(m_Shaders);
	m_Framebuffers->Destroy(m_Device);
	m_RHI->DeleteFramebuffers(m_Framebuffers);
	m_SwapChain->Destroy(m_Device);
	m_RHI->DeleteSwapChain(m_SwapChain);
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