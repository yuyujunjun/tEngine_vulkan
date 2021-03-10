#include"CommandBufferBase.h"
namespace tEngine {
	
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
	vk::BufferMemoryBarrier bufferBarrier(const tBufferMemoryBarrier& buffBarrier)
	{
		VkBufferMemoryBarrier barrier;
		barrier.sType = static_cast<VkStructureType>(vk::StructureType::eBufferMemoryBarrier);
		barrier.pNext = 0;
		barrier.srcAccessMask = static_cast<VkAccessFlags>(buffBarrier.getSrcAccessMask());
		barrier.dstAccessMask = static_cast<VkAccessFlags>(buffBarrier.getDstAccessMask());

		barrier.dstQueueFamilyIndex = static_cast<uint32_t>(-1);
		barrier.srcQueueFamilyIndex = static_cast<uint32_t>(-1);

		barrier.buffer = buffBarrier.getBuffer()->vkbuffer;
		barrier.offset = buffBarrier.getOffset();
		barrier.size = buffBarrier.getSize();
		return barrier;
	}
	vk::ImageMemoryBarrier imageBarrier(const tImageMemoryBarrier& imgBarrier)
	{
		using StructureType = vk::StructureType;
		VkImageMemoryBarrier barrier = {};
		barrier.sType = static_cast<VkStructureType>(StructureType::eImageMemoryBarrier);
		barrier.srcAccessMask = static_cast<VkAccessFlags>(imgBarrier.getSrcAccessMask());
		barrier.dstAccessMask = static_cast<VkAccessFlags>(imgBarrier.getDstAccessMask());

		barrier.srcQueueFamilyIndex = imgBarrier.getSrcQueueFamilyIndex();
		barrier.dstQueueFamilyIndex = imgBarrier.getDstQueueFamilyIndex();

		barrier.image = imgBarrier.getImage()->vkImage;
		barrier.newLayout = static_cast<VkImageLayout>(imgBarrier.getNewLayout());
		barrier.oldLayout = static_cast<VkImageLayout>(imgBarrier.getOldLayout());
		barrier.subresourceRange = imgBarrier.getSubresourceRange();
		return barrier;
	}
	void CommandBufferBase_::pipelineBarrier(PipelineStageFlags srcStage, PipelineStageFlags dstStage, DependencyFlags dependencyFlags, const MemoryBarrierSet& barriers)
	{
		std::vector<vk::MemoryBarrier> memoryBarriers(barriers.getMemoryBarriers().size());
		std::vector<vk::ImageMemoryBarrier> imageBarriers(barriers.getImageBarriers().size());
		std::vector<vk::BufferMemoryBarrier> bufferBarriers(barriers.getBufferBarriers().size());
		for (int i = 0; i < memoryBarriers.size(); ++i)memoryBarriers[i] = barriers.getMemoryBarriers()[i];
		for (int i = 0; i < bufferBarriers.size(); ++i) bufferBarriers[i] = bufferBarrier(barriers.getBufferBarriers()[i]);
		for (int i = 0; i < imageBarriers.size(); ++i) imageBarriers[i] = imageBarrier(barriers.getImageBarriers()[i]);

		cb.pipelineBarrier(srcStage, dstStage, dependencyFlags, memoryBarriers, bufferBarriers, imageBarriers);


#ifdef DEBUG
		//const auto& imageBarriers = barriers.getImageBarriers();
		for (uint32_t i = 0; i < barriers.getImageBarriers().size(); ++i)
		{
			const auto& currentImageMemoryBarrier = barriers.getImageBarriers()[i];
			currentImageMemoryBarrier.getImage()->setImageLayout(currentImageMemoryBarrier.getNewLayout());
		}
#endif
	}

