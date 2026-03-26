#pragma once

#include <iostream>


// Forward declarations.
class IInstance;
class IDevice;
class ISwapChain;
class IRenderPass;
class IFramebuffers;
class VulkanMaliceToImGuiBridge;

class IMaliceToImGuiBridge
{
protected:
	/// Class properties ///

	IInstance* storedInstance = nullptr;		// Exceptionnally store the instance used to create this object, as it will be needed later by ImGui to be initialized correctly.
	IDevice* storedDevice = nullptr;			// For the same reason, expectionnally store the device.
	ISwapChain* storedSwapChain = nullptr;		// Same here.
	IRenderPass* storedRenderPass = nullptr;	// Same here.
	IFramebuffers* storedFramebuffers = nullptr;// Same here.

public:
	// Class destructor
	virtual ~IMaliceToImGuiBridge() = default;


	/// Lifetime methods ///

	virtual void Create(IInstance* _instance, IDevice* _device, ISwapChain* _swapChain, IRenderPass* _renderPass, IFramebuffers* _framebuffers) = 0;
	virtual void Destroy(IDevice* _device) = 0;


	/// Retrieving the backend ///

	virtual VulkanMaliceToImGuiBridge& API_Vulkan() { throw std::runtime_error("Bad API call : object is not a VulkanMaliceToImGuiBridge."); }
	IInstance* GetLinkedInstance() const { return storedInstance; }
	IDevice* GetLinkedDevice() const { return storedDevice; }
	ISwapChain* GetLinkedSwapChain() const { return storedSwapChain; }
	IRenderPass* GetLinkedRenderPass() const { return storedRenderPass; }
	IFramebuffers* GetLinkedFramebuffers() const { return storedFramebuffers; }
};