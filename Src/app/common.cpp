#include "common.h"


void vk::Image::SetImageLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout, uint32_t mipLevels) {

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = currentLayout;
    barrier.newLayout = newLayout;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layerCount;

    VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    switch (currentLayout) {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        barrier.srcAccessMask = 0;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;

    default:
        break;
    }

    switch (newLayout) {

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
        break;

    default:
        break;
    }


    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage,
        destinationStage,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier
    );

    currentLayout = newLayout;

}
