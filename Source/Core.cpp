#include"Core.h"
#include "vma/src/vk_mem_alloc.h"
#include"tResource.h"
#include"CommandBufferBase.h"
#include"tAsset.h"
namespace tEngine {
	using namespace std;
	static inline VkImageViewType get_image_view_type(const ImageCreateInfo& create_info, const ImageViewCreateInfo* view)
	{
		unsigned layers = view ? view->layers : create_info.layers;
		unsigned base_layer = view ? view->base_layer : 0;

		if (layers == VK_REMAINING_ARRAY_LAYERS)
			layers = create_info.layers - base_layer;

		bool force_array =
			view ? (view->misc & IMAGE_VIEW_MISC_FORCE_ARRAY_BIT) : (create_info.misc & IMAGE_MISC_FORCE_ARRAY_BIT);

		switch (create_info.type)
		{
		case VK_IMAGE_TYPE_1D:
			VK_ASSERT(create_info.width >= 1);
			VK_ASSERT(create_info.height == 1);
			VK_ASSERT(create_info.depth == 1);
			VK_ASSERT(create_info.samples == VK_SAMPLE_COUNT_1_BIT);

			if (layers > 1 || force_array)
				return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
			else
				return VK_IMAGE_VIEW_TYPE_1D;

		case VK_IMAGE_TYPE_2D:
			VK_ASSERT(create_info.width >= 1);
			VK_ASSERT(create_info.height >= 1);
			VK_ASSERT(create_info.depth == 1);

			if ((create_info.flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) && (layers % 6) == 0)
			{
				VK_ASSERT(create_info.width == create_info.height);

				if (layers > 6 || force_array)
					return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
				else
					return VK_IMAGE_VIEW_TYPE_CUBE;
			}
			else
			{
				if (layers > 1 || force_array)
					return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
				else
					return VK_IMAGE_VIEW_TYPE_2D;
			}

		case VK_IMAGE_TYPE_3D:
			VK_ASSERT(create_info.width >= 1);
			VK_ASSERT(create_info.height >= 1);
			VK_ASSERT(create_info.depth >= 1);
			return VK_IMAGE_VIEW_TYPE_3D;

		default:
			VK_ASSERT(0 && "bogus");
			return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
		}
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
				vmaFreeMemory(*allocator, allocation);
			
		}
	};

	Device::Device(VkDevice device,VkPhysicalDevice phyDevice, VmaAllocator allocator) :vk::Device(device), allocator(allocator) {
		physicalDevice.SetPhysicalDevice(phyDevice);
		vk::PipelineCacheCreateInfo info;
		pipelineCache=createPipelineCache(info);
	}
	void Device::freeAllocation(VmaAllocation allocation)const {
		if (allocation != nullptr) {
			allocator->FreeMemory(1, &allocation);
		}
	}
	bool imageFormat_is_supported(VkPhysicalDevice physicalDevice,VkFormat format, VkFormatFeatureFlags required, VkImageTiling tiling) 
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
		auto flags = tiling == VK_IMAGE_TILING_OPTIMAL ? props.optimalTilingFeatures : props.linearTilingFeatures;
		return (flags & required) == required;
	}
	VkFormat default_depth_stencil_format(VkPhysicalDevice physicalDevice) 
	{
		if (imageFormat_is_supported(physicalDevice,VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL))
			return VK_FORMAT_D24_UNORM_S8_UINT;
		if (imageFormat_is_supported(physicalDevice,VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL))
			return VK_FORMAT_D32_SFLOAT_S8_UINT;

		return VK_FORMAT_UNDEFINED;
	}

	VkFormat default_depth_format(VkPhysicalDevice physicalDevice) 
	{
		if (imageFormat_is_supported(physicalDevice,VK_FORMAT_D32_SFLOAT, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL))
			return VK_FORMAT_D32_SFLOAT;
		if (imageFormat_is_supported(physicalDevice,VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL))
			return VK_FORMAT_X8_D24_UNORM_PACK32;
		if (imageFormat_is_supported(physicalDevice,VK_FORMAT_D16_UNORM, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL))
			return VK_FORMAT_D16_UNORM;

		return VK_FORMAT_UNDEFINED;
	}
	inline  uint32_t minBufferAlignment(const tPhysicalDevice& physicalDevice) {
		int alignment = std::max(static_cast<uint32_t>(
			physicalDevice.getProperties().limits.minUniformBufferOffsetAlignment),
			static_cast<uint32_t>(
				physicalDevice.getProperties().limits.minStorageBufferOffsetAlignment));
		return alignment;
	}
	void fill_buffer_sharing_indices(const tPhysicalDevice& physicalDevice, VkBufferCreateInfo& info, uint32_t* sharing_indices)
	{
		auto& graphics_queue_family_index = physicalDevice.graphicsQueuefamilyId;
		auto& compute_queue_family_index = physicalDevice.computeQueuefamilyId;
		auto& transfer_queue_family_index = physicalDevice.transferQueuefamilyId;
		if (graphics_queue_family_index != compute_queue_family_index ||
			graphics_queue_family_index != transfer_queue_family_index)
		{
			// For buffers, always just use CONCURRENT access modes,
			// so we don't have to deal with acquire/release barriers in async compute.
			info.sharingMode = VK_SHARING_MODE_CONCURRENT;

			sharing_indices[info.queueFamilyIndexCount++] = graphics_queue_family_index;

			if (graphics_queue_family_index != compute_queue_family_index)
				sharing_indices[info.queueFamilyIndexCount++] = compute_queue_family_index;

			if (graphics_queue_family_index != transfer_queue_family_index &&
				compute_queue_family_index != transfer_queue_family_index)
			{
				sharing_indices[info.queueFamilyIndexCount++] = transfer_queue_family_index;
			}

			info.pQueueFamilyIndices = sharing_indices;
		}
	}
	void find_memory_type(BufferDomain domain, VmaAllocationCreateInfo& info) {
		uint32_t prio[3] = {};
		switch (domain)
		{
		case BufferDomain::Device:
			prio[0] = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			break;

		case BufferDomain::LinkedDeviceHost:
			prio[0] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			prio[1] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			prio[2] = prio[1];
			break;

		case BufferDomain::LinkedDeviceHostPreferDevice:
			prio[0] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			prio[1] = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			prio[2] = prio[1];
			break;

		case BufferDomain::Host:
			prio[0] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			prio[1] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			prio[2] = prio[1];
			break;

		case BufferDomain::CachedHost:
			prio[0] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
			prio[1] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			prio[2] = prio[1];
			break;

		case BufferDomain::CachedCoherentHostPreferCached:
			prio[0] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			prio[1] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
			prio[2] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			break;

		case BufferDomain::CachedCoherentHostPreferCoherent:
			prio[0] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			prio[1] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			prio[2] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			break;
		}
		info.preferredFlags = prio[0];
		info.requiredFlags = prio[1];
		
	}
	void find_memory_type(ImageDomain domain, VmaAllocationCreateInfo& info)
	{
		uint32_t desired = 0, fallback = 0;
		switch (domain)
		{
		case ImageDomain::Physical:
			desired = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			fallback = 0;
			break;

		case ImageDomain::Transient:
			desired = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
			fallback = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			break;

		case ImageDomain::LinearHostCached:
			desired = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
			fallback = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			break;

		case ImageDomain::LinearHost:
			desired = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			fallback = 0;
			break;
		}

		info.requiredFlags = fallback;
		info.preferredFlags = desired;

		
	}
	bool memory_type_is_host_visible(const tPhysicalDevice& physicalDevice, uint32_t type)
	{
		return (static_cast<uint32_t>(physicalDevice.getMemoryProperties().memoryTypes[type].propertyFlags) & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
	}

	BufferHandle Device::createBuffer(const BufferCreateInfo& create_info, const void* initial, CommandBufferHandle cb = nullptr)const {
		vk::Buffer buffer;
		VkMemoryRequirements reqs;
		VmaAllocation allocation;

		bool zero_initialize = (create_info.misc & BUFFER_MISC_ZERO_INITIALIZE_BIT) != 0;
		if (initial && zero_initialize)
		{
			LOGI("Cannot initialize buffer with data and clear.\n");
			return BufferHandle{};
		}

		VkBufferCreateInfo info;// = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		info.size = create_info.size;
		info.usage = create_info.usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		uint32_t sharing_indices[3];
		if (physicalDevice.bUniqueQueueFamily()) {
			fill_buffer_sharing_indices(physicalDevice,info, sharing_indices);
		}
	

		VmaAllocationCreateInfo allocCreateInfo;
		VkBuffer vkBuffer;
		VmaAllocation allocation;
		VmaAllocationInfo allocinfo;
		find_memory_type(create_info.domain, allocCreateInfo);
		vmaCreateBuffer(allocator, &info, &allocCreateInfo, &vkBuffer, &allocation, &allocinfo);
		BufferHandle handle = std::make_shared<tBuffer>(this,vkBuffer,allocation,create_info);

		if (create_info.domain == BufferDomain::Device && (initial || zero_initialize) && !memory_type_is_host_visible(physicalDevice,allocinfo.memoryType))
		{
			assert(cb != nullptr&&"Device memory must have commandBufer");
			CommandBufferHandle cmd=cb;
			if (initial)
			{
				auto staging_info = create_info;
				staging_info.domain = BufferDomain::Host;
				auto staging_buffer = createBuffer(staging_info, initial);
				vk::BufferCopy region;
				region.setSize(create_info.size);
				region.setSrcOffset(0);
				region.setDstOffset(0);
				cmd->copyBuffer(staging_buffer,handle,region);
			}
			else
			{
				cmd->fillBuffer(handle, 0,0, create_info.size);
			}

		//	LOCK();
		//	submit_staging(cmd, info.usage, true);
		}
		else if (initial || zero_initialize)
		{
			void* ptr = allocinfo.pMappedData;
			if (!ptr)
				return BufferHandle(nullptr);

			if (initial)
				memcpy(ptr, initial, create_info.size);
			else
				memset(ptr, 0, create_info.size);
			
		}
		return handle;
	}
	ImageHandle Device::createImage(const ImageCreateInfo& info, std::shared_ptr<ImageAsset> initial , CommandBufferHandle cb)const {
		if (initial) {
			auto staging_buffer = create_image_staging_buffer(info, initial.get());
			return create_image_from_staging_buffer(info, &staging_buffer);
		}
		else {
			return create_image_from_staging_buffer(info);
		}
	}
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
			VmaAllocationCreateInfo allocCreateInfo;
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
				VK_ASSERT(create_info.domain != ImageDomain::Transient);
				VK_ASSERT(create_info.initial_layout != VK_IMAGE_LAYOUT_UNDEFINED);
				bool generate_mips = (create_info.misc & IMAGE_MISC_GENERATE_MIPS_BIT) != 0;
				setImageLayout(cb, handle, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
				cb->copyBufferToImage(staging_buffer->buffer, handle, vk::ImageLayout::eTransferDstOptimal, staging_buffer->blits);
				if (generate_mips) {
					barrier_prepare_generate_mipmap(cb, handle, vk::ImageLayout::eTransferDstOptimal,
						vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferWrite, true);
					generateMipmap(cb,handle);
				}
				setImageLayout(cb,handle,
					generate_mips?vk::ImageLayout::eTransferSrcOptimal:vk::ImageLayout::eTransferDstOptimal,
					(vk::ImageLayout)create_info.initial_layout);

				
			}
			else if(create_info.initial_layout!=VK_IMAGE_LAYOUT_UNDEFINED){
				setImageLayout(cb, handle,
					vk::ImageLayout::eUndefined,
					(vk::ImageLayout)create_info.initial_layout);
			}
			return handle;
			
	}
	InitialImageBuffer Device::create_image_staging_buffer(const ImageCreateInfo& info, const ImageAsset* initial)const {
		InitialImageBuffer result;
		assert(info.layers == 0 && "Can't support image array for reading image from file");
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
		auto* mapped = static_cast<uint8_t*>(result.buffer->getAllocation()->GetMappedData());
		unsigned index = 0;

		layout.set_buffer(mapped, layout.get_required_size());

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
		vk::PipelineLayoutCreateInfo  info;

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

		auto* mapped = static_cast<uint8_t*>(result.buffer->getAllocation()->GetMappedData());
		memcpy(mapped, layout.data(), layout.get_required_size());
		
		//unmap_host_buffer(*result.buffer, MEMORY_ACCESS_WRITE_BIT);

		layout.build_buffer_image_copies(result.blits);
		return result;
	}
	static VkSamplerCreateInfo fill_vk_sampler_info(const SamplerCreateInfo& sampler_info)
	{
		VkSamplerCreateInfo info = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };

		info.magFilter = sampler_info.mag_filter;
		info.minFilter = sampler_info.min_filter;
		info.mipmapMode = sampler_info.mipmap_mode;
		info.addressModeU = sampler_info.address_mode_u;
		info.addressModeV = sampler_info.address_mode_v;
		info.addressModeW = sampler_info.address_mode_w;
		info.mipLodBias = sampler_info.mip_lod_bias;
		info.anisotropyEnable = sampler_info.anisotropy_enable;
		info.maxAnisotropy = sampler_info.max_anisotropy;
		info.compareEnable = sampler_info.compare_enable;
		info.compareOp = sampler_info.compare_op;
		info.minLod = sampler_info.min_lod;
		info.maxLod = sampler_info.max_lod;
		info.borderColor = sampler_info.border_color;
		info.unnormalizedCoordinates = sampler_info.unnormalized_coordinates;
		return info;
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
		
			auto backbuffer = std::make_shared<tImage>(device, image, image_view, nullptr, info,VkImageViewType::VK_IMAGE_VIEW_TYPE_2D);
			backbuffer->set_swapchain_layout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
			images.push_back(backbuffer);
		}
		SwapChainHandle handle= std::make_shared<tSwapChain>(device,swapChain);
		handle->setImages(images);
		return handle;
	}
	
	//without shader
	GraphicsPipelineCreateInfo getDefaultPipelineCreateInfo(tShaderInterface* shader,const tRenderPass* renderPass,uint32_t subpass,const tFrameBuffer* frameBuffer) {
		// Shader
		GraphicsPipelineCreateInfo createInfo;
		for (int i = 0; i < shader->getShader()->shaderModule.size(); ++i) {
			const auto& sha = shader->getShader();
			createInfo.addShader(sha->shaderModule[i], sha->shaderStage[i]);
		}
		//Blend
		createInfo.coloBlendState.logicOpEnable = false;
		vk::PipelineColorBlendAttachmentState blendState;
		blendState.blendEnable = false;
		for (int i = 0; i < renderPass->getSubpass(subpass)->getColorAttachmentCount(); ++i) {
			createInfo.coloBlendState.Attachments.emplace_back(blendState);
		}
		//DepthStencil
		createInfo.depthStencilState.depthTestEnable = true;
		createInfo.depthStencilState.depthWriteEnable = true;

		//dynamicState
		createInfo.dynamicState.dynamicStates.emplace_back(vk::DynamicState::eViewport);
		
		//Layout
		createInfo.layout = shader->getShader()->pipelinelayout->getVkHandle();

		//multiSampleState
		createInfo.multisampleState.sampleShadingEnable = false;
		//rasterizationState
		createInfo.rasterizationState.cullMode = vk::CullModeFlagBits::eNone;
		createInfo.rasterizationState.polygonMode = vk::PolygonMode::eFill;

		createInfo.renderPass = renderPass->getVkHandle();
		createInfo.subpass = subpass;

		//tessellstion
		//createInfo.tessellation
		//Topology
		createInfo.topology.primitiveRestartEnable = false;
		createInfo.topology.topolygy = vk::PrimitiveTopology::eTriangleList;


		//VertexAttribute
		createInfo.vertexAttribute = {
			{ 0, 0, vk::Format::eR32G32B32Sfloat, 0 } ,
			{ 1, 0, vk::Format::eR32G32B32Sfloat, 12 } ,
			{ 2,0,vk::Format::eR32G32Sfloat,24 } ,
			{ 3,0,vk::Format::eR32G32B32A32Sfloat,32 } };
		
		createInfo.vertexInput = { {0,sizeof(Vertex),vk::VertexInputRate::eVertex} };


		createInfo.viewport.viewPorts = { frameBuffer->getViewPort() };
		createInfo.viewport.scissors = { frameBuffer->getRenderArea() };
		return createInfo;
	
	}

}