#pragma once
#include"Core.h"
#include"tFormatInterfacce.h"
namespace tEngine {
	static inline vk::ImageAspectFlags format_to_aspect_mask(vk::Format format)
	{
		switch (static_cast<VkFormat>(format))
		{
		case VK_FORMAT_UNDEFINED:
			return static_cast<vk::ImageAspectFlags>(0);

		case VK_FORMAT_S8_UINT:
			return static_cast<vk::ImageAspectFlags>(VK_IMAGE_ASPECT_STENCIL_BIT);

		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			return static_cast<vk::ImageAspectFlags>(VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT);

		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_D32_SFLOAT:
		case VK_FORMAT_X8_D24_UNORM_PACK32:
			return static_cast<vk::ImageAspectFlags>(VK_IMAGE_ASPECT_DEPTH_BIT);

		default:
			return static_cast<vk::ImageAspectFlags>(VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}

	
}