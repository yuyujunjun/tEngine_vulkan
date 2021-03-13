#pragma once
#include"vulkan/vulkan.hpp"
#include"Core.h"
#include"tPipeline.h"
#include"tResource.h"
#include"tMemoryBarrier.h"
#include"tDescriptorPool.h"
#include"tFrameBuffer.h"
#include"tShader.h"
#include<memory>
namespace tEngine {

	struct tCommandPool {
		using SharedPtr = std::shared_ptr<tCommandPool>;
		static SharedPtr Create(const uniqueDevice& device, const vk::CommandPool& vkPool) {
			return std::make_shared<tCommandPool>(device,vkPool);
		}
		tCommandPool(const uniqueDevice& device, const vk::CommandPool& vkPool) :device(device.get()),vkCommandPool(vkPool) {}
		vk::CommandPool vkCommandPool;
		
		~tCommandPool() {
			if (vkCommandPool) {
				
				
					device->destroyCommandPool(vkCommandPool);
					vkCommandPool = vk::CommandPool();
			
			}
		}
	private:
		weakDevice device;
	};
	struct CommandBufferBase_ {
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
		
		const vk::CommandBuffer& getVkHandle()
		{
			return cb;
		}
		//!\cond NO_DOXYGEN
		DECLARE_NO_COPY_SEMANTICS(CommandBufferBase_)
		/// <summary>Constructor. This constructor shouldn't be called directly and should instead be called indirectly via a call to
		/// CommandPool::allocateCommandBuffers.</summary>
		/// <param name="device">The device used to allocate this command buffer.</param>
		/// <param name="pool">The pool from which the command buffer was allocated.</param>
		/// <param name="myHandle">The vulkan handle for this command buffer.</param>
		CommandBufferBase_(const Device* device,const vk::CommandBuffer& cb)
			: device(device), _isRecording(false),cb(cb)
		{}
		
		/// <summary>Destructor. Virtual (for polymorphic use).</summary>
		//virtual ~CommandBufferBase_();
		//!\endcond

		/// <summary>Call this function before beginning to record commands.</summary>
		/// <param name="flags">Flags is a bitmask of CommandBufferUsageFlags specifying usage behavior for the command buffer.</param>
		void begin(const CommandBufferUsageFlags flags = CommandBufferUsageFlags());

		/// <summary>Call this function when you are done recording commands. BeginRecording must be called first.</summary>
		void end();

		/// <summary>Begins identifying a region of work submitted to this command buffer. The calls to beginDebugUtilsLabel and endDebugUtilsLabel must be matched and
		/// balanced.</summary>
		/// <param name="labelInfo">Specifies the parameters of the label region to open</param>
		void beginDebugUtilsLabel(const vk::DebugUtilsLabelEXT& labelInfo)
		{
			cb.beginDebugUtilsLabelEXT(labelInfo);
		}

		/// <summary>Ends a label region of work submitted to this command buffer.</summary>
		void endDebugUtilsLabel() { cb.endDebugUtilsLabelEXT(); }

		/// <summary>Inserts a single debug label any time.</summary>
		/// <param name="labelInfo">Specifies the parameters of the label region to insert</param>
		void insertDebugUtilsLabel(const vk::DebugUtilsLabelEXT& labelInfo)
		{
			cb.insertDebugUtilsLabelEXT(labelInfo);
		}

		/// <summary>Begins a debug marked region.</summary>
		/// <param name="markerInfo">Specifies the creation info for a marked region.</param>
		void debugMarkerBeginEXT(vk::DebugMarkerMarkerInfoEXT& markerInfo)
		{
			cb.debugMarkerBeginEXT(markerInfo);
		}

		/// <summary>Ends a debug marked region.</summary>
		void debugMarkerEndEXT() { cb.debugMarkerEndEXT(); }

		/// <summary>Inserts a debug marker.</summary>
		/// <param name="markerInfo">Specifies creation info for the marker.</param>
		void debugMarkerInsertEXT(vk::DebugMarkerMarkerInfoEXT& markerInfo)
		{
			cb.debugMarkerInsertEXT(markerInfo);
		}

