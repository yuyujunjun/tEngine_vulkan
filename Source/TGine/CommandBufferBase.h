#pragma once
#include"vulkan/vulkan.hpp"
#include"Tgine.h"
#include<memory>
namespace tEngine {

	class tCommandPool {
	public:
		tCommandPool(const Device* device, const vk::CommandPool& vkPool, uint32_t queueFamilyIdx) :device(device),vkCommandPool(vkPool), queueFamilyIdx(queueFamilyIdx){}
		~tCommandPool();
		vk::CommandPool vkCommandPool;
		const uint32_t queueFamilyIdx;
	private:
		weakDevice device;
	};
	class CommandBuffer {
	protected:
		using CommandBufferUsageFlags = vk::CommandBufferUsageFlags;
		using QueryPool = vk::QueryPool;
		using QueryControlFlags = vk::QueryControlFlags;
		using PipelineStageFlags = vk::PipelineStageFlags;
		using DependencyFlags = vk::DependencyFlags;
		using Event = vk::Event;
		using PipelineBindPoint = vk::PipelineBindPoint;
		using PipelineLayout = vk::PipelineLayout;
		using ShaderStageFlags = vk::ShaderStageFlags;
		using ImageResolve = vk::ImageResolve;
		using ImageLayout = vk::ImageLayout;
		using ImageBlit = vk::ImageBlit;
		using Filter = vk::Filter;
		using ImageCopy = vk::ImageCopy;
		using BufferImageCopy = vk::BufferImageCopy;
		using BufferCopy = vk::BufferCopy;
		using Viewport = vk::Viewport;
		using AttachmentReference = vk::AttachmentReference;
		using ClearAttachment = vk::ClearAttachment;
		using ClearRect = vk::ClearRect;
		using StructureType = vk::StructureType;
		using Rect2D = vk::Rect2D;
		using ClearValue = vk::ClearValue;
		using StencilFaceFlags = vk::StencilFaceFlags;
		using ClearColorValue = vk::ClearColorValue;
		using ImageAspectFlagBits = vk::ImageAspectFlagBits;
		using IndexType = vk::IndexType;
	protected:
		std::vector<std::shared_ptr<void>> _objectReferences;
		std::shared_ptr<tEngine::tCommandPool> _pool;
		vk::CommandBuffer cb;

		bool _isRecording;
		weakDevice device;
		const Device* getDevice() {
			return device;
		}
	
	public:
		~CommandBuffer();
		int getQueueFamilyIdx() { return _pool->queueFamilyIdx; }
		bool isRecording() { return _isRecording; }
		const vk::CommandBuffer& getVkHandle()
		{
			return cb;
		}
		//!\cond NO_DOXYGEN
		DECLARE_NO_COPY_SEMANTICS(CommandBuffer)
	
		CommandBuffer(const Device* device,const vk::CommandBuffer& cb, CommandPoolHandle cmdPool)
			: device(device), _isRecording(false),cb(cb), _pool(cmdPool)
		{}

