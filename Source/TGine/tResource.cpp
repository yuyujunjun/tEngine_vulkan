
#include"tResource.h"
#include"utils.hpp"
#include"tTextureFormatLayout.h"
namespace tEngine {
	
    VkImageView tImageView::get_render_target_view(unsigned layer) const
    {
        // Transient images just have one layer.
        if (info.image->get_create_info().domain == ImageDomain::Transient)
            return view;

        VK_ASSERT(layer < get_create_info().layers);

        if (render_target_views.empty())
            return view;
        else
        {
            VK_ASSERT(layer < render_target_views.size());
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
}