		/// <summary>Resets a particular range of queries for a particular QueryPool and sets their status' to unavailable which also makes their numerical results undefined.</summary>
		/// <param name="queryPool">Specifies the query pool managing the queries being reset.</param>
		/// <param name="firstQuery">The first query index to reset.</param>
		/// <param name="queryCount">The number of queries to reset.</param>
		void resetQueryPool(QueryPool& queryPool, uint32_t firstQuery, uint32_t queryCount);

		/// <summary>Resets a particular range of queries for a particular QueryPool and sets their status' to unavailable which also makes their numerical results undefined.</summary>
		/// <param name="queryPool">Specifies the query pool managing the queries being reset.</param>
		/// <param name="queryIndex">The query to reset.</param>
		void resetQueryPool(QueryPool& queryPool, uint32_t queryIndex);

		/// <summary>Begins a query for a particular QueryPool.</summary>
		/// <param name="queryPool">Specifies the query pool which will manage the results of the query.</param>
		/// <param name="queryIndex">The query index within the QueryPool which will contain the results.</param>
		/// <param name="flags">Specifies the Query Control Flag bits which provide constraints on the type of queries that can be performed.</param>
		void beginQuery(QueryPool& queryPool, uint32_t queryIndex, QueryControlFlags flags = QueryControlFlags(0));

		/// <summary>Ends a query for a particular QueryPool.</summary>
		/// <param name="queryPool">Specifies the query pool which will manage the results of the query.</param>
		/// <param name="queryIndex">The query index within the QueryPool which will contain the results.</param>
		void endQuery(QueryPool& queryPool, uint32_t queryIndex);

		

		/// <summary>Queries if a command buffer is in the recording state</summary>
		/// <returns>True if recording, false otherwise</returns>
		bool isRecording() { return _isRecording; }

		/// <summary>Bind a graphics pipeline.</summary>
		/// <param name="pipeline">The GraphicsPipeline to bind.</param>
		void bindPipeline(const GraphicsPipeline::SharedPtr& pipeline)
		{
			_objectReferences.emplace_back(pipeline);
			cb.bindPipeline(pipeline->getBindPoint(), pipeline->VkHandle());
		}

		/// <summary>Bind a compute pipeline</summary>
		/// <param name="pipeline">The ComputePipeline to bind</param>
		void bindPipeline(const ComputePipeline::SharedPtr& pipeline)
		{
			_objectReferences.emplace_back(pipeline);
			cb.bindPipeline(pipeline->getBindPoint(), pipeline->VkHandle());
		}

		/// <summary>Bind descriptorsets</summary>
		/// <param name="bindingPoint">Pipeline binding point</param>
		/// <param name="pipelineLayout">Pipeline layout</param>
		/// <param name="firstSet">The set number of the first descriptor set to be bound</param>
		/// <param name="sets">Pointer to the descriptor sets to be bound</param>
		/// <param name="numDescriptorSets">Number of descriptor sets</param>
		/// <param name="dynamicOffsets">Pointer to an array of uint</param>32_t values specifying dynamic offsets
		/// <param name="numDynamicOffsets">Number of dynamic offsets</param>
		void bindDescriptorSets(PipelineBindPoint bindingPoint, const tPipelineLayout::SharedPtr& pipelineLayout, uint32_t firstSet, const std::vector<tDescriptorSets::SharedPtr>& sets,const std::vector<uint32_t>& dynamicOffsets);
		void bindDescriptorSets(tDescriptorPool::SharedPtr& descPool, PipelineBindPoint bindingPoint, tShaderInterface* mat);
		/// <summary>Bind descriptorset</summary>
		/// <param name="bindingPoint">Pipeline binding point</param>
		/// <param name="pipelineLayout">Pipeline layout</param>
		/// <param name="firstSet">The set number of the first descriptor set to be bound</param>
		/// <param name="set">Descriptor set to be bound</param>
		/// <param name="dynamicOffsets">Pointer to an array of uint</param>32_t values specifying dynamic offsets
		/// <param name="numDynamicOffsets">Number of dynamic offsets</param>
		void bindDescriptorSet(PipelineBindPoint bindingPoint, const tPipelineLayout::SharedPtr& pipelineLayout, uint32_t firstSet, const tDescriptorSets::SharedPtr& set,
			const uint32_t dynamicOffsets = 0)
		{
			bindDescriptorSets(bindingPoint, pipelineLayout, firstSet, { set }, { dynamicOffsets });
		}