		void begin(const CommandBufferUsageFlags flags = CommandBufferUsageFlags());
		void begin(const RenderPassHandle renderPass, uint32_t subpass, const vk::CommandBufferUsageFlags flags);
		void end();
		void beginDebugUtilsLabel(const vk::DebugUtilsLabelEXT& labelInfo)
		{
			cb.beginDebugUtilsLabelEXT(labelInfo);
		}
		void endDebugUtilsLabel() { cb.endDebugUtilsLabelEXT(); }
		void insertDebugUtilsLabel(const vk::DebugUtilsLabelEXT& labelInfo)
		{
			cb.insertDebugUtilsLabelEXT(labelInfo);
		}
		void debugMarkerBeginEXT(vk::DebugMarkerMarkerInfoEXT& markerInfo)
		{
			cb.debugMarkerBeginEXT(markerInfo);
		}
		void debugMarkerEndEXT() { cb.debugMarkerEndEXT(); }
		void debugMarkerInsertEXT(vk::DebugMarkerMarkerInfoEXT& markerInfo)
		{
			cb.debugMarkerInsertEXT(markerInfo);
		}
		void resetQueryPool(QueryPool& queryPool, uint32_t firstQuery, uint32_t queryCount);
		void resetQueryPool(QueryPool& queryPool, uint32_t queryIndex);
		void beginQuery(QueryPool& queryPool, uint32_t queryIndex, QueryControlFlags flags = QueryControlFlags(0));
		void endQuery(QueryPool& queryPool, uint32_t queryIndex);
		void bindDescriptorSet(vk::PipelineBindPoint bindingPoint, const PipelineLayoutHandle& pipelineLayout, uint32_t firstSet, const std::vector<DescriptorSetHandle>& sets,
			const std::vector<uint32_t>& dynamicOffsets);
		void bindVertexBuffers(const std::vector<BufferHandle>& buffers, uint32_t firstBinding, const std::vector<vk::DeviceSize>& offsets);
		void bindVertexBuffer(const BufferHandle& buffer, vk::DeviceSize offset, uint16_t bindingIndex);
		void bindIndexBuffer(const BufferHandle& buffer, vk::DeviceSize offset, IndexType indexType);
		void pipelineBarrier(PipelineStageFlags srcStage, PipelineStageFlags dstStage, DependencyFlags dependencyFlags, const MemoryBarrierSet& barriers);
		void waitForEvent(const Event& event, PipelineStageFlags srcStage, PipelineStageFlags dstStage, const MemoryBarrierSet& barriers);
		void waitForEvents(const std::vector<Event> events,  PipelineStageFlags srcStage, PipelineStageFlags dstStage, const MemoryBarrierSet& barriers);
		void setEvent(Event& event, PipelineStageFlags pipelineStageFlags = (vk::PipelineStageFlagBits)vk::FlagTraits<vk::PipelineStageFlagBits>::allFlags)
		{
			//_objectReferences.emplace_back(event);
			cb.setEvent(event, pipelineStageFlags);
		}
		void resetEvent(Event& event, PipelineStageFlags pipelineStageFlags = (vk::PipelineStageFlagBits)vk::FlagTraits<vk::PipelineStageFlagBits>::allFlags)
		{
			cb.resetEvent(event, pipelineStageFlags);
		}
		void reset(vk::CommandBufferResetFlags resetFlags )
		{
			_objectReferences.clear();
			cb.reset(resetFlags);
		}
		void copyImage(const ImageHandle& srcImage, const ImageHandle& dstImage, ImageLayout srcImageLayout, ImageLayout dstImageLayout,  const std::vector<ImageCopy>& regions);
		void copyImageToBuffer(const ImageHandle& srcImage, ImageLayout srcImageLayout, BufferHandle& dstBuffer, vk::ArrayProxy<const vk::BufferImageCopy> const& regions);
		void copyBuffer(const BufferHandle& srcBuffer, const BufferHandle& dstBuffer,  const  vk::ArrayProxy<const BufferCopy>& regions);
		void copyBufferToImage(const BufferHandle& buffer, const ImageHandle& image, ImageLayout dstImageLayout, const vk::ArrayProxy<const BufferImageCopy>& regions);
		void fillBuffer(const BufferHandle& dstBuffer, uint32_t dstOffset, uint32_t data, uint64_t size = VK_WHOLE_SIZE);
		void setViewport(const Viewport& viewport);
		void clearAttachments(const vk::ArrayProxy<const ClearAttachment>& clearAttachments, const vk::ArrayProxy<const ClearRect>& clearRectangles);
		void draw(uint32_t firstVertex, uint32_t numVertices, uint32_t firstInstance = 0, uint32_t numInstances = 1);
		void drawIndexed(uint32_t firstIndex, uint32_t numIndices, int32_t vertexOffset = 0, uint32_t firstInstance = 0, uint32_t numInstances = 1);
		void drawIndirect(const BufferHandle& buffer, uint32_t offset, uint32_t count, uint32_t stride);
		void drawIndexedIndirect(const BufferHandle& buffer, uint32_t offset, uint32_t count, uint32_t stride);
		void dispatch(uint32_t numGroupX, uint32_t numGroupY, uint32_t numGroupZ);
		void dispatchIndirect(BufferHandle& buffer, uint32_t offset);
		void clearColorImage(vk::CommandBuffer cb, const ImageHandle& image, ClearColorValue clearColor, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
			const vk::ArrayProxy<const uint32_t>& numLevels, const  vk::ArrayProxy<const uint32_t>& baseArrayLayers, const vk::ArrayProxy<const uint32_t>& numLayers, ImageLayout layout);
		void clearDepthStencilImage(const ImageHandle& image,float clearDepth, uint32_t clearStencil, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
			const vk::ArrayProxy<const uint32_t>& numLevels, const  vk::ArrayProxy<const uint32_t>& baseArrayLayers,
			const vk::ArrayProxy<const uint32_t>& numLayers, ImageLayout layout);
		void clearStencilImage(const ImageHandle& image, uint32_t clearStencil, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
			const vk::ArrayProxy<const uint32_t>& numLevels, const  vk::ArrayProxy<const uint32_t>& baseArrayLayers,
			const vk::ArrayProxy<const uint32_t>& numLayers, ImageLayout layout);
		void clearDepthImage(
			const ImageHandle& image, float clearDepth, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
			const vk::ArrayProxy<const uint32_t>& numLevels, const  vk::ArrayProxy<const uint32_t>& baseArrayLayers,
			const vk::ArrayProxy<const uint32_t>& numLayers, ImageLayout layout);

