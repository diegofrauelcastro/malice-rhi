#pragma once

#include "Interface/malicerhi_masterheader.h"
#include "Interface/idescriptorsetsgroup.h"

// Forward declarations
class VulkanDevice;
class VulkanPipeline;
class VulkanSwapChain;

// Vulkan descriptor pool/sets class 
class VulkanDescriptorSetsGroup : public IDescriptorSetsGroup
{
private:
	/// Class properties ///

	VkDescriptorPool descriptorPool;
	std::vector<std::vector<VkDescriptorSet>> descriptorSets;


	/// Helper functions ///

	// Create descriptor pool and descriptor sets.
	void CreateDescriptorPoolAndSets(VulkanDevice& _device, VulkanPipeline& _pipeline, VulkanSwapChain* _swapChain);

public:
	// Class destructor
	virtual ~VulkanDescriptorSetsGroup() override = default;


	/// Lifetime methods ///

	void Create(IDevice* _device, IPipeline* _pipeline, ISwapChain* _swapChain) override;
	void Create(IDevice* _device, IPipeline* _pipeline) override;
	void Destroy(IDevice* _device) override;


	/// Retrieving the backend ///

	VulkanDescriptorSetsGroup& API_Vulkan() override { return *this; }
	VkDescriptorPool GetVkHandle() const { return descriptorPool; }
	std::vector<std::vector<VkDescriptorSet>> GetDescriptorSetsVkHandles() const { return descriptorSets; }
};