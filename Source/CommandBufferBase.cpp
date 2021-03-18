#include"CommandBufferBase.h"
namespace tEngine {
	CommandBufferHandle allocateCommandBuffer(weakDevice device, CommandPoolHandle cmdPool) {
		vk::CommandBufferAllocateInfo info = {};
		info.setCommandBufferCount(1);
		info.setCommandPool(cmdPool->vkCommandPool);
		info.setLevel(vk::CommandBufferLevel::ePrimary);
		auto vkcb = device->allocateCommandBuffers(info);
		return std::make_shared<CommandBuffer>(device, vkcb.front(), cmdPool);
	}
#ifdef DEBUG
	void CommandBuffer::updatePerSubpassImageLayouts()
	{
		using AttachmentReference = vk::AttachmentReference;
		auto& currentRenderPass = _currentlyBoundFramebuffer->renderPass;

		for (uint8_t i = 0; i < currentRenderPass->subpass[_currentSubpass].input.size(); ++i)
		{
			const vk::AttachmentReference& attachmentReference = currentRenderPass->subpass[_currentSubpass].input[i];
			_currentlyBoundFramebuffer->frameBufferImages[attachmentReference.attachment]->setImageLayout(attachmentReference.layout);
		}

		for (uint8_t i = 0; i < currentRenderPass->subpass[_currentSubpass].color.size(); ++i)
		{
			const AttachmentReference& attachmentReference = currentRenderPass->subpass[_currentSubpass].color[i];
			_currentlyBoundFramebuffer->frameBufferImages[attachmentReference.attachment]->setImageLayout(attachmentReference.layout);
		}

		for (uint8_t i = 0; i < currentRenderPass->subpass[_currentSubpass].resolved.size(); ++i)
		{
			const AttachmentReference& attachmentReference = currentRenderPass->subpass[_currentSubpass].resolved[i];
			_currentlyBoundFramebuffer->frameBufferImages[attachmentReference.attachment]->setImageLayout(attachmentReference.layout);
		}
	}
#endif

	void CommandBuffer::pipelineBarrier(PipelineStageFlags srcStage, PipelineStageFlags dstStage, DependencyFlags dependencyFlags, const MemoryBarrierSet& barriers)
	{
		cb.pipelineBarrier(srcStage, dstStage, dependencyFlags, barriers.getMemoryBarriers(), barriers.getBufferBarriers(), barriers.getImageBarriers());
	}

	void CommandBuffer::waitForEvent(const Event& event, PipelineStageFlags srcStage, PipelineStageFlags dstStage, const MemoryBarrierSet& barriers)
	{
		cb.waitEvents(event, srcStage, dstStage, barriers.getMemoryBarriers(), barriers.getBufferBarriers(), barriers.getImageBarriers());
	}

	void CommandBuffer::waitForEvents(const std::vector<Event> events, PipelineStageFlags srcStage, PipelineStageFlags dstStage, const MemoryBarrierSet& barriers)
	{

		cb.waitEvents(events, srcStage, dstStage, barriers.getMemoryBarriers(), barriers.getBufferBarriers(), barriers.getImageBarriers());
	}

	// bind pipelines, sets, vertex/index buffers




