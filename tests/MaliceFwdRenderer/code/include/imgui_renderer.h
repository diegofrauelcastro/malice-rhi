#include <iostream>

// Forward declarations
class IMaliceToImGuiBridge;
class ICommandBuffers;
class ISwapChain;
class IRenderPass;
class IDevice;
class Vulkan_GLFW_ImGuiRenderer;
struct GLFWwindow;

class IImGuiRenderer
{
protected:
	// Class properties

	IMaliceToImGuiBridge* bridge = nullptr;


public:
	// Constructor & destructor

	IImGuiRenderer() = default;
	virtual ~IImGuiRenderer() = default;


	// Lifetime methods

	virtual void Create(IMaliceToImGuiBridge* _bridge, GLFWwindow* _window) = 0;
	virtual void Destroy() = 0;


	// Retrieving the backend

	virtual Vulkan_GLFW_ImGuiRenderer& GetGLFW_Vulkan_ImGuiRenderer() { throw std::runtime_error("/!\\ This object is not an instance of Vulkan_GLFW_ImGuiRenderer!"); }

	
	// Class specific methods

	virtual void RecordOffscreen(ICommandBuffers* _cmd) = 0;
	virtual void RecordNewFrame() = 0;
	virtual void ShowDemoWindow() = 0;
	virtual void RenderFrame(ICommandBuffers* _cmd) = 0;
};