#include "imgui_renderer.h"
#include "imgui.h"

// Forward declarations
struct Offscreen;

class Vulkan_GLFW_ImGuiRenderer : public IImGuiRenderer
{
protected:
	// Class properties

	Offscreen* off = nullptr;
	GLFWwindow* window = nullptr;


	// Helper methods

	static void CreateOffscreen(IDevice* _device, ISwapChain* _swapChain, Offscreen& _out);
	static void DestroyOffscreen(IDevice* _device, Offscreen& _o);
	static uint32_t FindMemoryType(IDevice* _device, uint32_t _typeBits);
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

	virtual void RecordOffscreen(ICommandBuffers* _cmd) override;
	virtual void RecordNewFrame() override;
	virtual void ShowDemoWindow() override;
	virtual void RenderFrame(ICommandBuffers* _cmd) override;
};