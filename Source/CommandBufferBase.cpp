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
	
	void CommandBufferBase_::pipelineBarrier(PipelineStageFlags srcStage, PipelineStageFlags dstStage, DependencyFlags dependencyFlags, const MemoryBarrierSet& barriers)
	{
		cb.pipelineBarrier(srcStage, dstStage, dependencyFlags, barriers.getMemoryBarriers(), barriers.getBufferBarriers(), barriers.getImageBarriers());
	}

	void CommandBufferBase_::waitForEvent(const Event& event, PipelineStageFlags srcStage, PipelineStageFlags dstStage, const MemoryBarrierSet& barriers)
	{
			cb.waitEvents(event, srcStage, dstStage, barriers.getMemoryBarriers(), barriers.getBufferBarriers(), barriers.getImageBarriers());
	}

	void CommandBufferBase_::waitForEvents(const std::vector<Event> events, PipelineStageFlags srcStage, PipelineStageFlags dstStage, const MemoryBarrierSet& barriers)
	{

		cb.waitEvents(events, srcStage, dstStage, barriers.getMemoryBarriers(), barriers.getBufferBarriers(), barriers.getImageBarriers());
	}
	void CommandBufferBase_::bindDescriptorSets(tDescriptorPool::SharedPtr& descPool, PipelineBindPoint bindingPoint, tShaderInterface* mat) {

		//Get all tobindsets from set 0 to size
		auto tobindsets = descPool->AllocateDescriptorSets(mat->getShader(), mat->getBuffers(), mat->getImages());
		//No need for tobindsets
		if (tobindsets.size() == 0)return;

		//first ensure that same descriptorSet not binded
		for (int set_number = 0; set_number < tobindsets.size(); ) {
			if (tobindsets[set_number] == nullptr)continue;
			tobindsets[set_number]->UpdateDescriptorSets();

		}
		std::vector<tDescriptorSets::SharedPtr> sets;
		std::vector<uint32_t> dynamicOffsets;
		//bind tobindsets
		for (int set_number = 0; set_number <= tobindsets.size(); ++set_number) {
			//Bind DescriptorSets when meets interval sets or at last
			if (set_number == tobindsets.size() || tobindsets[set_number] == nullptr) {
				if (sets.size() != 0) {
					bindDescriptorSets(bindingPoint, mat->getShader()->pipelinelayout, static_cast<uint32_t>(set_number - sets.size()), sets, dynamicOffsets);
					dynamicOffsets.clear();
					sets.clear();
				}
				continue;
			}
			sets.emplace_back(tobindsets[set_number]);
			for (const auto& binding : sets.back()->Setlayout->bindings.bindings) {
				switch (binding.descriptorType) {
				case vk::DescriptorType::eUniformBufferDynamic:
				case vk::DescriptorType::eStorageBufferDynamic:
					dynamicOffsets.emplace_back(mat->getOffsets().at(set_number).at(binding.binding));
					break;
				}
			}
		}
		//Finally upload uniform buffer and update offset
		mat->uploadUniformBuffer();

	}
	// bind pipelines, sets, vertex/index buffers
	void CommandBufferBase_::bindDescriptorSets(PipelineBindPoint bindingPoint, const tPipelineLayout::SharedPtr& pipelineLayout, uint32_t firstSet,
		const std::vector<tDescriptorSets::SharedPtr>& sets, const std::vector<uint32_t>& dynamicOffsets)
	{
		_objectReferences.push_back(pipelineLayout);
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
			cb.bindDescriptorSets(bindingPoint, pipelineLayout->vkLayout, firstSet, native_sets, dynamicOffsets);
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
	void CommandBufferBase_::updateBuffer(const BufferHandle& buffer, const void* data, uint32_t offset, uint32_t length)
	{
		_objectReferences.emplace_back(buffer);
		cb.updateBuffer(buffer->getVkHandle(), offset, length, data);

	}

	void CommandBufferBase_::pushConstants(const tPipelineLayout::SharedPtr& pipelineLayout, ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data)
	{
		_objectReferences.emplace_back(pipelineLayout);
		cb.pushConstants(pipelineLayout->vkLayout, stageFlags, offset, size, data);
	}
	void  CommandBufferBase_::pushConstants(const std::shared_ptr<tShader>& shader) {
		
			if (shader->getInterface()->pushConstantBlock.size() > 0) {
				pushConstants(shader->pipelinelayout, shader->stageFlags, 0, static_cast<uint32_t>(shader->getInterface()->pushConstantBlock.size()), shader->getInterface()->pushConstantBlock.data());
			}
		
	}
	void CommandBufferBase_::resolveImage(const ImageHandle& srcImage, const ImageHandle& dstImage, const std::vector<ImageResolve>& regions, ImageLayout srcLayout, ImageLayout dstLayout)
	{
		_objectReferences.emplace_back(srcImage);
		_objectReferences.emplace_back(dstImage);
		assert(sizeof(ImageResolve) == sizeof(VkImageResolve));
		cb.resolveImage(srcImage->getVkHandle(), srcLayout, dstImage->getVkHandle(), dstLayout, regions);

	}

	void CommandBufferBase_::blitImage(const ImageHandle& srcImage, const ImageHandle& dstImage, const std::vector<vk::ImageBlit>& regions, Filter filter)
	{
		_objectReferences.emplace_back(srcImage);
		_objectReferences.emplace_back(dstImage);
		cb.blitImage(srcImage->getVkHandle(), vk::ImageLayout::eTransferSrcOptimal, dstImage->getVkHandle(), vk::ImageLayout::eTransferDstOptimal, regions, filter);

	}
	void CommandBufferBase_::blitImage(const ImageHandle& dst, const ImageHandle& src,
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
	void CommandBufferBase_::copyImage(const ImageHandle& srcImage, const ImageHandle& dstImage, ImageLayout srcImageLayout, ImageLayout dstImageLayout, const std::vector<ImageCopy>& regions)
	{
		_objectReferences.emplace_back(srcImage);
		_objectReferences.emplace_back(dstImage);
		// Try to avoid heap allocation
		cb.copyImage(srcImage->getVkHandle(), srcImageLayout, dstImage->getVkHandle(), dstImageLayout, regions);
	}

	void CommandBufferBase_::copyImageToBuffer(const ImageHandle& srcImage, ImageLayout srcImageLayout, BufferHandle& dstBuffer, vk::ArrayProxy<const vk::BufferImageCopy> const& regions)
	{
		_objectReferences.emplace_back(srcImage);
		_objectReferences.emplace_back(dstBuffer);

		cb.copyImageToBuffer(srcImage->getVkHandle(), srcImageLayout, dstBuffer->getVkHandle(), regions);

	}

	void CommandBufferBase_::copyBuffer(const BufferHandle& srcBuffer, const BufferHandle& dstBuffer, vk::ArrayProxy<const vk::BufferCopy>const& regions)
	{
		_objectReferences.emplace_back(srcBuffer);
		_objectReferences.emplace_back(dstBuffer);
		cb.copyBuffer(srcBuffer->getVkHandle(), dstBuffer->getVkHandle(), regions);
	}
	void CommandBufferBase_::copyBufferToImage(const BufferHandle& buffer, const ImageHandle& image, vk::ImageLayout dstImageLayout, const vk::ArrayProxy<const BufferImageCopy>& regions)
	{

		_objectReferences.emplace_back(buffer);
		_objectReferences.emplace_back(image);
		cb.copyBufferToImage(buffer->getVkHandle(), image->getVkHandle(), dstImageLayout, regions);
	}

	void CommandBufferBase_::fillBuffer(const BufferHandle& dstBuffer, uint32_t dstOffset, uint32_t data, uint64_t size)
	{
		_objectReferences.emplace_back(dstBuffer);
		cb.fillBuffer(dstBuffer->getVkHandle(), dstOffset, size, data);
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

	inline void clearcolorimage( vk::CommandBuffer cb, const ImageHandle& image, vk::ClearColorValue clearColor, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
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

	
	void CommandBufferBase_::clearColorImage(vk::CommandBuffer cb, const ImageHandle& image, ClearColorValue clearColor, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
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
	inline static void clearDepthStencilImageHelper( vk::CommandBuffer cb, const ImageHandle& image, vk::ImageLayout layout, vk::ImageAspectFlags imageAspect,
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

	void CommandBufferBase_::clearDepthImage(
		const ImageHandle& image, float clearDepth, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
		const vk::ArrayProxy<const uint32_t>& numLevels, const  vk::ArrayProxy<const uint32_t>& baseArrayLayers, const vk::ArrayProxy<const uint32_t>& numLayers, ImageLayout layout)
	{
		_objectReferences.emplace_back(image);
		clearDepthStencilImageHelper(cb, image, layout, ImageAspectFlagBits::eDepth, clearDepth, 0u, baseMipLevel, numLevels, baseArrayLayers, numLayers);
	}



	void CommandBufferBase_::clearStencilImage(
		const ImageHandle& image, uint32_t clearStencil, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
		const vk::ArrayProxy<const uint32_t>& numLevels, const  vk::ArrayProxy<const uint32_t>& baseArrayLayers,
		const vk::ArrayProxy<const uint32_t>& numLayers, ImageLayout layout)
	{
		_objectReferences.emplace_back(image);
		clearDepthStencilImageHelper(
			cb,image, layout, ImageAspectFlagBits::eStencil, 0.0f, clearStencil, baseMipLevel, numLevels, baseArrayLayers, numLayers);
	}



	void CommandBufferBase_::clearDepthStencilImage(const ImageHandle& image, float clearDepth, uint32_t clearStencil, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
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

	void CommandBufferBase_::drawIndexedIndirect(const BufferHandle& buffer, uint32_t offset, uint32_t count, uint32_t stride)
	{
		_objectReferences.emplace_back(buffer);
		cb.drawIndexedIndirect(buffer->getVkHandle(), offset, count, stride);
	}

	void CommandBufferBase_::drawIndirect(const BufferHandle& buffer, uint32_t offset, uint32_t count, uint32_t stride)
	{
		_objectReferences.emplace_back(buffer);
		cb.drawIndirect(buffer->getVkHandle(), offset, count, stride);
	}

	void CommandBufferBase_::dispatch(uint32_t numGroupX, uint32_t numGroupY, uint32_t numGroupZ)
	{
		cb.dispatch(numGroupX, numGroupY, numGroupZ);
		
	}

	void CommandBufferBase_::dispatchIndirect(BufferHandle& buffer, uint32_t offset) { cb.dispatchIndirect(buffer->getVkHandle(), offset); }

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
		uint32_t instanceCount, uint32_t firstInstance, BufferHandle& counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride)
	{
		_objectReferences.emplace_back(counterBuffer);
		cb.drawIndirectByteCountEXT(instanceCount, firstInstance, counterBuffer->getVkHandle(), counterBufferOffset, counterOffset, vertexStride);

	}


}