		/// <summary>Bind vertex buffer</summary>
		/// <param name="buffers">A set of vertex buffers to bind</param>
		/// <param name="firstBinding">The first index into buffers</param>
		/// <param name="bindingCount">The number of vertex buffers to bind</param>
		/// <param name="offsets">A pointer to an array of bindingCount buffer offsets</param>
		void bindVertexBuffers(const std::vector<BufferHandle>& buffers, uint32_t firstBinding, const std::vector<vk::DeviceSize>& offsets)
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

		/// <summary>Bind vertex buffer</summary>
		/// <param name="buffer">Buffer</param>
		/// <param name="offset">Buffer offset</param>
		/// <param name="bindingIndex">The index of the vertex input binding whose state is updated by the command.</param>
		void bindVertexBuffer(const BufferHandle& buffer, vk::DeviceSize offset, uint16_t bindingIndex)
		{
			_objectReferences.emplace_back(buffer);
			
			cb.bindVertexBuffers( bindingIndex , { buffer->getVkHandle() }, { offset });
		
		}
		/// <summary>Bind index bufer</summary>
		/// <param name="buffer">Imdex buffer</param>
		/// <param name="offset">Buffer offset</param>
		/// <param name="indexType">IndexType</param>
		void bindIndexBuffer(const BufferHandle& buffer, vk::DeviceSize offset, IndexType indexType)
		{
			_objectReferences.emplace_back(buffer);
			cb.bindIndexBuffer(buffer->getVkHandle(), offset, indexType);
		
		}

		/// <summary>Add a memory barrier to the command stream, forcing preceeding commands to be written before
		/// succeeding commands are executed.</summary>
		/// <param name="srcStage">A bitmask of PipelineStageFlags specifying the src stage mask.</param>
		/// <param name="dstStage">A bitmask of PipelineStageFlags specifying the dst stage mask.</param>
		/// <param name="barriers">A set of memory barriers to be used in the pipeline barrier.</param>
		/// <param name="dependencyByRegion">A Specifes whether the dependencies in terms of how the execution and memory dependencies are formed.</param>
		void pipelineBarrier(PipelineStageFlags srcStage, PipelineStageFlags dstStage, DependencyFlags dependencyFlags, const MemoryBarrierSet& barriers);

		/// <summary>Defines a memory dependency between prior event signal operations and subsequent commands.</summary>
		/// <param name="event">The event object to wait on.</param>
		/// <param name="srcStage">A bitmask of PipelineStageFlags specifying the src stage mask.</param>
		/// <param name="dstStage">A bitmask of PipelineStageFlags specifying the dst stage mask.</param>
		/// <param name="barriers">A set of memory barriers to be used in the pipeline barrier.</param>
		void waitForEvent(const Event& event, PipelineStageFlags srcStage, PipelineStageFlags dstStage, const MemoryBarrierSet& barriers);

		/// <summary>Defines a set of memory dependencies between prior event signal operations and subsequent commands.</summary>
		/// <param name="events">A pointer to an array of Event objects to wait on.</param>
		/// <param name="numEvents">The number of event objects to wait on.</param>
		/// <param name="srcStage">A bitmask of PipelineStageFlags specifying the src stage mask.</param>
		/// <param name="dstStage">A bitmask of PipelineStageFlags specifying the dst stage mask.</param>
		/// <param name="barriers">A set of .</param>
		void waitForEvents(const std::vector<Event> events,  PipelineStageFlags srcStage, PipelineStageFlags dstStage, const MemoryBarrierSet& barriers);
		
