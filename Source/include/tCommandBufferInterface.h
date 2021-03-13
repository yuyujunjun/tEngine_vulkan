#pragma once
#include"CommandBufferBase.h"
#include"tEngineFunctional.h"
namespace tEngine {
	void setImageLayout(CommandBuffer::SharedPtr const& commandBuffer,
		tImage::SharedPtr                 image,
		vk::ImageLayout           oldImageLayout,
		vk::ImageLayout           newImageLayout)
	{
		vk::AccessFlags sourceAccessMask;
		switch (oldImageLayout)
		{
		case vk::ImageLayout::eTransferDstOptimal: sourceAccessMask = vk::AccessFlagBits::eTransferWrite; break;
		case vk::ImageLayout::ePreinitialized: sourceAccessMask = vk::AccessFlagBits::eHostWrite; break;
		case vk::ImageLayout::eGeneral:  // sourceAccessMask is empty
		case vk::ImageLayout::eUndefined: break;
		default: assert(false); break;
		}

		vk::PipelineStageFlags sourceStage;
		switch (oldImageLayout)
		{
		case vk::ImageLayout::eGeneral:
		case vk::ImageLayout::ePreinitialized: sourceStage = vk::PipelineStageFlagBits::eHost; break;
		case vk::ImageLayout::eTransferDstOptimal: sourceStage = vk::PipelineStageFlagBits::eTransfer; break;
		case vk::ImageLayout::eUndefined: sourceStage = vk::PipelineStageFlagBits::eTopOfPipe; break;
		default: assert(false); break;
		}

		vk::AccessFlags destinationAccessMask;
		switch (newImageLayout)
		{
		case vk::ImageLayout::eColorAttachmentOptimal:
			destinationAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			break;
		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			destinationAccessMask =
				vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			break;
		case vk::ImageLayout::eGeneral:  // empty destinationAccessMask
		case vk::ImageLayout::ePresentSrcKHR: break;
		case vk::ImageLayout::eShaderReadOnlyOptimal: destinationAccessMask = vk::AccessFlagBits::eShaderRead; break;
		case vk::ImageLayout::eTransferSrcOptimal: destinationAccessMask = vk::AccessFlagBits::eTransferRead; break;
		case vk::ImageLayout::eTransferDstOptimal: destinationAccessMask = vk::AccessFlagBits::eTransferWrite; break;
		default: assert(false); break;
		}

		vk::PipelineStageFlags destinationStage;
		switch (newImageLayout)
		{
		case vk::ImageLayout::eColorAttachmentOptimal:
			destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			break;
		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
			break;
		case vk::ImageLayout::eGeneral: destinationStage = vk::PipelineStageFlagBits::eHost; break;
		case vk::ImageLayout::ePresentSrcKHR: destinationStage = vk::PipelineStageFlagBits::eBottomOfPipe; break;
		case vk::ImageLayout::eShaderReadOnlyOptimal:
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
			break;
		case vk::ImageLayout::eTransferDstOptimal:
		case vk::ImageLayout::eTransferSrcOptimal: destinationStage = vk::PipelineStageFlagBits::eTransfer; break;
		default: assert(false); break;
		}

		vk::ImageAspectFlags aspectMask = (vk::ImageAspectFlags)format_to_aspect_mask((VkFormat)image->get_format());

		vk::ImageSubresourceRange imageSubresourceRange(aspectMask, 0, image->get_create_info().levels, 0, image->get_create_info().layers);

		vk::ImageMemoryBarrier imageBarrier;
		//	image->setImageLayout(newImageLayout);
		imageBarrier.setImage(image->getVkHandle());
		imageBarrier.setDstAccessMask(destinationAccessMask);
		imageBarrier.setSrcAccessMask(sourceAccessMask);
		imageBarrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
		imageBarrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
		imageBarrier.setOldLayout(oldImageLayout);
		imageBarrier.setNewLayout(newImageLayout);
		imageBarrier.setSubresourceRange(imageSubresourceRange);

		MemoryBarrierSet barrier;
		barrier.addBarrier(imageBarrier);
		commandBuffer->pipelineBarrier(sourceStage, destinationStage, {}, barrier);
	}
	void barrier_prepare_generate_mipmap(CommandBuffer::SharedPtr const& cb, const tImage::SharedPtr& image, vk::ImageLayout base_level_layout,
		vk::PipelineStageFlags src_stage, vk::AccessFlags src_access,
		bool need_top_level_barrier)
	{
		auto& create_info = image->get_create_info();
		vk::ImageMemoryBarrier barriers[2] = {};
		assert(create_info.layers > 1);
		(void)create_info;

		for (unsigned i = 0; i < 2; i++)
		{

			barriers[i].image = image->getVkHandle();
			barriers[i].subresourceRange.aspectMask = (vk::ImageAspectFlagBits)format_to_aspect_mask(image->get_format());
			barriers[i].subresourceRange.layerCount = create_info.layers;
			barriers[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			if (i == 0)
			{
				barriers[i].oldLayout = base_level_layout;
				barriers[i].newLayout = vk::ImageLayout::eTransferSrcOptimal;
				barriers[i].srcAccessMask = src_access;
				barriers[i].dstAccessMask = vk::AccessFlagBits::eTransferRead;
				barriers[i].subresourceRange.baseMipLevel = 0;
				barriers[i].subresourceRange.levelCount = 1;
			}
			else
			{
				barriers[i].oldLayout = vk::ImageLayout::eUndefined;
				barriers[i].newLayout = vk::ImageLayout::eTransferDstOptimal;
				barriers[i].srcAccessMask = vk::AccessFlagBits::eNoneKHR;
				barriers[i].dstAccessMask = vk::AccessFlagBits::eTransferWrite;
				barriers[i].subresourceRange.baseMipLevel = 1;
				barriers[i].subresourceRange.levelCount = create_info.levels - 1;
			}
		}
		tEngine::MemoryBarrierSet barrier;
		if (need_top_level_barrier) {
			barrier.addBarrier(barriers[0]);
		}
		barrier.addBarrier(barriers[1]);
		cb->pipelineBarrier(src_stage, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlagBits::eByRegion, barrier);

	}
	void generateMipmap(CommandBuffer::SharedPtr const& cb, tImage::SharedPtr image) {
		using  std::max;
		auto& create_info = image->get_create_info();
		vk::Offset3D size = { int(create_info.width), int(create_info.height), int(create_info.depth) };
		const vk::Offset3D origin = { 0, 0, 0 };

		//assert(image->getImageLayout() == vk::ImageLayout::eTransferSrcOptimal);


		vk::ImageMemoryBarrier b;
		//	VkImageMemoryBarrier b = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		b.setImage(image->getVkHandle());
		b.subresourceRange.levelCount = 1;
		b.subresourceRange.layerCount = image->get_create_info().layers;
		b.subresourceRange.aspectMask = (vk::ImageAspectFlagBits)format_to_aspect_mask(image->get_format());// get_format()); image->getCreateInfo().format
		b.oldLayout = (vk::ImageLayout)VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		b.newLayout = (vk::ImageLayout)VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		b.srcAccessMask = (vk::AccessFlags)VK_ACCESS_TRANSFER_WRITE_BIT;
		b.dstAccessMask = (vk::AccessFlags)VK_ACCESS_TRANSFER_READ_BIT;
		b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;



		for (unsigned i = 1; i < create_info.levels; i++)
		{
			vk::Offset3D src_size = size;
			size.x = max(size.x >> 1, 1);
			size.y = max(size.y >> 1, 1);
			size.z = max(size.z >> 1, 1);

			cb->blitImage(image, image,
				origin, size, origin, src_size, i, i - 1, 0, 0, create_info.layers, vk::Filter::eLinear);

			b.subresourceRange.baseMipLevel = i;
			MemoryBarrierSet barrier;
			barrier.addBarrier(b);
			cb->pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlagBits::eByRegion, barrier);
		}
	}
}