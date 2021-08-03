#include "CommandBuffer.h"
#include <stdexcept>

void Graphics::CommandBuffer::BuildCommandBuffer(shared_ptr<RenderPass> renderpass_ptr, shared_ptr<Pipeline> pipeline_ptr, shared_ptr<Buffer> vertexBuffer_ptr, shared_ptr<DescriptorSet> desc_ptr)
{

    for (size_t i = 0; i < drawCmdBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(drawCmdBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderpass_ptr->renderpass.render_pass;
        renderPassInfo.framebuffer = renderpass_ptr->renderpass.framebuffers[i];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = vulkan_ptr->swapchain.extent;


        renderPassInfo.clearValueCount = static_cast<uint32_t>(renderpass_ptr->clearValues.size());
        renderPassInfo.pClearValues = renderpass_ptr->clearValues.data();

        vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_ptr->pipeline.pipeline);

        VkBuffer vertexBuffers[] = { vertexBuffer_ptr->buffer.buffers->buffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(drawCmdBuffers[i], 0, 1, vertexBuffers, offsets);

        //vkCmdBindIndexBuffer(drawCmdBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        //vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

        vkCmdDraw(drawCmdBuffers[i], vertexBuffer_ptr->elem_count, 1, 0, 0);
        vkCmdEndRenderPass(drawCmdBuffers[i]);

        if (vkEndCommandBuffer(drawCmdBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}