	void CommandBuffer::begin(const CommandBufferUsageFlags flags)
	{
		if (_isRecording) { assert(false && "Called CommandBuffer::begin while a recording was already in progress. Call CommandBuffer::end first"); }
		_isRecording = true;

		vk::CommandBufferBeginInfo info;
		info.flags = flags;
		vk::CommandBufferInheritanceInfo inheritanceInfo;// = {};
		info.pInheritanceInfo = &inheritanceInfo;
		cb.begin(info);


	}
	void CommandBuffer::bindDescriptorSet(vk::PipelineBindPoint bindingPoint, const tPipelineLayout::SharedPtr& pipelineLayout, uint32_t firstSet, const std::vector<DescriptorSetHandle>& sets,
		const std::vector<uint32_t>& dynamicOffsets)
	{
		_objectReferences.emplace_back(pipelineLayout);
		std::vector<vk::DescriptorSet> vkSets;
		vkSets.reserve(sets.size());
		for (const auto& set : sets) {
			bool check = false;
			for (const auto& buf : set->getResource().getBuffers()) {
				if (buf.hasUsed) {
					_objectReferences.emplace_back(buf.buffer);
					check = true;
				}
			}
			for (const auto& img : set->getResource().getImages()) {
				if (img.hasUsed) {
					_objectReferences.emplace_back(img.getImageResource());
					check = true;
				}
			}
			assert(check && "binded descriptorSet must have resource");
			vkSets.emplace_back(set->getVkHandle());
		}
		cb.bindDescriptorSets(bindingPoint, pipelineLayout->getVkHandle(), firstSet, vkSets, dynamicOffsets);
	}
	void CommandBuffer::bindVertexBuffer(const BufferHandle& buffer, vk::DeviceSize offset, uint16_t bindingIndex)
	{
		_objectReferences.emplace_back(buffer);

		cb.bindVertexBuffers(bindingIndex, { buffer->getVkHandle() }, { offset });

	}
	void CommandBuffer::bindVertexBuffers(const std::vector<BufferHandle>& buffers, uint32_t firstBinding, const std::vector<vk::DeviceSize>& offsets)
	{
		uint32_t bindingCount = static_cast<uint32_t>(buffers.size());
		std::vector<vk::Buffer>native_buffers(static_cast<uint32_t>(bindingCount));// = { VK_NULL_HANDLE };
		for (uint32_t i = 0; i < bindingCount; ++i)
		{
			_objectReferences.emplace_back(buffers[i]);
			native_buffers[i] = buffers[i]->getVkHandle();
		}
		cb.bindVertexBuffers(firstBinding, native_buffers, offsets);

	}
	void CommandBuffer::bindIndexBuffer(const BufferHandle& buffer, vk::DeviceSize offset, IndexType indexType)
	{
		_objectReferences.emplace_back(buffer);
		cb.bindIndexBuffer(buffer->getVkHandle(), offset, indexType);

	}
	void CommandBuffer::begin(const RenderPassHandle renderPass, uint32_t subpass, const vk::CommandBufferUsageFlags flags)
	{
		if (_isRecording)
		{
			throw ("Called CommandBuffer::begin while a recording was already"
				" in progress. Call CommandBuffer::end first");
		}
		_objectReferences.emplace_back(renderPass);
		_isRecording = true;
		vk::CommandBufferBeginInfo info;// = {};
		vk::CommandBufferInheritanceInfo inheritInfo;// = {};
		info.flags = flags;

		inheritInfo.renderPass = renderPass->getVkHandle();
		inheritInfo.subpass = subpass;
		inheritInfo.occlusionQueryEnable = VK_FALSE;
		info.pInheritanceInfo = &inheritInfo;
		cb.begin(info);
	}

	void CommandBuffer::end()
	{
		if (!_isRecording)
		{
			throw ("Called CommandBuffer::end while a recording "
				"was not in progress. Call CommandBuffer::begin first");
		}
		_isRecording = false;
		cb.end();
	}

	void CommandBuffer::executeCommands(const CommandBufferHandle& secondaryCmdBuffer)
	{
		if (!secondaryCmdBuffer) { throw ("Secondary command buffer was NULL for ExecuteCommands"); }
		_objectReferences.emplace_back(secondaryCmdBuffer);
		cb.executeCommands(secondaryCmdBuffer->getVkHandle());
	}