		/// <summary>Defines an execution dependency on commands that were submitted before it, and defines an event signal operation
		/// which sets the event to the signaled state.</summary>
		/// <param name="event">The event object that will be signaled.</param>
		/// <param name="pipelineStageFlags">Specifies the src stage mask used to determine when the event is signaled.</param>
		void setEvent(Event& event, PipelineStageFlags pipelineStageFlags = (vk::PipelineStageFlagBits)vk::FlagTraits<vk::PipelineStageFlagBits>::allFlags)
		{
			//_objectReferences.emplace_back(event);
			cb.setEvent(event, pipelineStageFlags);
		}

		/// <summary>Defines an execution dependency on commands that were submitted before it, and defines an event unsignal
		/// operation which resets the event to the unsignaled state.</summary>
		/// <param name="event">The event object that will be unsignaled.</param>
		/// <param name="pipelineStageFlags">Is a bitmask of PipelineStageFlags specifying the src stage mask used to determine when the event is unsignaled.</param>
		void resetEvent(Event& event, PipelineStageFlags pipelineStageFlags = (vk::PipelineStageFlagBits)vk::FlagTraits<vk::PipelineStageFlagBits>::allFlags)
		{
			cb.resetEvent(event, pipelineStageFlags);
		}

		/// <summary>Clears this CommandBuffer discarding any previously recorded commands and puts the command buffer in the initial state.
		/// <param name="resetFlags">Is a bitmask of CommandBufferResetFlagBits controlling the reset operation.</param>
		void reset(vk::CommandBufferResetFlags resetFlags )
		{
			_objectReferences.clear();
			cb.reset(resetFlags);
		}

		/// <summary>Copy data between Images</summary>
		/// <param name="srcImage">Source image</param>
		/// <param name="dstImage">Destination image</param>
		/// <param name="srcImageLayout">Source image layout</param>
		/// <param name="dstImageLayout">Destination image layout</param>
		/// <param name="regions">Regions to copy</param>
		/// <param name="numRegions">Number of regions</param>
		void copyImage(const ImageHandle& srcImage, const ImageHandle& dstImage, ImageLayout srcImageLayout, ImageLayout dstImageLayout,  const std::vector<ImageCopy>& regions);

		/// <summary>Copy image to buffer</summary>
		/// <param name="srcImage">Source image to copy from</param>
		/// <param name="srcImageLayout">Current src image layout</param>
		/// <param name="dstBuffer">Destination buffer</param>
		/// <param name="regions">Regions to copy</param>
		/// <param name="numRegions">Number of regions</param>
		void copyImageToBuffer(const ImageHandle& srcImage, ImageLayout srcImageLayout, BufferHandle& dstBuffer, vk::ArrayProxy<const vk::BufferImageCopy> const& regions);

		/// <summary>Copy Buffer</summary>
		/// <param name="srcBuffer">Source buffer</param>
		/// <param name="dstBuffer">Destination buffer</param>
		/// <param name="numRegions">Number of regions to copy</param>
		/// <param name="regions">Pointer to an array of BufferCopy structures specifying the regions to copy.
		/// Each region in pRegions is copied from the source buffer to the same region of the destination buffer.
		/// srcBuffer and dstBuffer can be the same buffer or alias the same memory, but the result is undefined if the copy regions overlap in memory.</param>
		void copyBuffer(const BufferHandle& srcBuffer, const BufferHandle& dstBuffer,  const  vk::ArrayProxy<const BufferCopy>& regions);

		/// <summary>Copy buffer to image</summary>
		/// <param name="buffer">Source Buffer</param>
		/// <param name="image">Destination image</param>
		/// <param name="dstImageLayout">Destination image's current layout</param>
		/// <param name="regionsCount">Copy regions</param>
		/// <param name="regions">Number of regions</param>
		void copyBufferToImage(const BufferHandle& buffer, const ImageHandle& image, ImageLayout dstImageLayout, const vk::ArrayProxy<const BufferImageCopy>& regions);

