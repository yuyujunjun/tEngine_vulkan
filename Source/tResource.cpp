#include"tResource.h"
#include"utils.hpp"
namespace tEngine {
    tBuffer::tBuffer(sharedDevice device, vk::DeviceSize size, vk::MemoryPropertyFlags propertyFlags) :device(device), size(size), propertyFlags(propertyFlags) {
        using vk::BufferUsageFlagBits;
        usage = (BufferUsageFlagBits::eTransferSrc) |
            (BufferUsageFlagBits::eTransferDst) |
            (BufferUsageFlagBits::eUniformTexelBuffer) |
            (BufferUsageFlagBits::eStorageTexelBuffer) |
            (BufferUsageFlagBits::eUniformBuffer) |
            (BufferUsageFlagBits::eStorageBuffer) |
            (BufferUsageFlagBits::eIndexBuffer) |
            (BufferUsageFlagBits::eVertexBuffer) |
            (BufferUsageFlagBits::eIndirectBuffer) |
            (BufferUsageFlagBits::eShaderDeviceAddress) |
            (BufferUsageFlagBits::eTransformFeedbackBufferEXT) |
            (BufferUsageFlagBits::eTransformFeedbackCounterBufferEXT) |
            (BufferUsageFlagBits::eConditionalRenderingEXT) |
            (BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR) |
            (BufferUsageFlagBits::eAccelerationStructureStorageKHR) |
            (BufferUsageFlagBits::eShaderBindingTableKHR);
        CreateBufferWithMemory(device);

    }

    DeviceMemory:: DeviceMemory(sharedDevice device, vk::DeviceSize size, vk::MemoryRequirements const& requirements, vk::MemoryPropertyFlags const& propertyFlags) :device(device), size(size) {

        uint32_t memoryTypeIndex =
            vk::su::findMemoryType(device->getMemoryProperties(), requirements.memoryTypeBits, propertyFlags);

        deviceMemory = device->allocateMemory(vk::MemoryAllocateInfo(requirements.size, memoryTypeIndex));
        if (bHost()) {
           memory = device->mapMemory(deviceMemory,0,size);
        }
    }
	tSwapChain::tSwapChain(vk::PhysicalDevice const& physicalDevice,
        sharedDevice device,
		vk::SurfaceKHR const& surface,
		vk::Extent2D const& extent,
		vk::ImageUsageFlags        usage,
		vk::SwapchainKHR const& oldSwapChain,
		uint32_t                   graphicsQueueFamilyIndex,
		uint32_t                   presentQueueFamilyIndex):device(device) {
        using vk::su::clamp;
        vk::SurfaceFormatKHR surfaceFormat = vk::su::pickSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(surface));
        

        vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
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
        vk::PresentModeKHR presentMode = vk::su::pickPresentMode(physicalDevice.getSurfacePresentModesKHR(surface));
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
        swapChain = device->createSwapchainKHR(swapChainCreateInfo);
        auto images = device->getSwapchainImagesKHR(swapChain);

   
        
       
        vk::ComponentMapping componentMapping(
            vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA);
        vk::ImageSubresourceRange subResourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
        imageList.resize(images.size());
        for (int i = 0; i < images.size(); ++i) {
            vk::ImageViewCreateInfo imageViewCreateInfo(
                vk::ImageViewCreateFlags(), images[i], vk::ImageViewType::e2D, surfaceFormat.format, componentMapping, subResourceRange);
            imageList[i] = std::make_shared<tImageView>(device, imageViewCreateInfo);
        }
     
       
	}
}