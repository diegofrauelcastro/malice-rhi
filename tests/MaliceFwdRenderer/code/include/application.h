#pragma once

#include <GLFW/glfw3.h>
#include <MaliceRHI/malice_rhi.h>
#include <glm/glm.hpp>
#include <vector>
#include <chrono>


class Application
{
public:
	// Constructor and destructor.
	Application(const char* _appName, int _width, int _height);
	~Application();

	// Main loop.
	void Run();

private:
	/// Initialization methods ///

	void InitWindow(const char* _windowName);
	void InitRHI();
	void InitScreenRendering();
	void InitOffscreenRendering();
	void InitScene();


	/// Main loop methods ///

	void Update();
	void Draw();


	/// Cleanup method ///

	void Cleanup();


	/// Window properties ///

	int m_Width;
	int m_Height;
	GLFWwindow* m_Window = nullptr;


	/// RHI members ///

	// Main RHI objects

	IRenderInterface* m_RHI = nullptr;
	IInstance* m_Instance = nullptr;
	ISurface* m_Surface = nullptr;
	IDevice* m_Device = nullptr;
	ICommandPool* m_CommandPool = nullptr;
	ICommandBuffers* m_Commands = nullptr;

	// Screen render objects

	ISwapChain* m_SwapChain = nullptr;
	ITexture* m_DepthTex = nullptr;
	IRenderPass* m_RenderPass = nullptr;
	IFramebuffers* m_Framebuffers = nullptr;
	IShaderModules* m_Shaders = nullptr;
	IPipeline* m_Pipeline = nullptr;
	IDescriptorSetsGroup* m_DescriptorSets = nullptr;

	// Offscreen render objects

	ITexture* m_OffscreenColor = nullptr;
	ITexture* m_OffscreenDepthTex = nullptr;
	IRenderPass* m_OffscreenRenderPass = nullptr;
	IFramebuffers* m_OffscreenFramebuffers = nullptr;
	IShaderModules* m_OffscreenShaders = nullptr;
	IPipeline* m_OffscreenPipeline = nullptr;
	IDescriptorSetsGroup* m_OffscreenDescriptorSets = nullptr;
	IBuffer* m_ScreenVertexBuffer = nullptr;
	IBuffer* m_ScreenIndexBuffer = nullptr;

	// Buffers and image textures
	IBuffer* m_VertexBuffer = nullptr;
	IBuffer* m_IndexBuffer = nullptr;
	IUniformBuffers* m_CamBuffer = nullptr;
	IUniformBuffers* m_ModelBuffer = nullptr;
	IUniformBuffers* m_ColorBuffer = nullptr;
	ITexture* m_Texture = nullptr;


	/// Helpers ///

	// User-defined vertex structure.
	struct UserVertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 uv;
	};
	// Screen vertex.
	struct ScreenVertex
	{
		glm::vec2 pos;
		glm::vec2 uv;
	};

	// Uniform buffer object structure.
	struct UniformBufferObject
	{
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 model;
	};

	// Vertex data for a simple square.
	const std::vector<UserVertex> userVertices = {
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
		{{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

		{{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
		{{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
		{{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
	};
	// Index data for the square (two triangles).
	std::vector<uint16_t> userIndices = {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4 
	};

	const std::vector<ScreenVertex> screenTriangle = {
		{ {-1.0f, -1.0f}, {0.0f, 0.0f} },
		{ { 3.0f, -1.0f}, {2.0f, 0.0f} },
		{ {-1.0f,  3.0f}, {0.0f, 2.0f} }
	};
	std::vector<uint16_t> screenIndices = {
		2, 1, 0
	};

	// Method to update uniform buffers each frame. Makes use of time since application start.
	UniformBufferObject UpdateUniforms();
};