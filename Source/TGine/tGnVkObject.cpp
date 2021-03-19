#include"tDevice.h"
#include"tShader.h"
#include"tDescriptorPool.h"
#include"CommandBufferBase.h"
#include"tFenceSemaphore.h"
#include"tFrameBuffer.h"
#include"tShader.h"
#include"tLog.h"
#include"tTextureFormatLayout.h"
#include"tSwapChain.h"
#include"tAssetLoadManager.h"
#include"tResource.h"
#include"tPipeline.h"
#include"Reflector.h"
namespace tEngine {
	tCommandPool::~tCommandPool() {
		if (vkCommandPool) {


			device->destroyCommandPool(vkCommandPool);
			vkCommandPool = vk::CommandPool();

		}
	}
	CommandBuffer::~CommandBuffer() {
		device->freeCommandBuffers(_pool->vkCommandPool, cb);
	}
	CommandBufferHandle allocateCommandBuffer(weakDevice device, CommandPoolHandle cmdPool) {
		vk::CommandBufferAllocateInfo info = {};
		info.setCommandBufferCount(1);
		info.setCommandPool(cmdPool->vkCommandPool);
		info.setLevel(vk::CommandBufferLevel::ePrimary);
		auto vkcb = device->allocateCommandBuffers(info);
		return std::make_shared<CommandBuffer>(device, vkcb.front(), cmdPool);
	}
	tDescriptorSet::~tDescriptorSet() {
		device->freeDescriptorSets(pool->getVkHandle(), set);
	}
	void tDescriptorSetAllocator::createPool() {
		uint32_t maxSets = 0;
		std::vector<vk::DescriptorPoolSize> poolSize;
		for (auto& bi : layout->getCreateInfo().bindings) {
			poolSize.emplace_back(bi.descriptorType, bi.descriptorCount * RingSize);
			maxSets += bi.descriptorCount * RingSize;
		}
		assert(0 < maxSets);
		vk::DescriptorPoolCreateInfo info = {};
		info.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
		info.setPoolSizes(poolSize);

		info.setMaxSets(maxSets);
		auto vkpool = device->createDescriptorPool(info);
		pool = std::make_shared<tDescriptorPool>(device, vkpool);
	}
	DescSetAllocHandle tDescriptorSetAllocatorManager::requestSetAllocator(Device* device, const DescriptorLayoutCreateInfo& bindings) {
		//find first
		for (size_t i = 0; i < bindingInfoList.size(); ++i) {
			if (bindings.bindings == bindingInfoList[i].bindings) {
				if (!allocList[i].expired()) {
					return allocList[i].lock();
				}
			}
		}
		std::lock_guard<std::mutex> lock(mtx);

		for (size_t i = 0; i < bindingInfoList.size(); ++i) {
			if (bindings.bindings == bindingInfoList[i].bindings) {
				if (!allocList[i].expired()) {
					return allocList[i].lock();
				}
				else {
					vk::DescriptorSetLayoutCreateInfo info;
					info.setBindings(bindings.bindings);
					auto vkLayout = device->createDescriptorSetLayout(info);
					auto descSetLayout = std::make_shared<tDescriptorSetLayout>(device, vkLayout, bindings);
					auto alloc = std::make_shared<tDescriptorSetAllocator>(device, descSetLayout);
					allocList[i] = alloc;
					return alloc;
				}
			}
		}
		vk::DescriptorSetLayoutCreateInfo info;
		info.setBindings(bindings.bindings);
		auto vkLayout = device->createDescriptorSetLayout(info);
		auto descSetLayout = std::make_shared<tDescriptorSetLayout>(device, vkLayout, bindings);
		auto alloc = std::make_shared<tDescriptorSetAllocator>(device, descSetLayout);
		allocList.push_back(alloc);
		bindingInfoList.push_back(bindings);
		return alloc;
	}
	tFence::~tFence() {
		if (fece) {
			auto result = device->waitForFences(fece, true, static_cast<uint64_t>(-1));
			assert(result == vk::Result::eSuccess);
			device->destroyFence(fece);
			fece = VkFence(VK_NULL_HANDLE);
		}
	}
	FenceHandle FenceManager::requestSingaledFence() {
		if (signalfences.size() == 0) {
			return std::make_shared<tFence>(device, device->createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)));
		}
		else {
			auto f = signalfences.back();
			signalfences.pop_back();
			return f;
		}
	}
	SemaphoreHandle SemaphoreManager::requestSemaphore() {
		if (semaphore.size() == 0) {
			return std::make_shared<tSemaphore>(device, device->createSemaphore(vk::SemaphoreCreateInfo()));
		}
		else {
			auto f = semaphore.back();
			semaphore.pop_back();
			return f;
		}
	}
	tSemaphore::~tSemaphore() {
		if (semaphore) {

			device->destroySemaphore(semaphore);
			semaphore = VkSemaphore(VK_NULL_HANDLE);
		}
	}
	void tRenderPass::setupRenderPass(VkPipelineBindPoint bindp) {
		vk::RenderPassCreateInfo info;
		std::vector<vk::AttachmentDescription> attachments;

		for (auto& re : resource) {
			attachments.emplace_back(re.description);
		}

		info.setAttachments(attachments);//Color attachment 没有设置成功
		info.setDependencies(dependencies);
		std::vector<vk::SubpassDescription> subPass;
		for (auto& pa : passes) {
			auto& desc = pa->createDescription(bindp);
			subPass.emplace_back(pa->createDescription(bindp));
		}
		info.setSubpasses(subPass);
		vkRenderPass = device->createRenderPass(info);
		renderFunction.resize(passes.size());
		images.resize(resource.size());
		views.resize(resource.size());
	}
	void tFrameBuffer::setupFrameBuffer() {
		std::vector<vk::ImageView> viewList;
		for (auto& frame : pass->getViews()) {
			viewList.emplace_back(frame);
		}

		width = pass->getImages()[0]->get_width();
		height = pass->getImages()[0]->get_height();
		vk::FramebufferCreateInfo info;
		info.setAttachments(viewList);
		info.setHeight(height);
		info.setLayers(pass->getImages()[0]->get_create_info().layers);
		info.setRenderPass(pass->getVkHandle());
		info.setWidth(width);
		vkFrameBuffer = device->createFramebuffer(info);

	}
	tRenderPass::~tRenderPass() {
		if (vkRenderPass) {
			device->destroyRenderPass(vkRenderPass);
		}
	}
	tFrameBuffer::~tFrameBuffer() {
		if (vkFrameBuffer) {

			device->destroyFramebuffer(vkFrameBuffer);

		}
	}
	BufferRangeManager createBufferFromBlock(Device* device, const GpuBlockBuffer& block, uint32_t rangeCount) {
		//	auto s_b = blockToSetBinding.at(name);
			//auto block = setInfos[s_b.first].blockBuffers.at(s_b.second);
			//auto type = setInfos[s_b.first].data.bindingAt(s_b.second).descriptorType;
		BufferCreateInfo createInfo;
		createInfo.domain = BufferDomain::Host;

		createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		size_t rangeSize = 0;
		rangeSize = alignUniformBufferAlignment(block.ByteSize(), device->getPhysicalDevice());

		createInfo.size = rangeSize * rangeCount;
		auto buffer = createBuffer(device,createInfo);
		return BufferRangeManager(buffer, rangeSize, 0);
	}
	vk::Pipeline GraphicsPipelineCreateInfo::createPipeline(const Device* device)const {
		vk::GraphicsPipelineCreateInfo info = {};

		info.setLayout(layout);
		auto cbs = coloBlendState.getCreateInfo();
		info.setPColorBlendState(&cbs);
		auto dss = depthStencilState.getCreateInfo();
		info.setPDepthStencilState(&dss);
		auto ds = dynamicState.getCreateInfo();
		info.setPDynamicState(&ds);
		vk::PipelineVertexInputStateCreateInfo VertexInputState;
		VertexInputState.setVertexAttributeDescriptions(vertexAttribute);
		VertexInputState.setVertexBindingDescriptions(vertexInput);
		info.setPVertexInputState(&VertexInputState);
		auto t = topology.getCreateInfo();
		info.setPInputAssemblyState(&t);
		auto multisample = multisampleState.getCreateInfo();
		info.setPMultisampleState(&multisample);
		auto rasterization = rasterizationState.getCreateInfo();
		info.setPRasterizationState(&rasterization);
		info.setStages(Stages);
		vk::PipelineTessellationStateCreateInfo tessellationState;
		tessellationState.setPatchControlPoints(tessellation.patchControlPoints);
		info.setPTessellationState(&tessellationState);
		vk::PipelineViewportStateCreateInfo view;
		view.setScissors(viewport.scissors);
		view.setViewports(viewport.viewPorts);
		info.setPViewportState(&view);
		info.setRenderPass(renderPass);
		info.setSubpass(subpass);
		info.setBasePipelineIndex(-1);
		auto result = device->createGraphicsPipeline(device->getPipelineCache(), info);
		assert(result.result == vk::Result::eSuccess);
		return result.value;
	}
	vk::Pipeline ComputePipelineCreateInfo::createPipeline(const Device* device)const {
		vk::ComputePipelineCreateInfo info = {};
		info.setLayout(layout);
		info.setStage(Stage);
		auto result = device->createComputePipeline(device->getPipelineCache(), info);
		assert(result.result == vk::Result::eSuccess);
		return result.value;

	}
	tPipeline::~tPipeline() {
		if (vkpipeline) {
			//	assert(false);
			device->destroyPipeline(vkpipeline);

		}
	}
	tPipelineLayout::~tPipelineLayout() {
		if (vkLayout) {
			device->destroyPipelineLayout(vkLayout);
			//		assert(false);
			vkLayout = vk::PipelineLayout();
		}
	}

	tBuffer::~tBuffer() {

		if (vkHandle) {

			device->freeAllocation(alloc);

			device->destroyBuffer(vkHandle);
			vkHandle = vk::Buffer();
		}

	}
	tSampler::~tSampler() {
		if (vksampler) {
			device->destroySampler(vksampler);
			vksampler = vk::Sampler();
		}
	}
	tImageView::~tImageView() {
		if (view) {

			device->destroyImageView(view);
			view = vk::ImageView();

		}
		if (depth_view) {
			device->destroyImageView(depth_view);
		}
		if (stencil_view) {
			device->destroyImageView(stencil_view);
		}
		if (unorm_view) {
			device->destroyImageView(unorm_view);
		}
		if (srgb_view) {
			device->destroyImageView(srgb_view);
		}
		for (auto& view : render_target_views) {
			device->destroyImageView(view);
		}
	}
	tImage::~tImage() {

		if (vkImage) {
			if (owns_image) {
				device->destroyImage(vkImage);
			}
			if (alloc != nullptr) {
				device->freeAllocation(alloc);

			}

			vkImage = vk::Image();

		}

	}
	void updateBufferUsingStageBuffer(const Device* device, BufferHandle& handle, CommandBufferHandle& cb, const void* data, size_t size, size_t offset) {
		auto& create_info = handle->getCreateInfo();

		if (create_info.domain == BufferDomain::Device && !memory_type_is_host_visible(device->getPhysicalDevice(), getMemoryTypeIdx(handle->getAllocation())))
		{
			//	assert(cb != nullptr && "Device memory must have commandBufer");
			auto staging_info = create_info;
			staging_info.domain = BufferDomain::Host;
			staging_info.size = size;
			auto staging_buffer = createBuffer(device,staging_info, data);
			vk::BufferCopy region;
			region.setSize(create_info.size);
			region.setSrcOffset(0);
			region.setDstOffset(offset);
			CommandBufferHandle cmd;// = cb;
			if (cb != nullptr) {
				cmd = cb;

				cmd->copyBuffer(staging_buffer, handle, region);
			}
			else {
				cmd = device->requestTransientCommandBuffer();
				oneTimeSubmit(cmd, device->requestQueue(cmd->getQueueFamilyIdx()), [&](CommandBufferHandle& cmd) {cmd->copyBuffer(staging_buffer, handle, region); });
			}





		}
		else {

			memcpy(static_cast<uint8_t*>(getMappedData(handle->getAllocation())) + offset, data, size);
			handle->Flush();
		}

	}
	void fillBufferUsingStageBuffer(const Device* device, BufferHandle& handle, CommandBufferHandle& cb, uint32_t zero, size_t size, size_t offset) {
		auto& create_info = handle->getCreateInfo();

		if (create_info.domain == BufferDomain::Device && !memory_type_is_host_visible(device->getPhysicalDevice(), getMemoryTypeIdx(handle->getAllocation())))
		{
			assert(cb != nullptr && "Device memory must have commandBufer");
			CommandBufferHandle cmd = cb;


			auto staging_info = create_info;
			if (cb != nullptr) {
				cmd->fillBuffer(handle, offset, zero, size);
			}
			else {
				cmd = device->requestTransientCommandBuffer();
				oneTimeSubmit(cmd, device->requestQueue(cmd->getQueueFamilyIdx()), [&](CommandBufferHandle& cmd) {cmd->fillBuffer(handle, offset, zero, size); });
			}

		}
		else {
			//assert(static_cast<uint8_t*>(handle->getAllocation()->GetMappedData()));

			memset(static_cast<uint8_t*>(getMappedData(handle->getAllocation())) + offset, zero, size);
			handle->Flush();
		}
	}

	tShader::~tShader() {
		if (shaderModule.size() > 0) {

			for (int i = 0; i < shaderModule.size(); ++i) {
				if (shaderModule[i]) {
					device->destroyShaderModule(shaderModule[i]);
					shaderModule[i] = vk::ShaderModule();
				}
			}
		}

	}
	void tShader::CreateShaderLayout() {
		auto& d = device;
		if (setAllocator.size() == 0) {
			for (int i = 0; i < setInfos.size(); ++i) {
				setAllocator.emplace_back(tDescriptorSetAllocatorManager::manager.requestSetAllocator(d, setInfos[i].data));
			}
		}
		if (pipelinelayout == nullptr) {
			std::vector<DescriptorSetLayoutHandle> layouts;
			for (auto& set : setAllocator) {
				layouts.emplace_back(set->getLayout());
			}
			pipelinelayout = createPipelineLayout(d,layouts, pushConstant, allstageFlags);

		}
		isCreate = true;
	}
	void tShader::AddShaderModule(std::string fileName, vk::ShaderStageFlagBits stageFlag) {
		assert(!isCreate && "Can't change shader after set it to material");
		if (isCreate)throw("Can't change shader after set it to material");
		pipelinelayout = nullptr;
		setAllocator.clear();
		auto& d = device;
		shaderAsset.push_back(LoadShader(fileName));
		Reflector::reflectionShader(shaderAsset.back()->shaderReflection, setInfos, pushConstant, stageFlag);
		allstageFlags |= stageFlag;
		//To check stage Flags
		vk::ShaderModuleCreateInfo info;
		info.setCode(shaderAsset.back()->shaderSource);
		shaderModule.push_back(d->createShaderModule(info));
		shaderStage.emplace_back(stageFlag);
	}
	tDescriptorSetLayout::~tDescriptorSetLayout() {
		if (vkLayout) {
			device->destroyDescriptorSetLayout(vkLayout);
			vkLayout = vk::DescriptorSetLayout();
		}
	}
	bool BindingResourceInfo::operator==(const BindingResourceInfo& info)const {
		bool equal = dstBinding == info.dstBinding && dstArrayElement == info.dstArrayElement && type == info.type && view == info.view && sampler == info.sampler
			&& range == info.range;
		if (!equal)return false;

		if (buffer && info.buffer) {
			return  buffer->getVkHandle() == info.buffer->getVkHandle();
		}
		else if (image && info.image) {
			return image->getVkHandle() == info.image->getVkHandle();
		}
		else return true;


	}
	tDescriptorPool::~tDescriptorPool() {
		if (pool) {
			device->destroyDescriptorPool(pool);
		}
	}
	std::shared_ptr<tDescriptorSet> tDescriptorSetAllocator::requestDescriptorSet(const ResSetBinding& rb) {
		if (rb.size() == 0)return nullptr;

		auto re = descriptorSetPool.request(rb);
		if (re != nullptr) {
			return re;
		}
		//Only need rebind
		if (descriptorSetPool.isFull()) {
			re = descriptorSetPool.moveLastToFront(rb);
		}
		else {//Create a new descriptorSet
			LOGD(LogLevel::Performance, "Allocate descriptorSet");
			vk::DescriptorSetAllocateInfo descSetInfo;
			descSetInfo.setDescriptorPool(pool->getVkHandle());
			descSetInfo.setSetLayouts(layout->getVkHandle());
			auto set = device->allocateDescriptorSets(descSetInfo);
			re = descriptorSetPool.allocate(rb, device, pool, set[0], rb);
		}



		std::vector<vk::WriteDescriptorSet> write;
		//allocate enough memory for vector, we need use address of them
		std::vector< vk::DescriptorBufferInfo> bufferInfo;
		bufferInfo.reserve(rb.size());
		//bufferInfo.reserve(rb.getBuffers().size());
		std::vector<vk::DescriptorImageInfo> imageInfo;
		imageInfo.reserve(rb.size());
		//imageInfo.reserve(rb.getImages().size());
		for (auto& bindingInfo : rb) {
			if (bindingInfo.emptyResource())continue;
			auto& resource = bindingInfo;
			switch (resource.type) {
			case vk::DescriptorType::eUniformBufferDynamic:
			case vk::DescriptorType::eStorageBufferDynamic:
				//assert(resource.offset == 0);
				bufferInfo.emplace_back(vk::DescriptorBufferInfo(resource.buffer->getVkHandle(), 0, resource.range));
				;
				write.emplace_back(vk::WriteDescriptorSet(re->getVkHandle(), resource.dstBinding, resource.dstArrayElement, 1, resource.type, nullptr, &bufferInfo.back(), nullptr));
				break;
			case vk::DescriptorType::eCombinedImageSampler:
			case vk::DescriptorType::eStorageImage:
				imageInfo.emplace_back(device->getSampler(resource.sampler)->getVkHandle(), resource.view, vk::ImageLayout::eShaderReadOnlyOptimal);
				write.emplace_back(vk::WriteDescriptorSet(re->getVkHandle(), resource.dstBinding, resource.dstArrayElement, 1, resource.type, &imageInfo.back(), nullptr, nullptr));
				break;
			default:assert(false && "can't support other type for now");
			}
		}

		if (write.size() != 0) {
			LOGD(LogLevel::Performance, "Rebind descriptorSet");
			device->updateDescriptorSets(write, {});
		}
		return re;
	}

	SamplerHandle createSampler(weakDevice device, const SamplerCreateInfo& sampler_info) {
		auto info = fill_vk_sampler_info(sampler_info);
		VkSampler sampler = device->createSampler(info);
		SamplerHandle handle = std::make_shared<tSampler>(device, sampler, sampler_info);
		return handle;
	}



	class ImageResourceHolder
	{
	public:
		explicit ImageResourceHolder(const Device* device_)
			: device(device_)

		{
		}

		~ImageResourceHolder()
		{
			if (owned)
				cleanup();
		}

		const Device* device;
		//	const VolkDeviceTable& table;

		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkImageView image_view = VK_NULL_HANDLE;
		VkImageView depth_view = VK_NULL_HANDLE;
		VkImageView stencil_view = VK_NULL_HANDLE;
		VkImageView unorm_view = VK_NULL_HANDLE;
		VkImageView srgb_view = VK_NULL_HANDLE;
		VkImageViewType default_view_type = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
		std::vector<VkImageView> rt_views;
		VmaAllocation allocation;
		VmaAllocator* allocator = nullptr;
		bool owned = true;

		VkImageViewType get_default_view_type() const
		{
			return default_view_type;
		}



		bool create_default_views(const ImageCreateInfo& create_info, const VkImageViewCreateInfo* view_info,
			bool create_unorm_srgb_views = false, const VkFormat* view_formats = nullptr)
		{
			//	VkDevice vkdevice = devic();

			if ((create_info.usage & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) == 0)
			{
				LOGI("Cannot create image view unless certain usage flags are present.\n");
				return false;
			}

			VkImageViewCreateInfo default_view_info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			VkSamplerYcbcrConversionInfo conversion_info = { VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO };

			if (!view_info)
			{
				default_view_info.image = image;
				default_view_info.format = create_info.format;
				default_view_info.components = create_info.swizzle;
				default_view_info.subresourceRange.aspectMask = format_to_aspect_mask(default_view_info.format);
				default_view_info.viewType = get_image_view_type(create_info, nullptr);
				default_view_info.subresourceRange.baseMipLevel = 0;
				default_view_info.subresourceRange.baseArrayLayer = 0;
				default_view_info.subresourceRange.levelCount = create_info.levels;
				default_view_info.subresourceRange.layerCount = create_info.layers;

				default_view_type = default_view_info.viewType;
			}
			else
				default_view_info = *view_info;

			view_info = &default_view_info;


			if (!create_alt_views(create_info, *view_info))
				return false;

			if (!create_render_target_views(create_info, *view_info))
				return false;

			if (!create_default_view(*view_info))
				return false;

			if (create_unorm_srgb_views)
			{
				auto info = *view_info;

				info.format = view_formats[0];
				unorm_view = device->createImageView(*view_info);


				info.format = view_formats[1];
				srgb_view = device->createImageView(*view_info);
			}

			return true;
		}

	private:
		bool create_render_target_views(const ImageCreateInfo& image_create_info, const VkImageViewCreateInfo& info)
		{
			rt_views.reserve(info.subresourceRange.layerCount);

			if (info.viewType == VK_IMAGE_VIEW_TYPE_3D)
				return true;

			// If we have a render target, and non-trivial case (layers = 1, levels = 1),
			// create an array of render targets which correspond to each layer (mip 0).
			if ((image_create_info.usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) != 0 &&
				((info.subresourceRange.levelCount > 1) || (info.subresourceRange.layerCount > 1)))
			{
				auto view_info = info;
				view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
				view_info.subresourceRange.baseMipLevel = info.subresourceRange.baseMipLevel;
				for (uint32_t layer = 0; layer < info.subresourceRange.layerCount; layer++)
				{
					view_info.subresourceRange.levelCount = 1;
					view_info.subresourceRange.layerCount = 1;
					view_info.subresourceRange.baseArrayLayer = layer + info.subresourceRange.baseArrayLayer;

					VkImageView rt_view;
					rt_view = device->createImageView(view_info);


					rt_views.push_back(rt_view);
				}
			}

			return true;
		}

		bool create_alt_views(const ImageCreateInfo& image_create_info, const VkImageViewCreateInfo& info)
		{
			if (info.viewType == VK_IMAGE_VIEW_TYPE_CUBE ||
				info.viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY ||
				info.viewType == VK_IMAGE_VIEW_TYPE_3D)
			{
				return true;
			}

			//VkDevice vkdevice = device->get_device();

			if (info.subresourceRange.aspectMask == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))
			{
				if ((image_create_info.usage & ~VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0)
				{
					// Sanity check. Don't want to implement layered views for this.
					if (info.subresourceRange.levelCount > 1)
					{
						LOGI("Cannot create depth stencil attachments with more than 1 mip level currently, and non-DS usage flags.\n");
						return false;
					}

					if (info.subresourceRange.layerCount > 1)
					{
						LOGI("Cannot create layered depth stencil attachments with non-DS usage flags.\n");
						return false;
					}

					auto view_info = info;

					// We need this to be able to sample the texture, or otherwise use it as a non-pure DS attachment.
					view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
					depth_view = device->createImageView(view_info);


					view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
					stencil_view = device->createImageView(view_info);
				}
			}

			return true;
		}

		bool create_default_view(const VkImageViewCreateInfo& info)
		{
			//VkDevice vkdevice = device->get_device();
			// Create the normal image view. This one contains every subresource.
			image_view = device->createImageView(info);



			return true;
		}

		void cleanup()
		{


			if (image_view)
				device->destroyImageView(image_view);

			if (depth_view)device->destroyImageView(depth_view);

			if (stencil_view)device->destroyImageView(stencil_view);

			if (unorm_view)
				device->destroyImageView(unorm_view);
			if (srgb_view)
				device->destroyImageView(srgb_view);
			for (auto& view : rt_views)
				device->destroyImageView(view);

			if (image)
				device->destroyImage(image);
			if (memory)
				FreeMemory(*allocator, allocation);

		}
	};

	ImageHandle create_image_from_staging_buffer(const Device* device, const ImageCreateInfo& create_info, const InitialImageBuffer* staging_buffer, CommandBufferHandle cb) {

		auto physicalDevice = device->getPhysicalDevice();

		VkImageCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		info.format = create_info.format;
		info.extent.width = create_info.width;
		info.extent.height = create_info.height;
		info.extent.depth = create_info.depth;
		info.imageType = create_info.type;
		info.mipLevels = create_info.levels;
		info.arrayLayers = create_info.layers;
		info.samples = create_info.samples;

		if (create_info.domain == ImageDomain::LinearHostCached || create_info.domain == ImageDomain::LinearHost)
		{
			info.tiling = VK_IMAGE_TILING_LINEAR;
			info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		}
		else
		{
			info.tiling = VK_IMAGE_TILING_OPTIMAL;
			info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}

		info.usage = create_info.usage;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (create_info.domain == ImageDomain::Transient)
			info.usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
		if (staging_buffer)
			info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		info.flags = create_info.flags;

		if (info.mipLevels == 0)
			info.mipLevels = image_num_miplevels(info.extent);


		if ((create_info.usage & VK_IMAGE_USAGE_STORAGE_BIT) ||
			(create_info.misc & IMAGE_MISC_MUTABLE_SRGB_BIT))
		{
			info.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
		}

		if (info.tiling == VkImageTiling(VK_IMAGE_TILING_LINEAR))
		{
			assert(info.mipLevels == 1 && info.arrayLayers == 1 && info.imageType == VkImageType::VK_IMAGE_TYPE_2D && info.samples == VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT);

			// Do some more stringent checks.
		}

		// Only do this conditionally.
		// On AMD, using CONCURRENT with async compute disables compression.
		uint32_t sharing_indices[3] = {};

		uint32_t queue_flags = create_info.misc & (IMAGE_MISC_CONCURRENT_QUEUE_GRAPHICS_BIT |
			IMAGE_MISC_CONCURRENT_QUEUE_ASYNC_COMPUTE_BIT |
			IMAGE_MISC_CONCURRENT_QUEUE_ASYNC_GRAPHICS_BIT |
			IMAGE_MISC_CONCURRENT_QUEUE_ASYNC_TRANSFER_BIT);
		bool concurrent_queue = queue_flags != 0;
		assert(!concurrent_queue && "Don't want to support concurrent_queue");
		if (concurrent_queue)
		{
			info.sharingMode = VK_SHARING_MODE_CONCURRENT;

			const auto add_unique_family = [&](uint32_t family) {
				for (uint32_t i = 0; i < info.queueFamilyIndexCount; i++)
				{
					if (sharing_indices[i] == family)
						return;
				}
				sharing_indices[info.queueFamilyIndexCount++] = family;
			};

			if (queue_flags & (IMAGE_MISC_CONCURRENT_QUEUE_GRAPHICS_BIT | IMAGE_MISC_CONCURRENT_QUEUE_ASYNC_GRAPHICS_BIT))
				add_unique_family(physicalDevice.graphicsQueuefamilyId);
			if (queue_flags & IMAGE_MISC_CONCURRENT_QUEUE_ASYNC_COMPUTE_BIT)
				add_unique_family(physicalDevice.computeQueuefamilyId);
			if (staging_buffer || (queue_flags & IMAGE_MISC_CONCURRENT_QUEUE_ASYNC_TRANSFER_BIT) != 0)
				add_unique_family(physicalDevice.transferQueuefamilyId);
			if (staging_buffer)
				add_unique_family(physicalDevice.graphicsQueuefamilyId);

			if (info.queueFamilyIndexCount > 1)
				info.pQueueFamilyIndices = sharing_indices;
			else
			{
				info.pQueueFamilyIndices = nullptr;
				info.queueFamilyIndexCount = 0;
				info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			}
		}

		VkFormatFeatureFlags check_extra_features = 0;
		if ((create_info.misc & IMAGE_MISC_VERIFY_FORMAT_FEATURE_SAMPLED_LINEAR_FILTER_BIT) != 0)
			check_extra_features |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

		if (info.tiling == VK_IMAGE_TILING_LINEAR)
		{
			if (staging_buffer)
				return ImageHandle(nullptr);

			// Do some more stringent checks.
			if (info.mipLevels > 1)
				return ImageHandle(nullptr);
			if (info.arrayLayers > 1)
				return ImageHandle(nullptr);
			if (info.imageType != VK_IMAGE_TYPE_2D)
				return ImageHandle(nullptr);
			if (info.samples != VK_SAMPLE_COUNT_1_BIT)
				return ImageHandle(nullptr);


		}
		if ((create_info.flags & VK_IMAGE_CREATE_EXTENDED_USAGE_BIT) == 0 &&
			(!imageFormat_is_supported(physicalDevice.physicalDevice, create_info.format, image_usage_to_features(info.usage) | check_extra_features, info.tiling)))
		{
			LOGI("Format %u is not supported for usage flags!\n", unsigned(create_info.format));
			return ImageHandle(nullptr);
		}
		VmaAllocationCreateInfo allocCreateInfo = {};
		ImageResourceHolder holder(device);
		VmaAllocationInfo allocinfo;
		find_memory_type(create_info.domain, allocCreateInfo);
		vmaCreateImage(device->getAllocator(), &info, &allocCreateInfo, &holder.image, &holder.allocation, &allocinfo);

		auto tmpinfo = create_info;
		tmpinfo.usage = info.usage;
		tmpinfo.flags = info.flags;
		tmpinfo.levels = info.mipLevels;

		bool has_view = (info.usage & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) != 0;


		VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
		if (has_view)
		{
			if (!holder.create_default_views(tmpinfo, nullptr, false, nullptr))
				return ImageHandle(nullptr);
			view_type = holder.get_default_view_type();
		}
		ImageHandle handle = std::make_shared<tImage>(device, holder.image, holder.image_view, holder.allocation, tmpinfo, view_type);
		if (handle)
		{
			holder.owned = false;
			if (has_view)
			{
				handle->get_view()->set_alt_views(holder.depth_view, holder.stencil_view);
				handle->get_view()->set_render_target_views(move(holder.rt_views));
				handle->get_view()->set_unorm_view(holder.unorm_view);
				handle->get_view()->set_srgb_view(holder.srgb_view);
			}

			// Set possible dstStage and dstAccess.
			handle->set_stage_flags(image_usage_to_possible_stages(info.usage));
			handle->set_access_flags(image_usage_to_possible_access(info.usage));
		}
		if (staging_buffer) {

			//VK_ASSERT(cb != nullptr && "Need commandBuffer to uploading data");
			VK_ASSERT(create_info.domain != ImageDomain::Transient);
			VK_ASSERT(create_info.initial_layout != VK_IMAGE_LAYOUT_UNDEFINED);
			auto stagingToBuffer = [&](CommandBufferHandle& cb) {
				bool generate_mips = (create_info.misc & IMAGE_MISC_GENERATE_MIPS_BIT) != 0;
				setImageLayout(cb, handle, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
				cb->copyBufferToImage(staging_buffer->buffer, handle, vk::ImageLayout::eTransferDstOptimal, staging_buffer->blits);
				if (generate_mips) {
					barrier_prepare_generate_mipmap(cb, handle, vk::ImageLayout::eTransferDstOptimal,
						vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferWrite, true);
					generateMipmap(cb, handle);
				}
				setImageLayout(cb, handle,
					generate_mips ? vk::ImageLayout::eTransferSrcOptimal : vk::ImageLayout::eTransferDstOptimal,
					(vk::ImageLayout)create_info.initial_layout);
			};
			if (cb == nullptr) {
				auto cmd = device->requestTransientCommandBuffer();
				oneTimeSubmit(cmd, device->requestQueue(cmd->getQueueFamilyIdx()), stagingToBuffer);
			}
			else {
				stagingToBuffer(cb);
			}



		}
		else if (create_info.initial_layout != VK_IMAGE_LAYOUT_UNDEFINED) {
			if (cb == nullptr) {
				auto cmd = device->requestTransientCommandBuffer();
				oneTimeSubmit(cmd, device->requestQueue(cmd->getQueueFamilyIdx()), [&](CommandBufferHandle cb) {setImageLayout(cb, handle,
					vk::ImageLayout::eUndefined,
					(vk::ImageLayout)create_info.initial_layout); });
			}
			else {
				setImageLayout(cb, handle,
					vk::ImageLayout::eUndefined,
					(vk::ImageLayout)create_info.initial_layout);
			}
		}
		return handle;

	}
	InitialImageBuffer create_image_staging_buffer(const Device* device, const ImageCreateInfo& info, const ImageAsset* initial) {
		InitialImageBuffer result;
		assert(info.layers == 1 && "Can't support image array for reading image from file");
		bool generate_mips = (info.misc & IMAGE_MISC_GENERATE_MIPS_BIT) != 0;
		TextureFormatLayout layout;

		unsigned copy_levels;
		if (generate_mips)
			copy_levels = 1;
		else if (info.levels == 0)
			copy_levels = TextureFormatLayout::num_miplevels(info.width, info.height, info.depth);
		else
			copy_levels = info.levels;

		switch (info.type)
		{
		case VK_IMAGE_TYPE_1D:
			layout.set_1d(info.format, info.width, info.layers, copy_levels);
			break;
		case VK_IMAGE_TYPE_2D:
			layout.set_2d(info.format, info.width, info.height, info.layers, copy_levels);
			break;
		case VK_IMAGE_TYPE_3D:
			layout.set_3d(info.format, info.width, info.height, info.depth, copy_levels);
			break;
		default:
			return {};
		}

		BufferCreateInfo buffer_info = {};
		buffer_info.domain = BufferDomain::Host;
		buffer_info.size = layout.get_required_size();
		buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		result.buffer = createBuffer(device,buffer_info, nullptr);
		//set_name(*result.buffer, "image-upload-staging-buffer");

		// And now, do the actual copy.

		auto* mapped = static_cast<uint8_t*>(getMappedData(result.buffer->getAllocation()));
		unsigned index = 0;

		layout.set_buffer((void*)mapped, layout.get_required_size());

		for (unsigned level = 0; level < copy_levels; level++)
		{
			const auto& mip_info = layout.get_mip_info(level);
			uint32_t dst_height_stride = layout.get_layer_size(level);
			size_t row_size = layout.get_row_size(level);

			for (unsigned layer = 0; layer < info.layers; layer++, index++)
			{
				uint32_t src_row_length = mip_info.row_length;
				uint32_t src_array_height = mip_info.image_height;

				uint32_t src_row_stride = layout.row_byte_stride(src_row_length);
				uint32_t src_height_stride = layout.layer_byte_stride(src_array_height, src_row_stride);

				uint8_t* dst = static_cast<uint8_t*>(layout.data(layer, level));
				const uint8_t* src = static_cast<const uint8_t*>(initial[index].pixels);

				for (uint32_t z = 0; z < mip_info.depth; z++)
					for (uint32_t y = 0; y < mip_info.block_image_height; y++)
						memcpy(dst + z * dst_height_stride + y * row_size, src + z * src_height_stride + y * src_row_stride, row_size);
			}
		}

		//	unmap_host_buffer(*result.buffer, MEMORY_ACCESS_WRITE_BIT);
		layout.build_buffer_image_copies(result.blits);

		return result;
	}
	PipelineLayoutHandle createPipelineLayout(const Device* device, std::vector<DescriptorSetLayoutHandle>& descLayouts, GpuBlockBuffer& pushConstant, vk::ShaderStageFlags shaderStage) {
		vk::PipelineLayoutCreateInfo  info = {};

		std::vector<vk::PushConstantRange> ranges(pushConstant.size());

		for (int idx = 0; idx < ranges.size(); ++idx) {
			auto& pus = pushConstant[idx];
			ranges[idx].setOffset(pus.offset);
			ranges[idx].setSize(pus.size);
			ranges[idx].setStageFlags(shaderStage);
		}
		info.setPushConstantRanges(ranges);
		std::vector<vk::DescriptorSetLayout> layouts(descLayouts.size());
		for (int i = 0; i < layouts.size(); ++i) {
			layouts[i] = descLayouts[i]->getVkHandle();
		}
		info.setSetLayouts(layouts);
		auto vklayout = device->createPipelineLayout(info);
		return std::make_shared<tPipelineLayout>(device, vklayout);
	}
	InitialImageBuffer create_image_staging_buffer(const Device* device,const TextureFormatLayout& layout) {
		InitialImageBuffer result;

		BufferCreateInfo buffer_info = {};
		buffer_info.domain = BufferDomain::Host;
		buffer_info.size = layout.get_required_size();
		buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		result.buffer = createBuffer(device,buffer_info, nullptr);
		//set_name(*result.buffer, "image-upload-staging-buffer");

		auto* mapped = getMappedData(result.buffer->getAllocation());
		memcpy(mapped, layout.data(), layout.get_required_size());

		//unmap_host_buffer(*result.buffer, MEMORY_ACCESS_WRITE_BIT);

		layout.build_buffer_image_copies(result.blits);
		return result;
	}
	BufferHandle createBuffer(const Device* device, BufferCreateInfo& create_info, const void* initial, CommandBufferHandle cb) {
		vk::Buffer buffer;
		//VkMemoryRequirements reqs;
		//VmaAllocation allocation;
		auto physicalDevice = device->getPhysicalDevice();
		bool zero_initialize = (create_info.misc & BUFFER_MISC_ZERO_INITIALIZE_BIT) != 0;
		if (initial && zero_initialize)
		{
			LOGI("Cannot initialize buffer with data and clear.\n");
			return BufferHandle{};
		}

		VkBufferCreateInfo info = static_cast<VkBufferCreateInfo>(vk::BufferCreateInfo());// = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

		if (create_info.usage & (VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) != 0) {
			create_info.size = align(create_info.size, minBufferAlignment(physicalDevice));
		}
		else {
			if ((create_info.usage & VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) != 0) {
				create_info.size = std::max(create_info.size, align(create_info.size, physicalDevice.getProperties().limits.minStorageBufferOffsetAlignment));
			}
			else if ((create_info.usage & VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) != 0) {
				create_info.size = std::max(create_info.size, align(create_info.size, physicalDevice.getProperties().limits.minUniformBufferOffsetAlignment));
			}
		}
		info.size = create_info.size;

		info.usage = create_info.usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		uint32_t sharing_indices[3];
		if (physicalDevice.bUniqueQueueFamily()) {
			fill_buffer_sharing_indices(physicalDevice, info, sharing_indices);
		}


		VmaAllocationCreateInfo allocCreateInfo = {};

		allocCreateInfo.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VkBuffer vkBuffer;
		VmaAllocation allocation;
		VmaAllocationInfo allocinfo;
		find_memory_type(create_info.domain, allocCreateInfo);
		vmaCreateBuffer(device->getAllocator(), &info, &allocCreateInfo, &vkBuffer, &allocation, &allocinfo);

		BufferHandle handle = std::make_shared<tBuffer>(device, vkBuffer, allocation, create_info);

		if (initial) {
			updateBufferUsingStageBuffer(device, handle, cb, initial, create_info.size, 0);
		}
		else if (zero_initialize) {
			fillBufferUsingStageBuffer(device, handle, cb, (uint32_t)0, create_info.size, 0);
		}
		return handle;
	}
	//If need commandBuffer but don't provide, use transient


	ImageHandle createImage(const Device* device, const ImageCreateInfo& info, std::shared_ptr<ImageAsset> initial, CommandBufferHandle cb) {
		if (initial) {
			auto staging_buffer = create_image_staging_buffer(device,info, initial.get());
			return create_image_from_staging_buffer(device,info, &staging_buffer, cb);
		}
		else {
			return create_image_from_staging_buffer(device,info, nullptr, cb);
		}
	}
	tSwapChain::~tSwapChain() {
		if (swapChain) {

			device->destroySwapchainKHR(swapChain);
		}
	}
	SwapChainHandle createSwapChain(Device* device, vk::SurfaceKHR const& surface, vk::Extent2D const& extent, vk::ImageUsageFlags usage, vk::SwapchainKHR const& oldSwapChain, uint32_t graphicsQueueFamilyIndex, uint32_t presentQueueFamilyIndex) {
		using vk::su::clamp;
		vk::SurfaceFormatKHR surfaceFormat = vk::su::pickSurfaceFormat(device->getPhysicalDevice().physicalDevice.getSurfaceFormatsKHR(surface));
		vk::SurfaceCapabilitiesKHR surfaceCapabilities = device->getPhysicalDevice().physicalDevice.getSurfaceCapabilitiesKHR(surface);
		VkExtent2D                 swapchainExtent;
		if (surfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
		{
			// If the surface size is undefined, the size is set to the size of the images requested.
			swapchainExtent.width =
				clamp(extent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
			swapchainExtent.height =
				clamp(extent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
		}
		else
		{
			// If the surface size is defined, the swap chain size must match
			swapchainExtent = surfaceCapabilities.currentExtent;
		}
		vk::SurfaceTransformFlagBitsKHR preTransform =
			(surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
			? vk::SurfaceTransformFlagBitsKHR::eIdentity
			: surfaceCapabilities.currentTransform;
		vk::CompositeAlphaFlagBitsKHR compositeAlpha =
			(surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)
			? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
			: (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)
			? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
			: (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit)
			? vk::CompositeAlphaFlagBitsKHR::eInherit
			: vk::CompositeAlphaFlagBitsKHR::eOpaque;
		vk::PresentModeKHR presentMode = vk::su::pickPresentMode(device->getPhysicalDevice().physicalDevice.getSurfacePresentModesKHR(surface));
		vk::SwapchainCreateInfoKHR swapChainCreateInfo({},
			surface,
			surfaceCapabilities.minImageCount,
			surfaceFormat.format,
			surfaceFormat.colorSpace,
			swapchainExtent,
			1,
			usage,
			vk::SharingMode::eExclusive,
			{},
			preTransform,
			compositeAlpha,
			presentMode,
			true,
			oldSwapChain);

		if (graphicsQueueFamilyIndex != presentQueueFamilyIndex)
		{
			uint32_t queueFamilyIndices[2] = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };
			// If the graphics and present queues are from different queue families, we either have to explicitly transfer
			// ownership of images between the queues, or we have to create the swapchain with imageSharingMode as
			// vk::SharingMode::eConcurrent
			swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			swapChainCreateInfo.queueFamilyIndexCount = 2;
			swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		auto swapChain = device->createSwapchainKHR(swapChainCreateInfo);
		auto swapchain_images = device->getSwapchainImagesKHR(swapChain);
		ImageCreateInfo info = ImageCreateInfo::render_target(swapchainExtent.width, swapchainExtent.height, (VkFormat)surfaceFormat.format);
		info.usage = (VkImageUsageFlags)usage;


		std::vector<ImageHandle> images;
		for (auto& image : swapchain_images)
		{
			VkImageViewCreateInfo view_info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			view_info.image = image;
			view_info.format = (VkFormat)surfaceFormat.format;
			view_info.components.r = VK_COMPONENT_SWIZZLE_R;
			view_info.components.g = VK_COMPONENT_SWIZZLE_G;
			view_info.components.b = VK_COMPONENT_SWIZZLE_B;
			view_info.components.a = VK_COMPONENT_SWIZZLE_A;
			view_info.subresourceRange.aspectMask = format_to_aspect_mask((VkFormat)surfaceFormat.format);
			view_info.subresourceRange.baseMipLevel = 0;
			view_info.subresourceRange.baseArrayLayer = 0;
			view_info.subresourceRange.levelCount = 1;
			view_info.subresourceRange.layerCount = 1;
			view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;



			VkImageView image_view = device->createImageView((vk::ImageViewCreateInfo)view_info);

			auto backbuffer = std::make_shared<tImage>(device, image, image_view, nullptr, info, VkImageViewType::VK_IMAGE_VIEW_TYPE_2D);
			backbuffer->set_swapchain_layout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
			backbuffer->ownsImage(false);
			images.push_back(backbuffer);
		}
		SwapChainHandle handle = std::make_shared<tSwapChain>(device, swapChain, swapchainExtent);

		handle->setImages(images);
		return handle;
	}
}