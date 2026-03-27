#pragma once

#include <iostream>


// Forward declarations.
class IInstance;
class IDevice;
class ISwapChain;
class ITexture;
class IRenderPass;
class IFramebuffers;
class VulkanMaliceToImGuiBridge;

// Offscreen render target (color-only) to show inside ImGui
struct Offscreen
{
	ITexture* colorTex = nullptr;
	IRenderPass* renderPass = nullptr;
	IFramebuffers* framebuffers = nullptr;
};

class IMaliceToImGuiBridge
{
protected:
	/// Class properties ///

	IInstance* storedInstance = nullptr;		// Exceptionnally store the instance used to create this object, as it will be needed later by ImGui to be initialized correctly.
	IDevice* storedDevice = nullptr;			// For the same reason, expectionnally store the device.
	ISwapChain* storedScreenSwapChain = nullptr;		// Same here.
	IRenderPass* storedScreenRenderPass = nullptr;	// Same here.
	IFramebuffers* storedScreenFramebuffers = nullptr;// Same here.
	Offscreen offscreenParams;

public:
	// Class destructor
	virtual ~IMaliceToImGuiBridge() = default;


	/// Lifetime methods ///

	virtual void Create(IInstance* _instance, IDevice* _device, ISwapChain* _screenSwapChain, IRenderPass* _screenRenderPass, IFramebuffers* _screenFramebuffers, Offscreen _offscreenParams) = 0;
	virtual void Destroy(IDevice* _device) = 0;


	/// Retrieving the backend ///

	virtual VulkanMaliceToImGuiBridge& API_Vulkan() { throw std::runtime_error("Bad API call : object is not a VulkanMaliceToImGuiBridge."); }
	IInstance* GetLinkedInstance() const { return storedInstance; }
	IDevice* GetLinkedDevice() const { return storedDevice; }
	ISwapChain* GetLinkedSwapChain() const { return storedScreenSwapChain; }
	IRenderPass* GetLinkedRenderPass() const { return storedScreenRenderPass; }
	IFramebuffers* GetLinkedFramebuffers() const { return storedScreenFramebuffers; }
	Offscreen GetOffscreenParams() const { return offscreenParams; }
};