	void CommandBufferBase_::waitForEvent(const Event& event, PipelineStageFlags srcStage, PipelineStageFlags dstStage, const MemoryBarrierSet& barriers)
	{
		std::vector<vk::MemoryBarrier> memoryBarriers(barriers.getMemoryBarriers().size());
		std::vector<vk::ImageMemoryBarrier> imageBarriers(barriers.getImageBarriers().size());
		std::vector<vk::BufferMemoryBarrier> bufferBarriers(barriers.getBufferBarriers().size());
		for (int i = 0; i < memoryBarriers.size(); ++i)memoryBarriers[i] = barriers.getMemoryBarriers()[i];
		for (int i = 0; i < bufferBarriers.size(); ++i) bufferBarriers[i] = bufferBarrier(barriers.getBufferBarriers()[i]);
		for (int i = 0; i < imageBarriers.size(); ++i) imageBarriers[i] = imageBarrier(barriers.getImageBarriers()[i]);
		cb.waitEvents(event, srcStage, dstStage, memoryBarriers, bufferBarriers, imageBarriers);
	}

	void CommandBufferBase_::waitForEvents(const std::vector<Event> events, PipelineStageFlags srcStage, PipelineStageFlags dstStage, const MemoryBarrierSet& barriers)
	{
		std::vector<vk::MemoryBarrier> memoryBarriers(barriers.getMemoryBarriers().size());
		std::vector<vk::ImageMemoryBarrier> imageBarriers(barriers.getImageBarriers().size());
		std::vector<vk::BufferMemoryBarrier> bufferBarriers(barriers.getBufferBarriers().size());
		for (int i = 0; i < memoryBarriers.size(); ++i)memoryBarriers[i] = barriers.getMemoryBarriers()[i];
		for (int i = 0; i < bufferBarriers.size(); ++i) bufferBarriers[i] = bufferBarrier(barriers.getBufferBarriers()[i]);
		for (int i = 0; i < imageBarriers.size(); ++i) imageBarriers[i] = imageBarrier(barriers.getImageBarriers()[i]);
		cb.waitEvents(events, srcStage, dstStage, memoryBarriers, bufferBarriers, imageBarriers);
	}

	// bind pipelines, sets, vertex/index buffers
	void CommandBufferBase_::bindDescriptorSets(PipelineBindPoint bindingPoint, const PipelineLayout& pipelineLayout, uint32_t firstSet,
		const std::vector<tDescriptorSets::SharedPtr>& sets, const std::vector<uint32_t>& dynamicOffsets)
	{
		auto numDescriptorSets = sets.size();
		const int MaxDescriptorSets = 8;
		assert(numDescriptorSets < static_cast<uint32_t>(MaxDescriptorSets) && "Attempted to bind more than 8 descriptor sets");
		if (numDescriptorSets < static_cast<uint32_t>(MaxDescriptorSets))
		{
			std::vector<vk::DescriptorSet> native_sets(numDescriptorSets);
			for (uint32_t i = 0; i < numDescriptorSets; ++i)
			{
				_objectReferences.emplace_back(sets[i]);
				native_sets[i] = sets[i]->vkDescSet;
			}
			cb.bindDescriptorSets(bindingPoint, pipelineLayout, firstSet, native_sets, dynamicOffsets);
		}
	}



	void CommandBufferBase_::begin(const CommandBufferUsageFlags flags)
	{
		if (_isRecording) { assert(false && "Called CommandBuffer::begin while a recording was already in progress. Call CommandBuffer::end first"); }
		_isRecording = true;

		vk::CommandBufferBeginInfo info;
		info.flags = flags;
		vk::CommandBufferInheritanceInfo inheritanceInfo;// = {};
		info.pInheritanceInfo = &inheritanceInfo;
		cb.begin(info);

	}

	void SecondaryCommandBuffer::begin(const tFrameBuffer::SharedPtr& framebuffer, uint32_t subpass, const vk::CommandBufferUsageFlags flags)
	{
		if (_isRecording) { throw ("Called CommandBuffer::begin while a recording was already in progress. Call CommandBuffer::end first"); }
		_objectReferences.emplace_back(framebuffer);
		_isRecording = true;
		vk::CommandBufferBeginInfo info;// = {};
		vk::CommandBufferInheritanceInfo inheritanceInfo;// = {};

		info.flags = flags;

		inheritanceInfo.renderPass = framebuffer->renderPass->vkRenderPass;
		inheritanceInfo.framebuffer = framebuffer->vkFrameBuffer;
		inheritanceInfo.subpass = subpass;
		inheritanceInfo.occlusionQueryEnable = VK_FALSE;
		info.pInheritanceInfo = &inheritanceInfo;
		cb.begin(info);
	}

