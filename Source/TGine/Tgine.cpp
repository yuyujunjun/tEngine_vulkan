#include"Tgine.h"
#include"tDevice.h"
#include"tResource.h"
#include"tFrameBuffer.h"
namespace tEngine {
	//void SetRange(const VmaAllocation allocation, void* data, size_t size, size_t offset) {
	//	//assert(allocation->GetMappedData() != nullptr);
	//	memcpy(static_cast<char*>(getMappedData(allocation)) + offset, data, size);
	//}
	bool imageFormat_is_supported(VkPhysicalDevice physicalDevice,VkFormat format, VkFormatFeatureFlags required, VkImageTiling tiling) 
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
		auto flags = tiling == VK_IMAGE_TILING_OPTIMAL ? props.optimalTilingFeatures : props.linearTilingFeatures;
		return (flags & required) == required;
	}
	VkFormat default_depth_format(VkPhysicalDevice physicalDevice)
	{
		if (imageFormat_is_supported(physicalDevice, VK_FORMAT_D32_SFLOAT, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL))
			return VK_FORMAT_D32_SFLOAT;
		if (imageFormat_is_supported(physicalDevice, VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL))
			return VK_FORMAT_X8_D24_UNORM_PACK32;
		if (imageFormat_is_supported(physicalDevice, VK_FORMAT_D16_UNORM, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL))
			return VK_FORMAT_D16_UNORM;

		return VK_FORMAT_UNDEFINED;
	}
	VkFormat default_depth_stencil_format(VkPhysicalDevice physicalDevice)
	{
		if (imageFormat_is_supported(physicalDevice, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL))
			return VK_FORMAT_D24_UNORM_S8_UINT;
		if (imageFormat_is_supported(physicalDevice, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL))
			return VK_FORMAT_D32_SFLOAT_S8_UINT;

		return VK_FORMAT_UNDEFINED;
	}

	 uint32_t minBufferAlignment(const tPhysicalDevice& physicalDevice) {
		int alignment = std::max(static_cast<uint32_t>(
			physicalDevice.getProperties().limits.minUniformBufferOffsetAlignment),
			static_cast<uint32_t>(
				physicalDevice.getProperties().limits.minStorageBufferOffsetAlignment));
		return alignment;
	}
	 uint32_t alignUniformBufferAlignment(size_t size,const tPhysicalDevice& physicalDevice) {
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
	uint32_t findQueueFamilyIndex(std::vector<vk::QueueFamilyProperties>const& queueFamilyProperties, vk::QueueFlagBits bits) {
		std::vector<vk::QueueFamilyProperties>::const_iterator graphicsQueueFamilyProperty = std::find_if(
			queueFamilyProperties.begin(), queueFamilyProperties.end(), [bits](vk::QueueFamilyProperties const& qfp) {
				return qfp.queueFlags & bits;
			});
		assert(graphicsQueueFamilyProperty != queueFamilyProperties.end());
		return static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
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

	 
}