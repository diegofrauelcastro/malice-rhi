#include "vulkan_commandbuffers.h"

#include "vulkan_device.h"
#include "vulkan_commandpool.h"
#include "vulkan_swapchain.h"
#include "vulkan_renderpass.h"
#include "vulkan_framebuffers.h"
#include "vulkan_pipeline.h"
#include "vulkan_buffer.h"

void VulkanCommandBuffers::CreateCommandBuffers(VulkanDevice& _device, VulkanCommandPool& _commandPool, VulkanSwapChain& _swapChain)
{
	commandBuffers.resize(_swapChain.GetMaxFramesInFlight());

	// Allocate for one primary command buffer alone.
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _commandPool.GetVkHandle();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	// Create the command buffer and ensure it succeeded.
	VkResult result = vkAllocateCommandBuffers(_device.GetLogicalDeviceVkHandle(), &allocInfo, commandBuffers.data());
	if (result != VK_SUCCESS)
		throw std::runtime_error("/!\\ Failed to allocate command buffers!");
}

void VulkanCommandBuffers::BeginDraw(IRenderPass* _renderPass, ISwapChain* _swapChain, IFramebuffers* _framebuffers, uint32_t& _imageIndex)
{
	// Reset the current command buffer.
	vkResetCommandBuffer(commandBuffers[currentFrame], 0);
	// Begin recording the command buffer.
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;					// Optional
	beginInfo.pInheritanceInfo = nullptr;	// Optional

	// Begin the command buffer recording and ensure it succeeded.
	VkResult resultBeginCommandBuffer = vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo);
	if (resultBeginCommandBuffer != VK_SUCCESS)
		throw std::runtime_error("/!\\ Failed to begin recording command buffer!");

	// Info about the render pass.
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = _renderPass->API_Vulkan().GetVkHandle();
	renderPassInfo.framebuffer = _framebuffers->API_Vulkan().GetVkHandles()[_imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = _swapChain->API_Vulkan().GetImageExtent();
	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };	// Color of the "background" (the color we fill the image before rendering).
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	// Bind the render pass.
	vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Setup viewport values (required because we set it as dynamic).
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(_swapChain->API_Vulkan().GetImageExtent().width);
	viewport.height = static_cast<float>(_swapChain->API_Vulkan().GetImageExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

	// Setup scissor values (required because we set it as dynamic).
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = _swapChain->API_Vulkan().GetImageExtent();
	vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);
}

void VulkanCommandBuffers::EndDraw()
{
	// End render pass.
	vkCmdEndRenderPass(commandBuffers[currentFrame]);

	// Finish recording the command buffer.
	VkResult resultEndCommandBuffer = vkEndCommandBuffer(commandBuffers[currentFrame]);
	if (resultEndCommandBuffer != VK_SUCCESS)
		throw std::runtime_error("/!\\ Failed to record command buffer!");
}

void VulkanCommandBuffers::SubmitAndPresent(IDevice* _device, ISwapChain* _swapChain, uint32_t& _imageIndex)
{
	// Update the uniform buffer for the current frame.
	//UpdateUniformBuffer(currentFrame);		// TODO Adapt for uniform buffers

	// Submit the commands inside the command buffer.
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = { _swapChain->API_Vulkan().GetImageAvailableSemaphoresVkHandles()[currentFrame] }; // Which semaphores to wait on before execution of the command buffer.
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	// Which command buffers to submit for execution.
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
	// Specify the semaphore to signal once the command buffer has finished execution.
	VkSemaphore signalSemaphores[] = { _swapChain->API_Vulkan().GetRenderFinishedSemaphoresVkHandles()[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	// Submit to the graphics queue and ensure it succeeded.
	VkResult result = vkQueueSubmit(_device->API_Vulkan().GetGraphicsQueueVkHandle(), 1, &submitInfo, _swapChain->API_Vulkan().GetInFlightFencesVkHandles()[currentFrame]);
	if (result != VK_SUCCESS)
		throw std::runtime_error("/!\\ Failed to submit draw command buffer!");

	// Present the rendered image to the screen.
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	// Specify the semaphore to wait on before presentation.
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	// Specify the swap chain and image index to present.
	VkSwapchainKHR swapChains[] = { _swapChain->API_Vulkan().GetSwapChainVkHandle() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &_imageIndex;
	presentInfo.pResults = nullptr; // Optional
	// Present the image to the screen.
	result = vkQueuePresentKHR(_device->API_Vulkan().GetPresentQueueVkHandle(), &presentInfo);

	// Handle swap chain recreation if it's out of date (when window resized for example).
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		//_swapChain->API_Vulkan().RecreateSwapChain();
		throw std::runtime_error("/!\\ Swap chain is obsolete/suboptimal!");
	else if (result != VK_SUCCESS)
		throw std::runtime_error("/!\\ Failed to present swap chain image!");

	// Advance to the next frame.
	currentFrame = (currentFrame + 1) % _swapChain->GetMaxFramesInFlight();

	// This removes the validation layers about the semaphores being unsignaled (TODO ???)
	_device->WaitIdle();
}

void VulkanCommandBuffers::BindPipeline(IPipeline* _pipeline)
{
	// Bind the graphics pipeline.
	vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline->API_Vulkan().GetVkHandle());
}

void VulkanCommandBuffers::DrawVerticesByIndices(uint32_t _vertexNumber, IBuffer* _vertexBuffer, IBuffer* _indexBuffer)
{
	// Bind the vertex buffer.
	VkBuffer vertexBuffers[] = { _vertexBuffer->API_Vulkan().GetVkHandle() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, vertexBuffers, offsets);
	// Bind the index buffer.
	vkCmdBindIndexBuffer(commandBuffers[currentFrame], _indexBuffer->API_Vulkan().GetVkHandle(), 0, VK_INDEX_TYPE_UINT16);
	// Bind the descriptor set for the current frame.
	//vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr); // TODO Adapt for uniform buffer
	// Draw command.
	vkCmdDrawIndexed(commandBuffers[currentFrame], _vertexNumber, 1, 0, 0, 0);
}

void VulkanCommandBuffers::Create(IDevice* _device, ICommandPool* _commandPool, ISwapChain* _swapChain)
{
	CreateCommandBuffers(_device->API_Vulkan(), _commandPool->API_Vulkan(), _swapChain->API_Vulkan());
}

void VulkanCommandBuffers::Destroy(IDevice* _device)
{

}