		/// <summary>Clear buffer data</summary>
		/// <param name="dstBuffer">Destination buffer to be filled</param>
		/// <param name="dstOffset">The byte offset into the buffer at which to start filling.</param>
		/// <param name="data">A 4-byte word written repeatedly to the buffer to fill size bytes of data.
		/// The data word is written to memory according to the host endianness.</param>
		/// <param name="size">The number of bytes to fill, and must be either a multiple of 4, or VK_WHOLE_SIZE to
		/// fill the range from offset to the end of the buffer</param>
		void fillBuffer(const BufferHandle& dstBuffer, uint32_t dstOffset, uint32_t data, uint64_t size = VK_WHOLE_SIZE);

		/// <summary>Set viewport</summary>
		/// <param name="viewport">Viewport</param>
		void setViewport(const Viewport& viewport);

		/// <summary>Clear a set of attacments using a number of regions for each selected attachment to clear whilst inside a renderpass.</summary>
		/// <param name="numAttachments">The number of entries in the clearAttachments array.</param>
		/// <param name="clearAttachments">Is a pointer to an array of ClearAttachment structures which defines the attachments
		/// to clear and the clear values to use.</param>
		/// <param name="numRectangles">Is the number of entries in the clearRects array.</param>
		/// <param name="clearRectangles">Points to an array of ClearRect structures defining regions within each selected attachment to clear.</param>
		void clearAttachments(const vk::ArrayProxy<const ClearAttachment>& clearAttachments, const vk::ArrayProxy<const ClearRect>& clearRectangles);

		/// <summary>Clears a particular attachment using a provided region whilst inside of a renderpass.</summary>
		/// <param name="clearAttachment">A single ClearAttachment structure defining the attachment to clear and the clear value to use</param>
		/// <param name="clearRectangle">A ClearRect structure defining a region within the attachment to clear</param>
	//	void clearAttachment(const ClearAttachment& clearAttachment, const ClearRect& clearRectangle) { clearAttachments(1, &clearAttachment, 1, &clearRectangle); }

		/// <summary>Non-indexed drawing command.</summary>
		/// <param name="firstVertex">The index of the first vertex to draw.</param>
		/// <param name="numVertices">The number of vertices to draw.</param>
		/// <param name="firstInstance">The instance ID of the first instance to draw.</param>
		/// <param name="numInstances">The number of instances to draw.</param>
		void draw(uint32_t firstVertex, uint32_t numVertices, uint32_t firstInstance = 0, uint32_t numInstances = 1);

		/// <summary>Indexed drawing command.</summary>
		/// <param name="firstIndex">The base index within the index buffer.</param>
		/// <param name="numIndices">The number of vertices to draw.</param>
		/// <param name="vertexOffset">The value added to the vertex index before indexining into the vertex buffer.</param>
		/// <param name="firstInstance">The instance ID of the first instance to draw.</param>
		/// <param name="numInstances">The number of instances to draw.</param>
		void drawIndexed(uint32_t firstIndex, uint32_t numIndices, int32_t vertexOffset = 0, uint32_t firstInstance = 0, uint32_t numInstances = 1);

		/// <summary>Non-indexed indirect drawing command.</summary>
		/// <param name="buffer">The buffer containing draw parameters.</param>
		/// <param name="offset">The byte offset into buffer where parameters begin.</param>
		/// <param name="count">The number of draws to execute.</param>
		/// <param name="stride">The byte stride between successive sets of draw commands.</param>
		void drawIndirect(const BufferHandle& buffer, uint32_t offset, uint32_t count, uint32_t stride);

		/// <summary>Non-indexed indirect drawing command.</summary>
		/// <param name="buffer">The buffer containing draw parameters.</param>
		/// <param name="offset">The byte offset into buffer where parameters begin.</param>
		/// <param name="count">The number of draws to execute.</param>
		/// <param name="stride">The byte stride between successive sets of draw commands.</param>
		void drawIndexedIndirect(const BufferHandle& buffer, uint32_t offset, uint32_t count, uint32_t stride);

