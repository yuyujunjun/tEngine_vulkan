
#include"tDevice.h"

#include"tResource.h"
#include"CommandBufferBase.h"
#include"tAsset.h"
#include"tFenceSemaphore.h"
namespace tEngine {
	using namespace std;
	void Device::clearDeviceObject(){
		for (auto& sampler : samplers) {
			sampler.reset();
		}
		//transientCb.reset();
		swapChain.reset();
		transientPool.reset();
		destroyPipelineCache(pipelineCache);
		vmaDestroyAllocator(allocator);
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
		vector<VkImageView> rt_views;
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
				unorm_view=device->createImageView(*view_info);
			

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

	uint32_t Device::getQueueId(vk::QueueFlagBits queueType)const {
		auto queueFamilyProperties = physicalDevice.getPhysicalDevice().getQueueFamilyProperties();
		return findQueueFamilyIndex(queueFamilyProperties, queueType);
	}
	Device::Device(VkDevice device, vk::Instance instance, vk::Extent2D extent):vk::Device(device),instance(instance) {

		//physical Device
		auto phyDevice = instance.enumeratePhysicalDevices().front();
		physicalDevice.SetPhysicalDevice(phyDevice);
		
		//Allocator
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
		allocatorInfo.physicalDevice = phyDevice;
		allocatorInfo.device = device;
		allocatorInfo.instance = instance;
		vmaCreateAllocator(&allocatorInfo, &allocator);
		//window
		glfwInit();
		glfwSetErrorCallback([](int error, const char* msg) {
			std::cerr << "glfw: "
				<< "(" << error << ") " << msg << std::endl;
			});


		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		gWindow = glfwCreateWindow(extent.width, extent.height, "Tgine", nullptr, nullptr);

		//surface
		VkSurfaceKHR _surface;
		VkResult err = glfwCreateWindowSurface(static_cast<VkInstance>(instance), gWindow, nullptr, &_surface);
		if (err != VK_SUCCESS)
			throw std::runtime_error("Failed to create window!");
		surface = vk::SurfaceKHR(_surface);
		
		
		
		
		
		
		vk::PipelineCacheCreateInfo info;
		pipelineCache=createPipelineCache(info);

		this->initStockSamplers();
		auto pair = vk::su::findGraphicsAndPresentQueueFamilyIndex(physicalDevice.getPhysicalDevice(), surface);
		this->getPhysicalDevice().graphicsQueuefamilyId = pair.first;
		this->getPhysicalDevice().presentQueuefamilyId = pair.second;
		auto queueFamilyProperties = physicalDevice.getPhysicalDevice().getQueueFamilyProperties();
		this->getPhysicalDevice().computeQueuefamilyId = findQueueFamilyIndex(queueFamilyProperties, vk::QueueFlagBits::eCompute);
		this->getPhysicalDevice().transferQueuefamilyId = findQueueFamilyIndex(queueFamilyProperties, vk::QueueFlagBits::eTransfer);

		this->queues[this->getPhysicalDevice().graphicsQueuefamilyId] = this->getQueue(this->getPhysicalDevice().graphicsQueuefamilyId, 0);
		this->queues[this->getPhysicalDevice().computeQueuefamilyId] = this->getQueue(this->getPhysicalDevice().computeQueuefamilyId, 0);
		this->queues[this->getPhysicalDevice().presentQueuefamilyId] = this->getQueue(this->getPhysicalDevice().presentQueuefamilyId, 0);
		this->swapChain = createSwapChain(this, surface,extent, vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst, vk::SwapchainKHR(), this->getPhysicalDevice().graphicsQueuefamilyId, this->getPhysicalDevice().presentQueuefamilyId);




		vk::CommandPoolCreateInfo poolInfo = {};
		poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eTransient| vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
		poolInfo.setQueueFamilyIndex(physicalDevice.graphicsQueuefamilyId);
		auto vkPool = createCommandPool(poolInfo);
		transientPool = std::make_shared<tCommandPool>(this,vkPool,physicalDevice.graphicsQueuefamilyId);
		
		fenceManager = std::make_unique<FenceManager>(this);
		semaphoreManager = std::make_unique<SemaphoreManager>(this);
	}

	BufferHandle Device::createBuffer(const BufferCreateInfo& create_info, const void* initial, CommandBufferHandle cb)const {
		vk::Buffer buffer;
		//VkMemoryRequirements reqs;
		//VmaAllocation allocation;

		bool zero_initialize = (create_info.misc & BUFFER_MISC_ZERO_INITIALIZE_BIT) != 0;
		if (initial && zero_initialize)
		{
			LOGI("Cannot initialize buffer with data and clear.\n");
			return BufferHandle{};
		}

		VkBufferCreateInfo info=static_cast<VkBufferCreateInfo>(vk::BufferCreateInfo());// = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.size = create_info.size;
		info.usage = create_info.usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		uint32_t sharing_indices[3];
		if (physicalDevice.bUniqueQueueFamily()) {
			fill_buffer_sharing_indices(physicalDevice,info, sharing_indices);
		}
	

		VmaAllocationCreateInfo allocCreateInfo = {};

		allocCreateInfo.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT;
		
		VkBuffer vkBuffer;
		VmaAllocation allocation;
		VmaAllocationInfo allocinfo;
		find_memory_type(create_info.domain, allocCreateInfo);
		vmaCreateBuffer(allocator, &info, &allocCreateInfo, &vkBuffer, &allocation, &allocinfo);
		
		BufferHandle handle = std::make_shared<tBuffer>(this,vkBuffer,allocation,create_info);
		
		if (initial) {
			updateBufferUsingStageBuffer(this,handle,cb,initial,create_info.size,0);
		}
		else if (zero_initialize) {
			fillBufferUsingStageBuffer(this, handle, cb, (uint32_t)0, create_info.size, 0);
		}
		return handle;
	}
	//If need commandBuffer but don't provide, use transient
	

	ImageHandle Device::createImage(const ImageCreateInfo& info, std::shared_ptr<ImageAsset> initial , CommandBufferHandle cb)const {
		if (initial) {
			auto staging_buffer = create_image_staging_buffer(info, initial.get());
			return create_image_from_staging_buffer(info, &staging_buffer,cb);
		}
		else {
			return create_image_from_staging_buffer(info,nullptr,cb);
		}
	}
	CommandBufferHandle Device::requestTransientCommandBuffer() const { return allocateCommandBuffer(this, transientPool); };
	ImageHandle Device::create_image_from_staging_buffer(const ImageCreateInfo& create_info, const InitialImageBuffer* staging_buffer, CommandBufferHandle cb)const {
			
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
			assert(!concurrent_queue&&"Don't want to support concurrent_queue");
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
				(!imageFormat_is_supported(physicalDevice.physicalDevice,create_info.format, image_usage_to_features(info.usage) | check_extra_features, info.tiling)))
			{
				LOGI("Format %u is not supported for usage flags!\n", unsigned(create_info.format));
				return ImageHandle(nullptr);
			}
			VmaAllocationCreateInfo allocCreateInfo = {};
			ImageResourceHolder holder(this);
			VmaAllocationInfo allocinfo;
			find_memory_type(create_info.domain, allocCreateInfo);
			vmaCreateImage(allocator,&info,&allocCreateInfo,&holder.image,&holder.allocation,&allocinfo);
			
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
			ImageHandle handle= std::make_shared<tImage>(this,holder.image,holder.image_view,holder.allocation,tmpinfo,view_type);
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
					auto cmd = this->requestTransientCommandBuffer();
					oneTimeSubmit(cmd, this->requestQueue(cmd->getQueueFamilyIdx()), stagingToBuffer);
				}
				else {
					stagingToBuffer(cb);
				}
			

				
			}
			else if(create_info.initial_layout!=VK_IMAGE_LAYOUT_UNDEFINED){
				if (cb == nullptr) {
					auto cmd = this->requestTransientCommandBuffer();
					oneTimeSubmit(cmd, this->requestQueue(cmd->getQueueFamilyIdx()), [&](CommandBufferHandle cb) {setImageLayout(cb, handle,
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
	InitialImageBuffer Device::create_image_staging_buffer(const ImageCreateInfo& info, const ImageAsset* initial)const {
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
		result.buffer = createBuffer(buffer_info, nullptr);
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
	PipelineLayoutHandle Device::createPipelineLayout(std::vector<DescriptorSetLayoutHandle>& descLayouts, GpuBlockBuffer& pushConstant, vk::ShaderStageFlags shaderStage) {
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
		auto vklayout=vk::Device::createPipelineLayout(info);
		return std::make_shared<tPipelineLayout>(this,vklayout);
	}
	InitialImageBuffer Device::create_image_staging_buffer(const TextureFormatLayout& layout)const {
		InitialImageBuffer result;

		BufferCreateInfo buffer_info = {};
		buffer_info.domain = BufferDomain::Host;
		buffer_info.size = layout.get_required_size();
		buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		result.buffer = createBuffer(buffer_info, nullptr);
		//set_name(*result.buffer, "image-upload-staging-buffer");
	
		auto* mapped = getMappedData(result.buffer->getAllocation());
		memcpy(mapped, layout.data(), layout.get_required_size());
		
		//unmap_host_buffer(*result.buffer, MEMORY_ACCESS_WRITE_BIT);

		layout.build_buffer_image_copies(result.blits);
		return result;
	}
	
	SamplerHandle Device::createSampler(const SamplerCreateInfo& sampler_info) {
		auto info = fill_vk_sampler_info(sampler_info);
		VkSampler sampler = vk::Device::createSampler(info);
		SamplerHandle handle = std::make_shared<tSampler>(this,sampler,sampler_info);
		return handle;
	}
	void Device::initStockSamplers() {
		SamplerCreateInfo info = {};
		info.max_lod = VK_LOD_CLAMP_NONE;
		info.max_anisotropy = 1.0f;

		for (unsigned i = 0; i < static_cast<unsigned>(StockSampler::Count); i++)
		{
			auto mode = static_cast<StockSampler>(i);

			switch (mode)
			{
			case StockSampler::NearestShadow:
			case StockSampler::LinearShadow:
				info.compare_enable = true;
				info.compare_op = VK_COMPARE_OP_LESS_OR_EQUAL;
				break;

			default:
				info.compare_enable = false;
				break;
			}

			switch (mode)
			{
			case StockSampler::TrilinearClamp:
			case StockSampler::TrilinearWrap:
				info.mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				break;

			default:
				info.mipmap_mode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
				break;
			}

			switch (mode)
			{
			case StockSampler::LinearClamp:
			case StockSampler::LinearWrap:
			case StockSampler::TrilinearClamp:
			case StockSampler::TrilinearWrap:
			case StockSampler::LinearShadow:
			case StockSampler::LinearYUV420P:
			case StockSampler::LinearYUV422P:
			case StockSampler::LinearYUV444P:
				info.mag_filter = VK_FILTER_LINEAR;
				info.min_filter = VK_FILTER_LINEAR;
				break;

			default:
				info.mag_filter = VK_FILTER_NEAREST;
				info.min_filter = VK_FILTER_NEAREST;
				break;
			}

			switch (mode)
			{
			default:
			case StockSampler::LinearWrap:
			case StockSampler::NearestWrap:
			case StockSampler::TrilinearWrap:
				info.address_mode_u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				info.address_mode_v = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				info.address_mode_w = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				break;

			case StockSampler::LinearClamp:
			case StockSampler::NearestClamp:
			case StockSampler::TrilinearClamp:
			case StockSampler::NearestShadow:
			case StockSampler::LinearShadow:
			case StockSampler::LinearYUV420P:
			case StockSampler::LinearYUV422P:
			case StockSampler::LinearYUV444P:
				info.address_mode_u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				info.address_mode_v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				info.address_mode_w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				break;
			}

			samplers[i] = createSampler(info);
		}
	}
	SamplerHandle Device::getSampler(const StockSampler& id)const {
		return samplers[static_cast<int>(id)];
	}

	
	
	
	
	
	
	

}