	void CommandBuffer::executeCommands(const std::vector<CommandBufferHandle>& secondaryCmdBuffers)
	{
		std::vector<vk::CommandBuffer> cmdBuffs(secondaryCmdBuffers.size());
		for (uint32_t i = 0; i < secondaryCmdBuffers.size(); ++i)
		{
			_objectReferences.emplace_back(secondaryCmdBuffers[i]);
			cmdBuffs[i] = secondaryCmdBuffers[i]->getVkHandle();
		}
		cb.executeCommands(cmdBuffs);
	}
	void CommandBuffer::beginRenderPass(const RenderPassHandle renderPass, const FrameBufferHandle frameBuffer, bool inlineFirstSubpass) {
		_objectReferences.emplace_back(renderPass);
		_objectReferences.emplace_back(frameBuffer);
		for (const auto& image : renderPass->getImages()) {
			_objectReferences.emplace_back(image);
		}
		std::vector<vk::ClearValue> clearValues(renderPass->getAttachmentCount());
		for (const auto& attachment : renderPass->getAttachments()) {
			clearValues[attachment.idx] = attachment.value;
		}
		vk::RenderPassBeginInfo nfo;// = {};
		nfo.setClearValues(clearValues);
		nfo.framebuffer = frameBuffer->getVkHandle();
		nfo.renderPass = renderPass->getVkHandle();// ->renderPass->vkRenderPass;
		nfo.renderArea = frameBuffer->getRenderArea();
		auto content = inlineFirstSubpass ? vk::SubpassContents::eInline : vk::SubpassContents::eSecondaryCommandBuffers;
		cb.beginRenderPass(nfo, content);
	}
	// RenderPasses, Subpasses

	void CommandBuffer::endRenderPass() {
		cb.endRenderPass();
	}


	// buffers, textures, images, push constants
	void CommandBuffer::updateBuffer(const BufferHandle& buffer, const void* data, uint32_t offset, uint32_t length)
	{
		_objectReferences.emplace_back(buffer);
		cb.updateBuffer(buffer->getVkHandle(), offset, length, data);

	}

	void CommandBuffer::pushConstants(const VkPipelineLayout layout, VkShaderStageFlags stage, uint32_t offset, uint32_t size, const void* data)
	{
		//	_objectReferences.emplace_back(pipelineLayout);
		cb.pushConstants(layout, (vk::ShaderStageFlags)stage, offset, size, data);
	}
	void CommandBuffer::pushConstants(const VkPipelineLayout layout, VkShaderStageFlags stage, std::vector<uint8_t> block) {
		pushConstants(layout, stage, 0, block.size(), block.data());
	}
	void CommandBuffer::resolveImage(const ImageHandle& srcImage, const ImageHandle& dstImage, const std::vector<ImageResolve>& regions, ImageLayout srcLayout, ImageLayout dstLayout)
	{
		_objectReferences.emplace_back(srcImage);
		_objectReferences.emplace_back(dstImage);
		assert(sizeof(ImageResolve) == sizeof(VkImageResolve));
		cb.resolveImage(srcImage->getVkHandle(), srcLayout, dstImage->getVkHandle(), dstLayout, regions);

	}

	void CommandBuffer::blitImage(const ImageHandle& srcImage, const ImageHandle& dstImage, const std::vector<vk::ImageBlit>& regions, Filter filter)
	{
		_objectReferences.emplace_back(srcImage);
		_objectReferences.emplace_back(dstImage);
		cb.blitImage(srcImage->getVkHandle(), vk::ImageLayout::eTransferSrcOptimal, dstImage->getVkHandle(), vk::ImageLayout::eTransferDstOptimal, regions, filter);

	}
	void CommandBuffer::blitImage(const ImageHandle& dst, const ImageHandle& src,
		const vk::Offset3D& dst_offset,
		const vk::Offset3D& dst_extent, const vk::Offset3D& src_offset, const vk::Offset3D& src_extent,
		unsigned dst_level, unsigned src_level, unsigned dst_base_layer, unsigned src_base_layer,
		unsigned num_layers, vk::Filter filter)
	{
		const auto add_offset = [](const vk::Offset3D& a, const vk::Offset3D& b) -> vk::Offset3D {
			return { a.x + b.x, a.y + b.y, a.z + b.z };
		};

		;
		const vk::ImageBlit blit = {
			{ (vk::ImageAspectFlags)format_to_aspect_mask(src->get_format()), src_level, src_base_layer, num_layers },
			{ src_offset, add_offset(src_offset, src_extent) },
			{ (vk::ImageAspectFlags)format_to_aspect_mask(dst->get_format()), dst_level, dst_base_layer, num_layers },
			{ dst_offset, add_offset(dst_offset, dst_extent) },
		};
		blitImage(src, dst, { blit }, filter);

	}
	void CommandBuffer::copyImage(const ImageHandle& srcImage, const ImageHandle& dstImage, ImageLayout srcImageLayout, ImageLayout dstImageLayout, const std::vector<ImageCopy>& regions)
	{
		_objectReferences.emplace_back(srcImage);
		_objectReferences.emplace_back(dstImage);
		// Try to avoid heap allocation
		cb.copyImage(srcImage->getVkHandle(), srcImageLayout, dstImage->getVkHandle(), dstImageLayout, regions);
	}