		/// <summary>Dispatching work provokes work in a compute pipeline. A compute pipeline must be bound to the command buffer
		/// before any dispatch commands are recorded.</summary>
		/// <param name="numGroupX">The number of local workgroups to dispatch in the X dimension.</param>
		/// <param name="numGroupY">The number of local workgroups to dispatch in the Y dimension.</param>
		/// <param name="numGroupZ">The number of local workgroups to dispatch in the Z dimension.</param>
		void dispatch(uint32_t numGroupX, uint32_t numGroupY, uint32_t numGroupZ);

		/// <summary>Dispatching work provokes work in a compute pipeline. A compute pipeline must be bound to the command buffer
		/// before any dispatch commands are recorded. dispatchIndirect behaves similarly to dispatch except that the parameters
		/// are read by the device from a buffer during execution. The parameters of the dispatch are encoded in a DispatchIndirectCommand
		/// structure taken from buffer starting at offset</summary>
		/// <param name="buffer">The buffer containing dispatch parameters.</param>
		/// <param name="offset">The byte offset into buffer where parameters begin.</param>
		void dispatchIndirect(BufferHandle& buffer, uint32_t offset);

	

		/// <summary>Clears a color image outside of a renderpass instance using a number of ranges.</summary>
		/// <param name="image">Image to clear.</param>
		/// <param name="clearColor">Clear color value.</param>
		/// <param name="currentLayout">Image current layout.</param>
		/// <param name="baseMipLevels">Base mip map level to clear.</param>
		/// <param name="numLevels">A pointer to an array of a number of mipmap levels to clear.</param>
		/// <param name="baseArrayLayers">A pointer to an array of base array layers to clear.</param>
		/// <param name="numLayers">A pointer to an array array layers to clear.</param>
		/// <param name="numRanges">The number of elements in the baseMipLevel, numLevels, baseArrayLayers and numLayers arrays.</param>
		void clearColorImage(vk::CommandBuffer cb, const ImageHandle& image, ClearColorValue clearColor, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
			const vk::ArrayProxy<const uint32_t>& numLevels, const  vk::ArrayProxy<const uint32_t>& baseArrayLayers, const vk::ArrayProxy<const uint32_t>& numLayers, ImageLayout layout);

		/// <summary>Clear depth stencil image outside of a renderpass instance.</summary>
		/// <param name="image">Image to clear</param>
		/// <param name="clearDepth">Clear depth value</param>
		/// <param name="clearStencil">Clear stencil value</param>
		/// <param name="baseMipLevel">Base mip map level to clear</param>
		/// <param name="numLevels">Number of mipmap levels to clear</param>
		/// <param name="baseArrayLayer">Base array layer to clear</param>
		/// <param name="numLayers">Number of array layers to clear</param>
		/// <param name="layout">Image current layout</param>
		void clearDepthStencilImage(const ImageHandle& image,float clearDepth, uint32_t clearStencil, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
			const vk::ArrayProxy<const uint32_t>& numLevels, const  vk::ArrayProxy<const uint32_t>& baseArrayLayers,
			const vk::ArrayProxy<const uint32_t>& numLayers, ImageLayout layout);

		/// <summary>Clear depth stencil image outside of a renderpass instance using a number of ranges.</summary>
		/// <param name="image">Image to clear</param>
		/// <param name="clearDepth">Clear depth value</param>
		/// <param name="clearStencil">Clear stencil value</param>
		/// <param name="baseMipLevels">A pointer to an array of base mip map levels to clear</param>
		/// <param name="numLevels">A pointer to an array of the number of mipmap levels to clear</param>
		/// <param name="baseArrayLayers">A pointer to an array of base array layers to clear</param>
		/// <param name="numLayers">A pointer to an array of the number of layers to clear</param>
		/// <param name="numRanges">A number of ranges of the depth stencil image to clear. This number will be
		/// used as the number of array elements in the arrays passed to baseMipLevels, numLevels,
		/// baseArrayLayers and numLayers</param>
		/// <param name="layout">Image current layout</param>
		