		void setScissor(uint32_t firstScissor, vk::ArrayProxy< const vk::Rect2D>const& scissors);

		void setDepthBounds(float min, float max);
		void setStencilWriteMask(StencilFaceFlags face, uint32_t writeMask);

		/// <summary>Sets the dynamic stencil reference mask state affecting pipeline objects created with VK_DYNAMIC_STATE_STENCIL_REFERENCE enabled.</summary>
		/// <param name="face">A bitmask of StencilFaceFlags specifying the set of stencil state for which to update the reference value.</param>
		/// <param name="reference">The new value to use as the stencil reference value.</param>
		void setStencilReference(StencilFaceFlags face, uint32_t reference);

		/// <summary>Sets the dynamic stencil compare mask state affecting pipeline objects created with VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK enabled.</summary>
		/// <param name="face">A bitmask of StencilFaceFlags specifying the set of stencil state for which to update the compare mask.</param>
		/// <param name="compareMask">The new value to use as the stencil compare value.</param>
		void setStencilCompareMask(StencilFaceFlags face, uint32_t compareMask);

		/// <summary>Sets the dynamic depth bias state affecting pipeline objects created where depthBiasEnable is enabled.</summary>
		/// <param name="constantFactor">A scalar factor controlling the constant depth value added to each fragment.</param>
		/// <param name="clamp">The maximum (or minimum) depth bias of a fragment.</param>
		/// <param name="slopeFactor">A scalar factor applied to a fragment's slope in depth bias calculations.</param>
		void setDepthBias(float constantFactor, float clamp, float slopeFactor);

		/// <summary>Sets the dynamic blend constant bias state affecting pipeline objects created with VK_DYNAMIC_STATE_BLEND_CONSTANTS enabled.</summary>
		/// <param name="rgba">An array of four values specifying the R, G, B, and A components of the blend constant color used in blending,
		/// depending on the blend factor</param>
		void setBlendConstants(float rgba[4]);

		/// <summary>Sets the dynamic line width state affecting pipeline objects created with VK_DYNAMIC_STATE_LINE_WIDTH enabled.</summary>
		/// <param name="lineWidth">The width of rasterized line segments.</param>
		void setLineWidth(float lineWidth);

		/// <summary>Copies regions of a src image into a dst image, potentially also performing format conversions, aritrary scaling and filtering.</summary>
		/// <param name="srcImage">The src Image in the copy.</param>
		/// <param name="dstImage">The dst image.</param>
		/// <param name="regions">A pointer to an array of ImageBlitRange structures specifying the regions to blit.</param>
		/// <param name="numRegions">The number of regions to blit.</param>
		/// <param name="filter">A Filter specifying the filter to apply if the blits require scaling</param>
		/// <param name="srcLayout">The layout of the src image subresrcs for the blit.</param>
		/// <param name="dstLayout">The layout of the dst image subresrcs for the blit.</param>
		void blitImage(const ImageHandle& srcImage, const ImageHandle& dstImage, const std::vector<vk::ImageBlit>& regions, Filter filter);
		void blitImage(const ImageHandle& dst, const ImageHandle& src,
			const vk::Offset3D& dst_offset,
			const vk::Offset3D& dst_extent, const vk::Offset3D& src_offset, const vk::Offset3D& src_extent,
			unsigned dst_level, unsigned src_level, unsigned dst_base_layer, unsigned src_base_layer,
			unsigned num_layers, vk::Filter filter);