	void CommandBuffer::copyImageToBuffer(const ImageHandle& srcImage, ImageLayout srcImageLayout, BufferHandle& dstBuffer, vk::ArrayProxy<const vk::BufferImageCopy> const& regions)
	{
		_objectReferences.emplace_back(srcImage);
		_objectReferences.emplace_back(dstBuffer);

		cb.copyImageToBuffer(srcImage->getVkHandle(), srcImageLayout, dstBuffer->getVkHandle(), regions);

	}

	void CommandBuffer::copyBuffer(const BufferHandle& srcBuffer, const BufferHandle& dstBuffer, vk::ArrayProxy<const vk::BufferCopy>const& regions)
	{
		_objectReferences.emplace_back(srcBuffer);
		_objectReferences.emplace_back(dstBuffer);
		cb.copyBuffer(srcBuffer->getVkHandle(), dstBuffer->getVkHandle(), regions);
	}
	void CommandBuffer::copyBufferToImage(const BufferHandle& buffer, const ImageHandle& image, vk::ImageLayout dstImageLayout, const vk::ArrayProxy<const BufferImageCopy>& regions)
	{

		_objectReferences.emplace_back(buffer);
		_objectReferences.emplace_back(image);
		cb.copyBufferToImage(buffer->getVkHandle(), image->getVkHandle(), dstImageLayout, regions);
	}

	void CommandBuffer::fillBuffer(const BufferHandle& dstBuffer, uint32_t dstOffset, uint32_t data, uint64_t size)
	{
		_objectReferences.emplace_back(dstBuffer);
		cb.fillBuffer(dstBuffer->getVkHandle(), dstOffset, size, data);
	}

	// dynamic commands
	void CommandBuffer::setViewport(const Viewport& viewport) { cb.setViewport(0, viewport); }//; getDevice()->getVkBindings().vkCmdSetViewport(getDefaultView(), 0, 1, &viewport.get());


	void CommandBuffer::setScissor(uint32_t firstScissor, vk::ArrayProxy< const vk::Rect2D>const& scissors)
	{
		cb.setScissor(firstScissor, scissors);

	}

	void CommandBuffer::setDepthBounds(float min, float max) { cb.setDepthBounds(min, max); }

	void CommandBuffer::setStencilCompareMask(StencilFaceFlags face, uint32_t compareMask)
	{
		cb.setStencilCompareMask(face, compareMask);
	}

	void CommandBuffer::setStencilWriteMask(StencilFaceFlags face, uint32_t writeMask)
	{
		cb.setStencilWriteMask(face, writeMask);
	}

	void CommandBuffer::setStencilReference(StencilFaceFlags face, uint32_t ref)
	{
		cb.setStencilReference(face, ref);

	}