		/// <summary>Clears a stencil image outside of a renderpass instance.</summary>
		/// <param name="image">Image to clear</param>
		/// <param name="clearStencil">Clear stencil value</param>
		/// <param name="baseMipLevel">Base mip map level to clear</param>
		/// <param name="numLevels">Number of mipmap levels to clear</param>
		/// <param name="baseArrayLayer">Base array layer to clear</param>
		/// <param name="numLayers">Number of array layers to clear</param>
		/// <param name="layout">Image current layout</param>
		void clearStencilImage(const ImageHandle& image, uint32_t clearStencil, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
			const vk::ArrayProxy<const uint32_t>& numLevels, const  vk::ArrayProxy<const uint32_t>& baseArrayLayers,
			const vk::ArrayProxy<const uint32_t>& numLayers, ImageLayout layout);

		/// <summary>Clear stencil image outside of a renderpass instance using a number of ranges.</summary>
		/// <param name="image">Image to clear</param>
		/// <param name="clearStencil">Clear stencil value</param>
		/// <param name="baseMipLevels">A pointer to an array of base mip map levels to clear</param>
		/// <param name="numLevels">A pointer to an array of the number of mipmap levels to clear</param>
		/// <param name="baseArrayLayers">A pointer to an array of base array layers to clear</param>
		/// <param name="numLayers">A pointer to an array of the number of layers to clear</param>
		/// <param name="numRanges">A number of ranges of the stencil image to clear. This number will be
		/// used as the number of array elements in the arrays passed to baseMipLevels, numLevels,
		/// baseArrayLayers and numLayers</param>
		/// <param name="layout">Image current layout</param>
		

		/// <summary>Clear depth image outside of a renderpass instance.</summary>
		/// <param name="image">Image to clear</param>
		/// <param name="clearDepth">Clear value</param>
		/// <param name="baseMipLevel">Base mip map level to clear</param>
		/// <param name="numLevels">Number of mipmap levels to clear</param>
		/// <param name="baseArrayLayer">Base arraylayer to clear</param>
		/// <param name="numLayers">Number of array layers to clear</param>
		/// <param name="layout">Current layout of the image</param>
		void clearDepthImage(
			const ImageHandle& image, float clearDepth, const vk::ArrayProxy<const uint32_t>& baseMipLevel,
			const vk::ArrayProxy<const uint32_t>& numLevels, const  vk::ArrayProxy<const uint32_t>& baseArrayLayers,
			const vk::ArrayProxy<const uint32_t>& numLayers, ImageLayout layout);


		/// <summary>Sets the dynamic scissor state affecting pipeline objects created with VK_DYNAMIC_STATE_SCISSOR enabled.</summary>
		/// <param name="firstScissor">The index of the first scissor whose state is updated.</param>
		/// <param name="numScissors">The number of scissors whose rectangles are updated.</param>
		/// <param name="scissors">A pointer to an array of Rect2Di structures defining scissor rectangles.</param>
		void setScissor(uint32_t firstScissor, vk::ArrayProxy< const vk::Rect2D>const& scissors);

		/// <summary>Sets the dynamic depth bounds state affecting pipeline objects created with VK_DYNAMIC_STATE_DEPTH_BOUNDS enabled.</summary>
		/// <param name="min">The lower bound of the range of depth values used in the depth bounds test.</param>
		/// <param name="max">The upper bound of the range.</param>
		void setDepthBounds(float min, float max);

		/// <summary>Sets the dynamic stencil write mask state affecting pipeline objects created with VK_DYNAMIC_STATE_STENCIL_WRITE_MASK enabled.</summary>
		/// <param name="face">A bitmask of StencilFaceFlags specifying the set of stencil state for which to update the write mask.</param>
		/// <param name="writeMask">The new value to use as the stencil write mask</param>
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

		/// <summary>Updates the value of shader push constants at the offset specified.</summary>
		/// <param name="pipelineLayout">The pipeline layout used to program the push constant updates.</param>
		/// <param name="stageFlags">A bitmask of ShaderStageFlag specifying the shader stages that will use the push constants in the updated range.</param>
		/// <param name="offset">The start offset of the push constant range to update, in units of bytes.</param>
		/// <param name="size">The size of the push constant range to update, in units of bytes.</param>
		/// <param name="data">An array of size bytes containing the new push constant values.</param>
		void pushConstants(const tPipelineLayout::SharedPtr& pipelineLayout, ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data);

