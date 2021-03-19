#include"tSwapChain.h"
#include"tImage.h"
#include"tDevice.h"
#include"utils.hpp"
#include"tTextureFormatLayout.h"
#include"tDevice.h"
namespace tEngine {
	vk::Format tSwapChain::getFormat() {
		return vk::Format(imageList[0]->get_format());
	}
	
	tSwapChain::tSwapChain(Device* device, vk::SwapchainKHR const& swapChain, vk::Extent2D extent) :device(device), swapChain(swapChain), extent(extent) {

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