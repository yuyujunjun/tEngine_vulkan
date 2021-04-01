
#include"Buffer.h"
#include"Image.h"
#include"Sampler.h"
#include"Device.h"
#include"utils.hpp"
#include"TextureFormatLayout.h"
#include"GpuBlock.h"
#include"CommandBufferBase.h"
#include"Log.h"
#include"Asset.h"
namespace tEngine {


    VkImageView tImageView::get_render_target_view(unsigned layer) const
    {
        // Transient images just have one layer.
        if (info.image->get_create_info().domain == ImageDomain::Transient)
            return view;

        assert(layer < get_create_info().layers);

        if (render_target_views.empty())
            return view;
        else
        {
            assert(layer < render_target_views.size());
            return render_target_views[layer];
        }
    }
 
  
    const vk::DeviceSize tBuffer::getSize()const {
        return bufferInfo.size;
      //  return getAllocationSize(alloc);
    }
    void tBuffer::setRange(const void* data, size_t offset, size_t size) {
        
        if (getMappedData(alloc)) {
            memcpy(static_cast<uint8_t*>(getMappedData(alloc)) + offset, data, size);
        }
        else {
            assert(false);
        }
    }
    void tBuffer::Flush() {
        FlushMemory(device, alloc, 0, bufferInfo.size);
    }
	size_t BufferRangeManager::SetRangeIncremental(void* data) {
	//	std::lock_guard<std::mutex> m(mtx);
		NextRangenoLock();
		SetRangeNoLock(data);
		return offset;
	}
	//return current offset
	void BufferRangeManager::SetRangeNoLock(void* data) {
		handle->setRange(data, offset, rangeSize);

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
	VkSamplerCreateInfo fill_vk_sampler_info(const SamplerCreateInfo& sampler_info)
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

	 ImageCreateInfo ImageCreateInfo::immutable_image(const TextureFormatLayout& layout)
	{

		ImageCreateInfo info;
		info.width = layout.get_width();
		info.height = layout.get_height();
		info.type = layout.get_image_type();
		info.depth = layout.get_depth();
		info.format = layout.get_format();
		info.layers = layout.get_layers();
		info.levels = layout.get_levels();
		info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
		info.initial_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.domain = ImageDomain::Physical;
		return info;
	}

	 ImageCreateInfo ImageCreateInfo::immutable_2d_image(unsigned width, unsigned height, VkFormat format, bool mipmapped )
	{
		ImageCreateInfo info;
		info.width = width;
		info.height = height;
		info.depth = 1;
		info.levels = mipmapped ? 0u : 1u;
		info.format = format;
		info.type = VK_IMAGE_TYPE_2D;
		info.layers = 1;
		info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.flags = 0;
		info.misc = mipmapped ? unsigned(IMAGE_MISC_GENERATE_MIPS_BIT) : 0u;
		info.initial_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		return info;
	}

	 ImageCreateInfo
		ImageCreateInfo::immutable_3d_image(unsigned width, unsigned height, unsigned depth, VkFormat format, bool mipmapped )
	{
		ImageCreateInfo info = immutable_2d_image(width, height, format, mipmapped);
		info.depth = depth;
		info.type = VK_IMAGE_TYPE_3D;
		return info;
	}

	 ImageCreateInfo ImageCreateInfo::render_target(unsigned width, unsigned height, VkFormat format)
	{
		ImageCreateInfo info;
		info.width = width;
		info.height = height;
		info.depth = 1;
		info.levels = 1;
		info.format = format;
		info.type = VK_IMAGE_TYPE_2D;
		info.layers = 1;
		info.usage = (format_has_depth_or_stencil_aspect(format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT :
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) |
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.flags = 0;
		info.misc = 0;
		info.initial_layout = format_has_depth_or_stencil_aspect(format) ?
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL :
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		return info;
	}

	 ImageCreateInfo ImageCreateInfo::transient_render_target(unsigned width, unsigned height, VkFormat format)
	{
		ImageCreateInfo info;
		info.domain = ImageDomain::Transient;
		info.width = width;
		info.height = height;
		info.depth = 1;
		info.levels = 1;
		info.format = format;
		info.type = VK_IMAGE_TYPE_2D;
		info.layers = 1;
		info.usage = (format_has_depth_or_stencil_aspect(format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT :
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) |
			VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.flags = 0;
		info.misc = 0;
		info.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		return info;
	}

	 uint32_t ImageCreateInfo::compute_view_formats(const ImageCreateInfo& info, VkFormat* formats)
	{
		if ((info.misc & IMAGE_MISC_MUTABLE_SRGB_BIT) == 0)
			return 0;

		switch (info.format)
		{
		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_R8G8B8A8_SRGB:
			formats[0] = VK_FORMAT_R8G8B8A8_UNORM;
			formats[1] = VK_FORMAT_R8G8B8A8_SRGB;
			return 2;

		case VK_FORMAT_B8G8R8A8_UNORM:
		case VK_FORMAT_B8G8R8A8_SRGB:
			formats[0] = VK_FORMAT_B8G8R8A8_UNORM;
			formats[1] = VK_FORMAT_B8G8R8A8_SRGB;
			return 2;

		case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
		case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
			formats[0] = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
			formats[1] = VK_FORMAT_A8B8G8R8_SRGB_PACK32;
			return 2;

		default:
			return 0;
		}
	}

	 VkImageViewType get_image_view_type(const ImageCreateInfo& create_info, const ImageViewCreateInfo* view)
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
			 assert(create_info.width >= 1);
			 assert(create_info.height == 1);
			 assert(create_info.depth == 1);
			 assert(create_info.samples == VK_SAMPLE_COUNT_1_BIT);

			 if (layers > 1 || force_array)
				 return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
			 else
				 return VK_IMAGE_VIEW_TYPE_1D;

		 case VK_IMAGE_TYPE_2D:
			 assert(create_info.width >= 1);
			 assert(create_info.height >= 1);
			 assert(create_info.depth == 1);

			 if ((create_info.flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) && (layers % 6) == 0)
			 {
				 assert(create_info.width == create_info.height);

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
			 assert(create_info.width >= 1);
			 assert(create_info.height >= 1);
			 assert(create_info.depth >= 1);
			 return VK_IMAGE_VIEW_TYPE_3D;

		 default:
			 assert(0 && "bogus");
			 return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
		 }
	 }


	 SamplerHandle createSampler(const Device* device, const SamplerCreateInfo& sampler_info) {
		 auto info = fill_vk_sampler_info(sampler_info);
		 VkSampler sampler = device->createSampler(info);
		 SamplerHandle handle = std::make_shared<tSampler>(device, sampler, sampler_info);
		 return handle;
	 }
	 bool imageFormat_is_supported(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatFeatureFlags required, VkImageTiling tiling)
	 {
		 VkFormatProperties props;
		 vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
		 auto flags = tiling == VK_IMAGE_TILING_OPTIMAL ? props.optimalTilingFeatures : props.linearTilingFeatures;
		 return (flags & required) == required;
	 }
	 uint32_t minBufferAlignment(const tPhysicalDevice& physicalDevice) {
		 int alignment = std::max(static_cast<uint32_t>(
			 physicalDevice.getProperties().limits.minUniformBufferOffsetAlignment),
			 static_cast<uint32_t>(
				 physicalDevice.getProperties().limits.minStorageBufferOffsetAlignment));
		 return alignment;
	 }
	 uint32_t alignUniformBufferAlignment(size_t size, const tPhysicalDevice& physicalDevice) {
		 return  align(size,
			 physicalDevice.getProperties().limits.minUniformBufferOffsetAlignment);
	 }
	 uint32_t alignStorageBufferAlignment(size_t size, const tPhysicalDevice& physicalDevice) {
		 return  align(size,
			 physicalDevice.getProperties().limits.minStorageBufferOffsetAlignment);
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
	 bool memory_type_is_host_visible(const tPhysicalDevice& physicalDevice, uint32_t type)
	 {
		 return (static_cast<uint32_t>(physicalDevice.getMemoryProperties().memoryTypes[type].propertyFlags) & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
	 }
	 vk::AccessFlags getAccesFlagsFromLayout(vk::ImageLayout layout)
	 {
		 switch (layout)
		 {
		 case vk::ImageLayout::eGeneral:
			 return vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eColorAttachmentRead |
				 vk::AccessFlagBits::eColorAttachmentWrite;
		 case vk::ImageLayout::eColorAttachmentOptimal: return vk::AccessFlagBits::eColorAttachmentRead |
			 vk::AccessFlagBits::eColorAttachmentWrite;
		 case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			 return vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		 case vk::ImageLayout::eTransferDstOptimal: return vk::AccessFlagBits::eTransferWrite;
		 case vk::ImageLayout::eTransferSrcOptimal: return vk::AccessFlagBits::eTransferRead;
		 case vk::ImageLayout::eShaderReadOnlyOptimal: return vk::AccessFlagBits::eShaderRead;
		 case vk::ImageLayout::ePresentSrcKHR: return vk::AccessFlagBits::eMemoryRead;
		 case vk::ImageLayout::ePreinitialized: return vk::AccessFlagBits::eHostWrite;
		 default: return (vk::AccessFlags)0;
		 }
	 }
	 static inline VkAccessFlags buffer_usage_to_possible_access(VkBufferUsageFlags usage)
	 {
		 VkAccessFlags flags = 0;
		 if (usage & (VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT))
			 flags |= VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		 if (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
			 flags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
		 if (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
			 flags |= VK_ACCESS_INDEX_READ_BIT;
		 if (usage & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT)
			 flags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
		 if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
			 flags |= VK_ACCESS_UNIFORM_READ_BIT;
		 if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
			 flags |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

		 return flags;
	 }
	 static inline VkPipelineStageFlags buffer_usage_to_possible_stages(VkBufferUsageFlags usage)
	 {
		 VkPipelineStageFlags flags = 0;
		 if (usage & (VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT))
			 flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
		 if (usage & (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT))
			 flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
		 if (usage & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT)
			 flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
		 if (usage & (VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT |
			 VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT))
			 flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
			 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		 if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
			 flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

		 return flags;
	 }
	 vk::BufferUsageFlags descriptorTypeToBufferUsage(vk::DescriptorType type) {
		 switch (type) {
		 case vk::DescriptorType::eUniformBuffer:
		 case vk::DescriptorType::eUniformBufferDynamic:
			 return vk::BufferUsageFlagBits::eUniformBuffer; break;
		 case vk::DescriptorType::eUniformTexelBuffer:
			 return vk::BufferUsageFlagBits::eUniformTexelBuffer; break;
		 case vk::DescriptorType::eStorageBuffer:
		 case vk::DescriptorType::eStorageBufferDynamic:
			 return vk::BufferUsageFlagBits::eStorageBuffer; break;
		 case vk::DescriptorType::eStorageTexelBuffer:
			 return vk::BufferUsageFlagBits::eStorageTexelBuffer; break;
		 default:
			 assert(false);
		 }
		 return vk::BufferUsageFlagBits(0);
	 }
	 std::shared_ptr<BufferRangeManager> createBufferFromBlock(Device* device, const GpuBlockBuffer& block, uint32_t rangeCount) {
		 //	auto s_b = blockToSetBinding.at(name);
			 //auto block = setInfos[s_b.first].blockBuffers.at(s_b.second);
			 //auto type = setInfos[s_b.first].data.bindingAt(s_b.second).descriptorType;
		 BufferCreateInfo createInfo;
		 createInfo.domain = BufferDomain::Host;

		 createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		 size_t rangeSize = 0;
		 rangeSize = alignUniformBufferAlignment(block.ByteSize(), device->getPhysicalDevice());

		 createInfo.size = rangeSize * rangeCount;
		 auto buffer = createBuffer(device, createInfo);
		 return std::make_shared<BufferRangeManager>(buffer, rangeSize, 0);
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
	 std::shared_ptr<tImage> tImage::requestDummyImage(const Device* device) {
		 static std::shared_ptr<tImage> dummyImage;
		 if (dummyImage == nullptr) {
			 ImageCreateInfo info = ImageCreateInfo::immutable_2d_image(1,1,(VkFormat)vk::Format::eB8G8R8A8Unorm,false);
			 std::vector<unsigned char> color(4);
			 color = { 255,0,255,255 };
			 auto asset=std::make_shared<ImageAsset>();
			 asset->pixels = static_cast<stbi_uc*>(color.data());
			 
			 dummyImage = createImage(device, info, asset);
			 asset->pixels = nullptr;

		 }
		 return dummyImage;
	 }
	  std::shared_ptr<tImage> tImage::requestWhiteImage(const Device* device) {
		 static std::shared_ptr<tImage> whiteImage;
		 if (whiteImage == nullptr) {
			 ImageCreateInfo info = ImageCreateInfo::immutable_2d_image(1, 1, (VkFormat)vk::Format::eB8G8R8A8Unorm, false);
			 std::vector<unsigned char> color(4);
			 color = { 255,255,255,255 };
			 auto asset = std::make_shared<ImageAsset>();
			 asset->pixels = static_cast<stbi_uc*>(color.data());

			 whiteImage = createImage(device, info, asset);
			 asset->pixels = nullptr;

		 }
		 return whiteImage;
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
			 auto staging_buffer = createBuffer(device, staging_info, data);
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

			 //assert(cb != nullptr && "Need commandBuffer to uploading data");
			 assert(create_info.domain != ImageDomain::Transient);
			 assert(create_info.initial_layout != VK_IMAGE_LAYOUT_UNDEFINED);
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
		 result.buffer = createBuffer(device, buffer_info, nullptr);
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

	 InitialImageBuffer create_image_staging_buffer(const Device* device, const TextureFormatLayout& layout) {
		 InitialImageBuffer result;

		 BufferCreateInfo buffer_info = {};
		 buffer_info.domain = BufferDomain::Host;
		 buffer_info.size = layout.get_required_size();
		 buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		 result.buffer = createBuffer(device, buffer_info, nullptr);
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
			 auto staging_buffer = create_image_staging_buffer(device, info, initial.get());
			 return create_image_from_staging_buffer(device, info, &staging_buffer, cb);
		 }
		 else {
			 return create_image_from_staging_buffer(device, info, nullptr, cb);
		 }
	 }
}