		void pushConstants(const tShader::SharedPtr& shader);
		/// <summary>Records a non-indexed draw call, where the vertex count is based on a byte count read from a buffer and the passed in vertex stride parameter.</summary>
		/// <param name="instanceCount">The number of instances to draw.</param>
		/// <param name="firstInstance">The instance ID of the first instance to draw.</param>
		/// <param name="counterBuffer">The buffer handle from where the byte count is read.</param>
		/// <param name="counterBufferOffset">The offset into the buffer used to read the byte count, which is used to calculate the vertex count for this draw call.</param>
		/// <param name="counterOffset">Is subtracted from the byte count read from the counterBuffer at the counterBufferOffset.</param>
		/// <param name="vertexStride">The stride in bytes between each element of the vertex data that is used to calculate the vertex count from the counter value</param>
		void drawIndirectByteCount(
			uint32_t instanceCount, uint32_t firstInstance, BufferHandle& counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride);

		/// <summary>Const getter for the command pool used to allocate this command buffer.</summary>
		/// <returns>The command pool used to allocate this command buffer.</returns>
		const std::shared_ptr<tEngine::tCommandPool>& getCommandPool() const { return _pool; }
	};
	struct SecondaryCommandBuffer :public CommandBufferBase_ {
		using SharedPtr = std::shared_ptr<SecondaryCommandBuffer>;
		static SharedPtr Create(const uniqueDevice& device, const vk::CommandBuffer& cb) {
			return std::make_shared<SecondaryCommandBuffer>(device,cb);
		}
		SecondaryCommandBuffer(const Device* device, const vk::CommandBuffer& cb) :CommandBufferBase_(device,cb) {}
		void begin(const tFrameBuffer::SharedPtr& framebuffer, uint32_t subpass, const vk::CommandBufferUsageFlags flags);
		void begin(const tRenderPass::SharedPtr& renderPass, uint32_t subpass, const vk::CommandBufferUsageFlags flags);
	};

	//using SecondaryCommandBuffer = std::shared_ptr<SecondaryCommandBuffer>;
	class CommandBuffer :public CommandBufferBase_ {
	public:
		enum class Type {
			Generic,
			AsyncGraphics,
			AsyncCompute,
			AsyncTransfer,
			Count
		};
		using SharedPtr = std::shared_ptr<CommandBuffer>;
		static SharedPtr Create(const uniqueDevice& device, const vk::CommandBuffer& cb) {
			return std::make_shared<CommandBuffer>(device, cb);
		}
		CommandBuffer(const Device* device, const vk::CommandBuffer& cb) :CommandBufferBase_(device, cb) {}
#ifdef DEBUG
		tFrameBuffer::SharedPtr _currentlyBoundFramebuffer;
		uint32_t _currentSubpass;
#endif
		void executeCommands(const SecondaryCommandBuffer::SharedPtr& secondaryCmdBuffer);
		void executeCommands(const std::vector< SecondaryCommandBuffer::SharedPtr>& secondaryCmdBuffer);
		void beginRenderPass(
			const tFrameBuffer::SharedPtr& framebuffer, const vk::Rect2D& renderArea, bool inlineFirstSubpass, const vk::ClearValue* clearValues, uint32_t numClearValues);
	private:
		void updatePerSubpassImageLayouts();
	};
	void setImageLayout(CommandBuffer::SharedPtr const& commandBuffer,
		ImageHandle                 image,
		vk::ImageLayout           oldImageLayout,
		vk::ImageLayout           newImageLayout);
	void barrier_prepare_generate_mipmap(CommandBuffer::SharedPtr const& cb, const ImageHandle& image, vk::ImageLayout base_level_layout,
		vk::PipelineStageFlags src_stage, vk::AccessFlags src_access,
		bool need_top_level_barrier);
	void generateMipmap(CommandBuffer::SharedPtr const& cb, ImageHandle image);
	//using CommandBuffer = std::shared_ptr<CommandBuffer>;
}