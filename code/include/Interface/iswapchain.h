#pragma once

#include <iostream>
#include <functional>


// Forward declarations
class VulkanSwapChain;
class IDevice;
class ISurface;
struct GLFWwindow;

// Swap chain interface
class ISwapChain
{
protected:
	/// Class properties ///

	// Maximum number of frames that can be modified in parallel during execution of the application with this swap chain.
	uint32_t maxFramesInFlight = 1;

	// This callback will execute when the target surface gets resized. This will allow the framebuffers linked to this swapchain to be resized too.
	std::function<void(IDevice*, ISwapChain*)> framebufferResizeCallback;

	// Flag marked as true when the framebuffer is resizing.
	bool bResizeRequested = false;

public:
	// Class destructor
	virtual ~ISwapChain() = default;


	/// Lifetime methods ///

	virtual void Create(IDevice* _device, ISurface* _surface, GLFWwindow* _window) = 0;
	virtual void Destroy(IDevice* _device) = 0;


	/// Retrieving the backend ///

	virtual VulkanSwapChain& API_Vulkan() { throw std::runtime_error("Bad API call : object is not a VulkanSwapChain."); }


	/// Class specific methods ///

	// Returns the maximum number of frames that can be modified in parallel during execution of the application.
	uint32_t GetMaxFramesInFlight() const { return maxFramesInFlight; }

	// Gets the next presentable image in the swap chain. Returns i, the index of the retrieved image.
	virtual bool AcquireNextImage(IDevice* _device, uint32_t _currentFrameIndex, uint32_t* _nextImageIndex) = 0;

	// Bind a method to the resize callback.
	void BindResizeCallback(std::function<void(IDevice*, ISwapChain*)> _callback) { framebufferResizeCallback = _callback; }
};