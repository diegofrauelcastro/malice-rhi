#include "vulkan_descriptorsetsgroup.h"
#include "vulkan_device.h"
#include "vulkan_pipeline.h"
#include "vulkan_swapchain.h"

void VulkanDescriptorSetsGroup::CreateDescriptorPoolAndSets(VulkanDevice& _device, VulkanPipeline& _pipeline, VulkanSwapChain& _swapChain)
{
    LOG_CLEAN("\n\n===== DESCRIPTOR POOL CREATION =====\n")

        const std::vector<VkDescriptorSetLayout> setLayouts = _pipeline.GetDescriptorSetLayoutsVkHandles();
    const uint32_t setCount = (uint32_t)(setLayouts.size());

    // Ensure that there is at least one descriptor set layout.
    if (setCount == 0)
        LOG_THROW("Pipeline has no descriptor set layouts.")
    else
        LOG_RHI("Creating descriptor pool...")

        // Get binding descriptions per set from the pipeline to adjust the size.
        std::vector<VkDescriptorPoolSize> poolSizes;
    for (const std::vector<VkDescriptorSetLayoutBinding>& setBinding : _pipeline.API_Vulkan().GetDescriptorSetLayoutBindingsPerSet())
    {
        for (const VkDescriptorSetLayoutBinding& binding : setBinding)
        {
            VkDescriptorPoolSize size{};
            size.type = binding.descriptorType;
            size.descriptorCount = binding.descriptorCount;
            poolSizes.push_back(size);
        }
    }

    // Create descriptor pool
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = setCount * _swapChain.GetMaxFramesInFlight();

    // Create the descriptor pool and ensure it succeeded.
    VkResult result = vkCreateDescriptorPool(_device.GetLogicalDeviceVkHandle(), &poolInfo, nullptr, &descriptorPool);
    if (result != VK_SUCCESS)
        LOG_THROW("/!\\ Failed to create descriptor pool!")
    else
        LOG_RHI("Descriptor pool created successfully.")
        LOG_CLEAN("")

        // Allocate descriptor sets for each frame in flight.
        descriptorSets.resize(_swapChain.GetMaxFramesInFlight());
    for (uint32_t i = 0; i < _swapChain.GetMaxFramesInFlight(); i++)
    {
        descriptorSets[i].resize(setCount);

        VkDescriptorSetAllocateInfo alloc{};
        alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc.descriptorPool = descriptorPool;
        alloc.descriptorSetCount = setCount;
        alloc.pSetLayouts = setLayouts.data();

        result = vkAllocateDescriptorSets(_device.GetLogicalDeviceVkHandle(), &alloc, descriptorSets[i].data());
        if (result != VK_SUCCESS)
            LOG_THROW("/!\\ Failed to allocate descriptor sets for frame %d!", (int)i)
        else
            LOG_RHI("Descriptor sets allocated successfully for frame %d.", (int)i)
    }
}

void VulkanDescriptorSetsGroup::Create(IDevice* _device, IPipeline* _pipeline, ISwapChain* _swapChain)
{
    // Create descriptor pool and sets.
    CreateDescriptorPoolAndSets(_device->API_Vulkan(), _pipeline->API_Vulkan(), _swapChain->API_Vulkan());
}

void VulkanDescriptorSetsGroup::Destroy(IDevice* _device)
{
    LOG_CLEAN("\n\n===== DESCRIPTOR POOL DESTRUCTION =====\n")

        // Destroy descriptor pool.
        if (descriptorPool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(_device->API_Vulkan().GetLogicalDeviceVkHandle(), descriptorPool, nullptr);
            LOG_RHI("Descriptor pool destroyed successfully.")
        }
        else
            LOG_RHI("Something went wrong trying to destroy a descriptor pool...")

            // Clear descriptor sets.
            descriptorSets.clear();
    descriptorSets.shrink_to_fit();
}