#pragma once

#include "imgui_renderer.h"
#include "imgui.h"

// Forward declarations
typedef struct VkDescriptorSet_T* VkDescriptorSet;

class Vulkan_GLFW_ImGuiRenderer : public IImGuiRenderer
{
protected:
	// Class properties

	GLFWwindow* window = nullptr;
	VkDescriptorSet imguiSet = nullptr;

public:
	// Constructor & destructor

	Vulkan_GLFW_ImGuiRenderer() = default;
	virtual ~Vulkan_GLFW_ImGuiRenderer() = default;


	// Lifetime methods

	virtual void Create(IMaliceToImGuiBridge* _bridge, GLFWwindow* _window) override;
	virtual void Destroy() override ;


	// Retrieving the backend

	virtual Vulkan_GLFW_ImGuiRenderer& GetGLFW_Vulkan_ImGuiRenderer() override { return *this; }

	
	// Class specific methods

	virtual void RecordNewFrame() override;
	virtual void ShowOffscreenRenderInWindow() override;
	virtual void ShowDemoWindow() override;
	virtual void Render() override;
	virtual void DrawImGuiData(ICommandBuffers* _cmd) override;
};