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

	IRenderInterface* m_RHI = nullptr;
	IInstance* m_Instance = nullptr;
	ISurface* m_Surface = nullptr;
	IDevice* m_Device = nullptr;
	ISwapChain* m_SwapChain = nullptr;
	IRenderPass* m_RenderPass = nullptr;
	IFramebuffers* m_Framebuffers = nullptr;
	IShaderModules* m_Shaders = nullptr;
	IPipeline* m_Pipeline = nullptr;
	ICommandPool* m_CommandPool = nullptr;
	ICommandBuffers* m_Commands = nullptr;
	IDescriptorSetsGroup* m_DescriptorSets = nullptr;
	ITexture* m_Texture = nullptr;
	ITexture* m_DepthTex = nullptr;

	IBuffer* m_VertexBuffer = nullptr;
	IBuffer* m_IndexBuffer = nullptr;
	IUniformBuffers* m_CamBuffer = nullptr;
	IUniformBuffers* m_ModelBuffer = nullptr;
	IUniformBuffers* m_ColorBuffer = nullptr;


	/// Helpers ///

	// User-defined vertex structure.
	struct UserVertex
	{
		glm::vec3 pos;
		glm::vec3 color;
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
	std::vector<UserVertex> userVertices = {
		{{-0.5f, -0.5f, 0.f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
		{{0.5f, -0.5f, 0.f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f, 0.f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
	};
	// Index data for the square (two triangles).
	std::vector<uint16_t> userIndices = {
		0, 1, 2, 2, 3, 0
	};

	// Method to update uniform buffers each frame. Makes use of time since application start.
	UniformBufferObject UpdateUniforms();
};