		/// <summary>Copies regions of a src image into a dst image, potentially also performing format conversions, aritrary scaling and filtering.</summary>
		/// <param name="srcImage">The src Image in the copy.</param>
		/// <param name="dstImage">The dst image.</param>
		/// <param name="regions">A pointer to an array of ImageBlitRange structures specifying the regions to blit.</param>
		/// <param name="numRegions">The number of regions to blit.</param>
		/// <param name="srcLayout">The layout of the src image subresrcs for the blit.</param>
		/// <param name="dstLayout">The layout of the dst image subresrcs for the blit.</param>
		void resolveImage(const ImageHandle& srcImage, const ImageHandle& dstImage, const std::vector<ImageResolve>& regions, ImageLayout srcLayout, ImageLayout dstLayout);

		/// <summary>Updates buffer data inline in a command buffer. The update is only allowed outside of a renderpass and is treated as a transfer operation
		/// for the purposes of syncrhonization.</summary>
		/// <param name="buffer">The buffer to be updated.</param>
		/// <param name="data">A pointer to the src data for the buffer update. The data must be at least length bytes in size.</param>
		/// <param name="offset">The byte offset into the buffer to start updating, and must be a multiple of 4.</param>
		/// <param name="length">The number of bytes to update, and must be a multiple of 4.</param>
		void updateBuffer(const BufferHandle& buffer, const void* data, uint32_t offset, uint32_t length);
		void bindPipeline(const PipelineHandle& pipeline);
		/// <summary>Updates the value of shader push constants at the offset specified.</summary>
		/// <param name="pipelineLayout">The pipeline layout used to program the push constant updates.</param>
		/// <param name="allstageFlags">A bitmask of ShaderStageFlag specifying the shader stages that will use the push constants in the updated range.</param>
		/// <param name="offset">The start offset of the push constant range to update, in units of bytes.</param>
		/// <param name="size">The size of the push constant range to update, in units of bytes.</param>
		/// <param name="data">An array of size bytes containing the new push constant values.</param>
		void pushConstants(const VkPipelineLayout layout, VkShaderStageFlags stage, uint32_t offset, uint32_t size, const void* data);
		void pushConstants(const VkPipelineLayout layout, VkShaderStageFlags stage, std::vector<uint8_t> block);
		void drawIndirectByteCount(
			uint32_t instanceCount, uint32_t firstInstance, BufferHandle& counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride);
		void executeCommands(const CommandBufferHandle& secondaryCmdBuffer);
		void executeCommands(const std::vector< CommandBufferHandle>& secondaryCmdBuffer);
		void CommandBuffer::beginRenderPass(const RenderPassHandle renderPass, const FrameBufferHandle frameBuffer, bool inlineFirstSubpass);
	
		void endRenderPass();
	};
	void setImageLayout(CommandBufferHandle const& commandBuffer,
		ImageHandle                 image,
		vk::ImageLayout           oldImageLayout,
		vk::ImageLayout           newImageLayout);
	void barrier_prepare_generate_mipmap(CommandBufferHandle const& cb, const ImageHandle& image, vk::ImageLayout base_level_layout,
		vk::PipelineStageFlags src_stage, vk::AccessFlags src_access,
		bool need_top_level_barrier);
	void generateMipmap(CommandBufferHandle const& cb, ImageHandle& image);
	template <typename Func>
	void oneTimeSubmit(CommandBufferHandle & commandBuffer, vk::Queue const& queue, Func const& func)
	{
		commandBuffer->begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		func(commandBuffer);
		commandBuffer->end();
		queue.submit(vk::SubmitInfo(0, nullptr, nullptr, 1, &commandBuffer->getVkHandle()), nullptr);
		queue.waitIdle();
	}
	

}