	void SecondaryCommandBuffer::begin(const tRenderPass::SharedPtr& renderPass, uint32_t subpass, const vk::CommandBufferUsageFlags flags)
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

		inheritInfo.renderPass = renderPass->vkRenderPass;
		inheritInfo.subpass = subpass;
		inheritInfo.occlusionQueryEnable = VK_FALSE;
		info.pInheritanceInfo = &inheritInfo;
		cb.begin(info);
	}

	void CommandBufferBase_::end()
	{
		if (!_isRecording)
		{
			throw ("Called CommandBuffer::end while a recording "
				"was not in progress. Call CommandBuffer::begin first");
		}
		_isRecording = false;
		cb.end();
	}

	void CommandBuffer::executeCommands(const SecondaryCommandBuffer::SharedPtr& secondaryCmdBuffer)
	{
		if (!secondaryCmdBuffer) { throw ("Secondary command buffer was NULL for ExecuteCommands"); }
		_objectReferences.emplace_back(secondaryCmdBuffer);
		cb.executeCommands(secondaryCmdBuffer->getVkHandle());
	}

	void CommandBuffer::executeCommands(const std::vector<SecondaryCommandBuffer::SharedPtr>& secondaryCmdBuffers)
	{
		std::vector<vk::CommandBuffer> cmdBuffs(secondaryCmdBuffers.size());
		for (uint32_t i = 0; i < secondaryCmdBuffers.size(); ++i)
		{
			_objectReferences.emplace_back(secondaryCmdBuffers[i]);
			cmdBuffs[i] = secondaryCmdBuffers[i]->getVkHandle();
		}
		cb.executeCommands(cmdBuffs);
	}

	// RenderPasses, Subpasses
	void CommandBuffer::beginRenderPass(
		const tFrameBuffer::SharedPtr& framebuffer, const Rect2D& renderArea, bool inlineFirstSubpass, const ClearValue* clearValues, uint32_t numClearValues)
	{
		_objectReferences.emplace_back(framebuffer);
		vk::RenderPassBeginInfo nfo;// = {};
		//nfo.sType = static_cast<VkStructureType>(StructureType::e_RENDER_PASS_BEGIN_INFO);
		nfo.pClearValues = clearValues;
		nfo.clearValueCount = numClearValues;
		nfo.framebuffer = framebuffer->vkFrameBuffer;
		nfo.renderPass = framebuffer->renderPass->vkRenderPass;
		nfo.renderArea = renderArea;
		//	copyRectangleToVulkan(renderArea, nfo.renderArea);


		cb.beginRenderPass(nfo, vk::SubpassContents(inlineFirstSubpass ? vk::SubpassContents::eInline : vk::SubpassContents::eSecondaryCommandBuffers));

#ifdef DEBUG
		_currentlyBoundFramebuffer = framebuffer;
		_currentSubpass = 0;
		updatePerSubpassImageLayouts();
#endif
	}



	// buffers, textures, images, push constants
	void CommandBufferBase_::updateBuffer(const tBuffer::SharedPtr& buffer, const void* data, uint32_t offset, uint32_t length)
	{
		_objectReferences.emplace_back(buffer);
		cb.updateBuffer(buffer->vkbuffer, offset, length, data);

	}

	void CommandBufferBase_::pushConstants(const PipelineLayout& pipelineLayout, ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data)
	{
		//_objectReferences.emplace_back(pipelineLayout);
		cb.pushConstants(pipelineLayout, stageFlags, offset, size, data);
	}

	void CommandBufferBase_::resolveImage(const tImage::SharedPtr& srcImage, const tImage::SharedPtr& dstImage, const std::vector<ImageResolve>& regions, ImageLayout srcLayout, ImageLayout dstLayout)
	{
		_objectReferences.emplace_back(srcImage);
		_objectReferences.emplace_back(dstImage);
		assert(sizeof(ImageResolve) == sizeof(VkImageResolve));
		cb.resolveImage(srcImage->vkImage, srcLayout, dstImage->vkImage, dstLayout, regions);

	}

	void CommandBufferBase_::blitImage(const tImage::SharedPtr& srcImage, const tImage::SharedPtr& dstImage, const std::vector<vk::ImageBlit>& regions, Filter filter, ImageLayout srcLayout, ImageLayout dstLayout)
	{
		_objectReferences.emplace_back(srcImage);
		_objectReferences.emplace_back(dstImage);
		cb.blitImage(srcImage->vkImage, srcLayout, dstImage->vkImage, dstLayout, regions, filter);

	}

	void CommandBufferBase_::copyImage(const tImage::SharedPtr& srcImage, const tImage::SharedPtr& dstImage, ImageLayout srcImageLayout, ImageLayout dstImageLayout, const std::vector<ImageCopy>& regions)
	{
		_objectReferences.emplace_back(srcImage);
		_objectReferences.emplace_back(dstImage);
		// Try to avoid heap allocation
		cb.copyImage(srcImage->vkImage, srcImageLayout, dstImage->vkImage, dstImageLayout, regions);
	}

	void CommandBufferBase_::copyImageToBuffer(const tImage::SharedPtr& srcImage, ImageLayout srcImageLayout, tBuffer::SharedPtr& dstBuffer, vk::ArrayProxy<const vk::BufferImageCopy> const& regions)
	{
		_objectReferences.emplace_back(srcImage);
		_objectReferences.emplace_back(dstBuffer);

		cb.copyImageToBuffer(srcImage->vkImage, srcImageLayout, dstBuffer->vkbuffer, regions);

	}

	void CommandBufferBase_::copyBuffer(const tBuffer::SharedPtr& srcBuffer, const tBuffer::SharedPtr& dstBuffer, vk::ArrayProxy<const vk::BufferCopy>const& regions)
	{
		_objectReferences.emplace_back(srcBuffer);
		_objectReferences.emplace_back(dstBuffer);
		cb.copyBuffer(srcBuffer->vkbuffer, dstBuffer->vkbuffer, regions);
	}
	void CommandBufferBase_::copyBufferToImage(const tBuffer::SharedPtr& buffer, const tImage::SharedPtr& image, vk::ImageLayout dstImageLayout, const vk::ArrayProxy<const BufferImageCopy>& regions)
	{

		_objectReferences.emplace_back(buffer);
		_objectReferences.emplace_back(image);
		cb.copyBufferToImage(buffer->vkbuffer, image->vkImage, dstImageLayout, regions);
	}

	void CommandBufferBase_::fillBuffer(const tBuffer::SharedPtr& dstBuffer, uint32_t dstOffset, uint32_t data, uint64_t size)
	{
		_objectReferences.emplace_back(dstBuffer);
		cb.fillBuffer(dstBuffer->vkbuffer, dstOffset, size, data);
	}

	// dynamic commands
	void CommandBufferBase_::setViewport(const Viewport& viewport) { cb.setViewport(0, viewport); }//; getDevice()->getVkBindings().vkCmdSetViewport(getVkHandle(), 0, 1, &viewport.get());


	void CommandBufferBase_::setScissor(uint32_t firstScissor, vk::ArrayProxy< const vk::Rect2D>const& scissors)
	{
		cb.setScissor(firstScissor, scissors);

	}

	void CommandBufferBase_::setDepthBounds(float min, float max) { cb.setDepthBounds(min, max); }

	void CommandBufferBase_::setStencilCompareMask(StencilFaceFlags face, uint32_t compareMask)
	{
		cb.setStencilCompareMask(face, compareMask);
	}

	void CommandBufferBase_::setStencilWriteMask(StencilFaceFlags face, uint32_t writeMask)
	{
		cb.setStencilWriteMask(face, writeMask);
	}

	void CommandBufferBase_::setStencilReference(StencilFaceFlags face, uint32_t ref)
	{
		cb.setStencilReference(face, ref);
		
	}

	void CommandBufferBase_::setDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
	{
		cb.setDepthBias(depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
	}

	void CommandBufferBase_::setBlendConstants(float rgba[4]) { cb.setBlendConstants(rgba); }

	void CommandBufferBase_::setLineWidth(float lineWidth) { cb.setLineWidth(lineWidth); }

	inline void clearcolorimage( vk::CommandBuffer cb, const tImage::SharedPtr& image, vk::ClearColorValue clearColor, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
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

		cb.clearColorImage(image->vkImage, layout, clearColor, subResourceRanges);
	}

	
	void CommandBufferBase_::clearColorImage(vk::CommandBuffer cb, const tImage::SharedPtr& image, ClearColorValue clearColor, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
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

		cb.clearColorImage(image->vkImage, layout, clearColor, subResourceRanges);
	}
	inline static void clearDepthStencilImageHelper( vk::CommandBuffer cb, const tImage::SharedPtr& image, vk::ImageLayout layout, vk::ImageAspectFlags imageAspect,
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
		cb.clearDepthStencilImage(image->vkImage, layout, clearDepthStencilValue, subResourceRanges);
		
	}

	void CommandBufferBase_::clearDepthImage(
		const tImage::SharedPtr& image, float clearDepth, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
		const vk::ArrayProxy<const uint32_t>& numLevels, const  vk::ArrayProxy<const uint32_t>& baseArrayLayers, const vk::ArrayProxy<const uint32_t>& numLayers, ImageLayout layout)
	{
		_objectReferences.emplace_back(image);
		clearDepthStencilImageHelper(cb, image, layout, ImageAspectFlagBits::eDepth, clearDepth, 0u, baseMipLevel, numLevels, baseArrayLayers, numLayers);
	}



	void CommandBufferBase_::clearStencilImage(
		const tImage::SharedPtr& image, uint32_t clearStencil, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
		const vk::ArrayProxy<const uint32_t>& numLevels, const  vk::ArrayProxy<const uint32_t>& baseArrayLayers,
		const vk::ArrayProxy<const uint32_t>& numLayers, ImageLayout layout)
	{
		_objectReferences.emplace_back(image);
		clearDepthStencilImageHelper(
			cb,image, layout, ImageAspectFlagBits::eStencil, 0.0f, clearStencil, baseMipLevel, numLevels, baseArrayLayers, numLayers);
	}



	void CommandBufferBase_::clearDepthStencilImage(const tImage::SharedPtr& image, float clearDepth, uint32_t clearStencil, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
		const vk::ArrayProxy<const uint32_t>& numLevels, const  vk::ArrayProxy<const uint32_t>& baseArrayLayers,
		const vk::ArrayProxy<const uint32_t>& numLayers, ImageLayout layout)
	{
		_objectReferences.emplace_back(image);
		clearDepthStencilImageHelper(cb, image, layout, ImageAspectFlagBits::eDepth | ImageAspectFlagBits::eStencil, clearDepth, clearStencil,
			baseMipLevel, numLevels, baseArrayLayers, numLayers);
	}

	

	void CommandBufferBase_::clearAttachments(const vk::ArrayProxy<const ClearAttachment>& clearAttachments, const vk::ArrayProxy<const ClearRect>& clearRectangles)
	{
		
		cb.clearAttachments(clearAttachments, clearRectangles);
		
	}

	// drawing commands
	void CommandBufferBase_::drawIndexed(uint32_t firstIndex, uint32_t numIndices, int32_t vertexOffset, uint32_t firstInstance, uint32_t numInstances)
	{
		cb.drawIndexed(numIndices, numInstances, firstIndex, vertexOffset, firstInstance);
	}

	void CommandBufferBase_::draw(uint32_t firstVertex, uint32_t numVertices, uint32_t firstInstance, uint32_t numInstances)
	{
		cb.draw(numVertices, numInstances, firstVertex, firstInstance);
	}

	void CommandBufferBase_::drawIndexedIndirect(const tBuffer::SharedPtr& buffer, uint32_t offset, uint32_t count, uint32_t stride)
	{
		_objectReferences.emplace_back(buffer);
		cb.drawIndexedIndirect(buffer->vkbuffer, offset, count, stride);
	}

	void CommandBufferBase_::drawIndirect(const tBuffer::SharedPtr& buffer, uint32_t offset, uint32_t count, uint32_t stride)
	{
		_objectReferences.emplace_back(buffer);
		cb.drawIndirect(buffer->vkbuffer, offset, count, stride);
	}

	void CommandBufferBase_::dispatch(uint32_t numGroupX, uint32_t numGroupY, uint32_t numGroupZ)
	{
		cb.dispatch(numGroupX, numGroupY, numGroupZ);
		
	}

	void CommandBufferBase_::dispatchIndirect(tBuffer::SharedPtr& buffer, uint32_t offset) { cb.dispatchIndirect(buffer->vkbuffer, offset); }

	void CommandBufferBase_::resetQueryPool(QueryPool& queryPool, uint32_t firstQuery, uint32_t queryCount)
	{
		//_objectReferences.emplace_back(queryPool);
	//	assert(firstQuery + queryCount <= queryPool->getNumQueries() && "Attempted to reset a query with index larger than the number of queries available to the QueryPool");
		cb.resetQueryPool(queryPool, firstQuery, queryCount);
	//	getDevice()->getVkBindings().vkCmdResetQueryPool(getVkHandle(), queryPool->getVkHandle(), firstQuery, queryCount);
	}

	void CommandBufferBase_::resetQueryPool(QueryPool& queryPool, uint32_t queryIndex)
	{
		//_objectReferences.emplace_back(queryPool);
		resetQueryPool(queryPool, queryIndex, 1);
	}

	void CommandBufferBase_::beginQuery(QueryPool& queryPool, uint32_t queryIndex, QueryControlFlags flags)
	{
		/*if (queryIndex >= queryPool->getNumQueries())
		{
			throw ErrorValidationFailedEXT("Attempted to begin a query with index larger than the number of queries available to the QueryPool");
		}*/
		//_objectReferences.emplace_back(queryPool);
		cb.beginQuery(queryPool, queryIndex, flags);
		
	}

	void CommandBufferBase_::endQuery(QueryPool& queryPool, uint32_t queryIndex)
	{
		
		//_objectReferences.emplace_back(queryPool);
	//	getDevice()->getVkBindings().vkCmdEndQuery(getVkHandle(), queryPool->getVkHandle(), queryIndex);
		cb.endQuery(queryPool, queryIndex);
	}



	void CommandBufferBase_::drawIndirectByteCount(
		uint32_t instanceCount, uint32_t firstInstance, tBuffer::SharedPtr& counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride)
	{
		_objectReferences.emplace_back(counterBuffer);
		cb.drawIndirectByteCountEXT(instanceCount, firstInstance, counterBuffer->vkbuffer, counterBufferOffset, counterOffset, vertexStride);

	}
	void setImageLayout(CommandBuffer::SharedPtr const& commandBuffer,
		tImage::SharedPtr                 image,
		vk::Format                format,
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

		vk::ImageAspectFlags aspectMask;
		if (newImageLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		{
			aspectMask = vk::ImageAspectFlagBits::eDepth;
			if (format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint)
			{
				aspectMask |= vk::ImageAspectFlagBits::eStencil;
			}
		}
		else
		{
			aspectMask = vk::ImageAspectFlagBits::eColor;
		}

		vk::ImageSubresourceRange imageSubresourceRange(aspectMask, 0, 1, 0, 1);

		tImageMemoryBarrier imageBarrier;
		imageBarrier.setImage(image);
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
}