	void CommandBuffer::setDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
	{
		cb.setDepthBias(depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
	}

	void CommandBuffer::setBlendConstants(float rgba[4]) { cb.setBlendConstants(rgba); }

	void CommandBuffer::setLineWidth(float lineWidth) { cb.setLineWidth(lineWidth); }

	inline void clearcolorimage(vk::CommandBuffer cb, const ImageHandle& image, vk::ClearColorValue clearColor, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
		const vk::ArrayProxy<const uint32_t>& numLevels, const  vk::ArrayProxy<const uint32_t>& baseArrayLayers, const vk::ArrayProxy<const uint32_t>& numLayers, vk::ImageLayout layout)
	{
		assert(layout == vk::ImageLayout::eGeneral || layout == vk::ImageLayout::eTransferDstOptimal);
		uint32_t numRanges = baseMipLevel.size();
		assert(numRanges <= 10);

		std::vector<vk::ImageSubresourceRange> subResourceRanges(numLevels.size());


		for (uint32_t i = 0; i < numRanges; ++i)
		{
			subResourceRanges[i].aspectMask = vk::ImageAspectFlagBits::eColor;
			subResourceRanges[i].baseMipLevel = reinterpret_cast<const uint32_t*>(baseMipLevel.data())[i];
			subResourceRanges[i].levelCount = reinterpret_cast<const uint32_t*>(numLevels.data())[i];
			subResourceRanges[i].baseArrayLayer = reinterpret_cast<const uint32_t*>(baseArrayLayers.data())[i];
			subResourceRanges[i].layerCount = reinterpret_cast<const uint32_t*>(numLayers.data())[i];
		}

		cb.clearColorImage(image->getVkHandle(), layout, clearColor, subResourceRanges);
	}


	void CommandBuffer::clearColorImage(vk::CommandBuffer cb, const ImageHandle& image, ClearColorValue clearColor, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
		const vk::ArrayProxy<const uint32_t>& numLevels, const  vk::ArrayProxy<const uint32_t>& baseArrayLayers, const vk::ArrayProxy<const uint32_t>& numLayers, ImageLayout layout)
	{
		_objectReferences.emplace_back(image);
		assert(layout == ImageLayout::eGeneral || layout == ImageLayout::eTransferDstOptimal);
		uint32_t numRanges = baseMipLevel.size();
		assert(numRanges <= 10);

		std::vector<vk::ImageSubresourceRange> subResourceRanges(numLevels.size());


		for (uint32_t i = 0; i < numRanges; ++i)
		{
			subResourceRanges[i].aspectMask = ImageAspectFlagBits::eColor;
			subResourceRanges[i].baseMipLevel = reinterpret_cast<const uint32_t*>(baseMipLevel.data())[i];
			subResourceRanges[i].levelCount = reinterpret_cast<const uint32_t*>(numLevels.data())[i];
			subResourceRanges[i].baseArrayLayer = reinterpret_cast<const uint32_t*>(baseArrayLayers.data())[i];
			subResourceRanges[i].layerCount = reinterpret_cast<const uint32_t*>(numLayers.data())[i];
		}

		cb.clearColorImage(image->getVkHandle(), layout, clearColor, subResourceRanges);
	}
	inline static void clearDepthStencilImageHelper(vk::CommandBuffer cb, const ImageHandle& image, vk::ImageLayout layout, vk::ImageAspectFlags imageAspect,
		float clearDepth, uint32_t clearStencil, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
		const vk::ArrayProxy<const uint32_t>& numLevels, const  vk::ArrayProxy<const uint32_t>& baseArrayLayers, const vk::ArrayProxy<const uint32_t>& numLayers)
	{
		using ImageLayout = vk::ImageLayout;
		if (!(layout == ImageLayout::eGeneral || layout == ImageLayout::eTransferDstOptimal))
		{
			throw ("Cannot clear depth stencil image: It is in neither e_GENERAL or e_TRANSFER_DST_OPTIMAL layout");
		}

		vk::ClearDepthStencilValue clearDepthStencilValue;
		clearDepthStencilValue.depth = clearDepth;
		clearDepthStencilValue.stencil = clearStencil;
		auto numRanges = baseMipLevel.size();
		std::vector<vk::ImageSubresourceRange> subResourceRanges(numRanges);

		for (uint32_t i = 0; i < numRanges; ++i)
		{
			subResourceRanges[i].aspectMask = (imageAspect);
			subResourceRanges[i].baseMipLevel = reinterpret_cast<const uint32_t*>(baseMipLevel.data())[i];
			subResourceRanges[i].levelCount = reinterpret_cast<const uint32_t*>(numLevels.data())[i];
			subResourceRanges[i].baseArrayLayer = reinterpret_cast<const uint32_t*>(baseArrayLayers.data())[i];
			subResourceRanges[i].layerCount = reinterpret_cast<const uint32_t*>(numLayers.data())[i];
		}
		cb.clearDepthStencilImage(image->getVkHandle(), layout, clearDepthStencilValue, subResourceRanges);

	}

	void CommandBuffer::clearDepthImage(
		const ImageHandle& image, float clearDepth, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
		const vk::ArrayProxy<const uint32_t>& numLevels, const  vk::ArrayProxy<const uint32_t>& baseArrayLayers, const vk::ArrayProxy<const uint32_t>& numLayers, ImageLayout layout)
	{
		_objectReferences.emplace_back(image);
		clearDepthStencilImageHelper(cb, image, layout, ImageAspectFlagBits::eDepth, clearDepth, 0u, baseMipLevel, numLevels, baseArrayLayers, numLayers);
	}



	void CommandBuffer::clearStencilImage(
		const ImageHandle& image, uint32_t clearStencil, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
		const vk::ArrayProxy<const uint32_t>& numLevels, const  vk::ArrayProxy<const uint32_t>& baseArrayLayers,
		const vk::ArrayProxy<const uint32_t>& numLayers, ImageLayout layout)
	{
		_objectReferences.emplace_back(image);
		clearDepthStencilImageHelper(
			cb, image, layout, ImageAspectFlagBits::eStencil, 0.0f, clearStencil, baseMipLevel, numLevels, baseArrayLayers, numLayers);
	}



	void CommandBuffer::clearDepthStencilImage(const ImageHandle& image, float clearDepth, uint32_t clearStencil, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
		const vk::ArrayProxy<const uint32_t>& numLevels, const  vk::ArrayProxy<const uint32_t>& baseArrayLayers,
		const vk::ArrayProxy<const uint32_t>& numLayers, ImageLayout layout)
	{
		_objectReferences.emplace_back(image);
		clearDepthStencilImageHelper(cb, image, layout, ImageAspectFlagBits::eDepth | ImageAspectFlagBits::eStencil, clearDepth, clearStencil,
			baseMipLevel, numLevels, baseArrayLayers, numLayers);
	}



	void CommandBuffer::clearAttachments(const vk::ArrayProxy<const ClearAttachment>& clearAttachments, const vk::ArrayProxy<const ClearRect>& clearRectangles)
	{

		cb.clearAttachments(clearAttachments, clearRectangles);

	}

	// drawing commands
	void CommandBuffer::drawIndexed(uint32_t firstIndex, uint32_t numIndices, int32_t vertexOffset, uint32_t firstInstance, uint32_t numInstances)
	{
		cb.drawIndexed(numIndices, numInstances, firstIndex, vertexOffset, firstInstance);
	}

	void CommandBuffer::draw(uint32_t firstVertex, uint32_t numVertices, uint32_t firstInstance, uint32_t numInstances)
	{
		cb.draw(numVertices, numInstances, firstVertex, firstInstance);
	}

	void CommandBuffer::drawIndexedIndirect(const BufferHandle& buffer, uint32_t offset, uint32_t count, uint32_t stride)
	{
		_objectReferences.emplace_back(buffer);
		cb.drawIndexedIndirect(buffer->getVkHandle(), offset, count, stride);
	}

	void CommandBuffer::drawIndirect(const BufferHandle& buffer, uint32_t offset, uint32_t count, uint32_t stride)
	{
		_objectReferences.emplace_back(buffer);
		cb.drawIndirect(buffer->getVkHandle(), offset, count, stride);
	}

	void CommandBuffer::dispatch(uint32_t numGroupX, uint32_t numGroupY, uint32_t numGroupZ)
	{
		cb.dispatch(numGroupX, numGroupY, numGroupZ);

	}

	void CommandBuffer::dispatchIndirect(BufferHandle& buffer, uint32_t offset) { cb.dispatchIndirect(buffer->getVkHandle(), offset); }

	void CommandBuffer::resetQueryPool(QueryPool& queryPool, uint32_t firstQuery, uint32_t queryCount)
	{
		//_objectReferences.emplace_back(queryPool);
	//	assert(firstQuery + queryCount <= queryPool->getNumQueries() && "Attempted to reset a query with index larger than the number of queries available to the QueryPool");
		cb.resetQueryPool(queryPool, firstQuery, queryCount);
		//	getDevice()->getVkBindings().vkCmdResetQueryPool(getDefaultView(), queryPool->getDefaultView(), firstQuery, queryCount);
	}

	void CommandBuffer::resetQueryPool(QueryPool& queryPool, uint32_t queryIndex)
	{
		//_objectReferences.emplace_back(queryPool);
		resetQueryPool(queryPool, queryIndex, 1);
	}

	void CommandBuffer::beginQuery(QueryPool& queryPool, uint32_t queryIndex, QueryControlFlags flags)
	{
		/*if (queryIndex >= queryPool->getNumQueries())
		{
			throw ErrorValidationFailedEXT("Attempted to begin a query with index larger than the number of queries available to the QueryPool");
		}*/
		//_objectReferences.emplace_back(queryPool);
		cb.beginQuery(queryPool, queryIndex, flags);

	}

	void CommandBuffer::endQuery(QueryPool& queryPool, uint32_t queryIndex)
	{

		//_objectReferences.emplace_back(queryPool);
	//	getDevice()->getVkBindings().vkCmdEndQuery(getDefaultView(), queryPool->getDefaultView(), queryIndex);
		cb.endQuery(queryPool, queryIndex);
	}



	void CommandBuffer::drawIndirectByteCount(
		uint32_t instanceCount, uint32_t firstInstance, BufferHandle& counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride)
	{
		_objectReferences.emplace_back(counterBuffer);
		cb.drawIndirectByteCountEXT(instanceCount, firstInstance, counterBuffer->getVkHandle(), counterBufferOffset, counterOffset, vertexStride);

	}
	void setImageLayout(CommandBufferHandle const& commandBuffer,
		ImageHandle                 image,
		vk::ImageLayout           oldImageLayout,
		vk::ImageLayout           newImageLayout)
	{
		vk::AccessFlags sourceAccessMask = getAccesFlagsFromLayout(oldImageLayout);
		//	switch (oldImageLayout)
		//	{
		//	case vk::ImageLayout::eTransferDstOptimal: sourceAccessMask = vk::AccessFlagBits::eTransferWrite; break;
		//	case vk::ImageLayout::ePreinitialized: sourceAccessMask = vk::AccessFlagBits::eHostWrite; break;
		//	case vk::ImageLayout::eGeneral:  // sourceAccessMask is empty
		//	case vk::ImageLayout::eUndefined: break;
		////	default: assert(false); break;
		//	}

		vk::PipelineStageFlags sourceStage;
		switch (oldImageLayout)
		{
		case vk::ImageLayout::eGeneral:
		case vk::ImageLayout::ePreinitialized: sourceStage = vk::PipelineStageFlagBits::eHost; break;
		case vk::ImageLayout::eTransferDstOptimal: sourceStage = vk::PipelineStageFlagBits::eTransfer; break;
		case vk::ImageLayout::eUndefined: sourceStage = vk::PipelineStageFlagBits::eTopOfPipe; break;
		case vk::ImageLayout::eShaderReadOnlyOptimal:sourceStage = vk::PipelineStageFlagBits::eTopOfPipe; break;

			//	default: assert(false); break;
		}

		vk::AccessFlags destinationAccessMask = getAccesFlagsFromLayout(newImageLayout);
		//	switch (newImageLayout)
		//	{
		//	case vk::ImageLayout::eColorAttachmentOptimal:
		//		destinationAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		//		break;
		//	case vk::ImageLayout::eDepthStencilAttachmentOptimal:
		//		destinationAccessMask =
		//			vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		//		break;
		//	case vk::ImageLayout::eGeneral:  // empty destinationAccessMask
		//	case vk::ImageLayout::ePresentSrcKHR: break;
		//	case vk::ImageLayout::eShaderReadOnlyOptimal: destinationAccessMask = vk::AccessFlagBits::eShaderRead; break;
		//	case vk::ImageLayout::eTransferSrcOptimal: destinationAccessMask = vk::AccessFlagBits::eTransferRead; break;
		//	case vk::ImageLayout::eTransferDstOptimal: destinationAccessMask = vk::AccessFlagBits::eTransferWrite; break;
		////	default: assert(false); break;
		//	}

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
			//	default: assert(false); break;
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
		commandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands, {}, barrier);
	}
	void barrier_prepare_generate_mipmap(CommandBufferHandle const& cb, const ImageHandle& image, vk::ImageLayout base_level_layout,
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
	void generateMipmap(CommandBufferHandle const& cb, ImageHandle& image) {
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
