#include"tResource.h"
#include"utils.hpp"
#include"CommandBufferBase.h"
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
     void CopyBufferToImage(const CommandBuffer::SharedPtr& cb, BufferHandle buffer, ImageviewHandle imageView, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout) {
        if (initialLayout != vk::ImageLayout::eTransferDstOptimal) {
            setImageLayout(cb, imageView->getImage(), imageView->getFormat(), initialLayout, vk::ImageLayout::eTransferDstOptimal);
        }
        auto extent = imageView->getImage()->getExtent3D();
        vk::BufferImageCopy copyRegion(0,
            extent.width,
            extent.height,
            vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
            vk::Offset3D(0, 0, 0),
            extent);
        
        cb->copyBufferToImage(buffer, imageView->getImage(), vk::ImageLayout::eTransferDstOptimal, copyRegion);
        setImageLayout(cb, imageView->getImage(), imageView->getFormat(), vk::ImageLayout::eTransferDstOptimal, finalLayout);


    }
     ImageviewHandle CreateImageViewWithImage(sharedDevice& device, std::shared_ptr<ImageAsset>& asset, const CommandBuffer::SharedPtr& cb) {
        vk::ImageCreateInfo imageCreateInfo(vk::ImageCreateFlags(),
            asset->imageType,
            asset->format,
            vk::Extent3D(asset->width, asset->height, asset->depth),
            1,
            1,
            vk::SampleCountFlagBits::e1,
            asset->imageTiling,
            asset->usageFlags | vk::ImageUsageFlagBits::eSampled|vk::ImageUsageFlagBits::eTransferDst,
            vk::SharingMode::eExclusive,
            {},
            vk::ImageLayout::eUndefined);
        auto image = tImage::Create(device, imageCreateInfo);
        
        image->AllocateMemory(device, vk::MemoryPropertyFlagBits::eDeviceLocal);
        image->_bakeImageAsset(asset);
        vk::ComponentMapping componentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA);
        vk::ImageSubresourceRange imageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
        vk::ImageViewType viewType = asset->imageType == vk::ImageType::e2D ? vk::ImageViewType::e2D : vk::ImageViewType::e3D;
        auto imageView = tImageView::Create(device, image, vk::ImageViewCreateInfo({}, image->vkImage, viewType, asset->format, componentMapping, imageSubresourceRange));
        auto size = image->getExtent3D().width * image->getExtent3D().height * image->getExtent3D().depth*asset->channels;
        auto buffer = tBuffer::Create(device, size, vk::MemoryPropertyFlagBits::eHostVisible, vk::BufferUsageFlagBits::eTransferSrc);
        buffer->getMemory()->SetRange(asset->pixels, size);
        buffer->getMemory()->Flush();
        CopyBufferToImage(cb, buffer, imageView, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal);
        return imageView;
    }
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

    DeviceMemory:: DeviceMemory(sharedDevice device, vk::DeviceSize size, vk::MemoryRequirements const& requirements, vk::MemoryPropertyFlags const& propertyFlags) :device(device), size(size), memoryProperties(propertyFlags){

        uint32_t memoryTypeIndex =
            vk::su::findMemoryType(device->physicalDevice.getMemoryProperties(), requirements.memoryTypeBits, propertyFlags);

        vkMemory = device->allocateMemory(vk::MemoryAllocateInfo(requirements.size, memoryTypeIndex));
        if (bHost()) {
           mappedMemory = device->mapMemory(vkMemory,0,size);
        }
    }
	tSwapChain::tSwapChain(
        sharedDevice device,
		vk::SurfaceKHR const& surface,
		vk::Extent2D const& extent,
		vk::ImageUsageFlags        usage,
		vk::SwapchainKHR const& oldSwapChain,
		uint32_t                   graphicsQueueFamilyIndex,
		uint32_t                   presentQueueFamilyIndex):device(device) {
        using vk::su::clamp;
        vk::SurfaceFormatKHR surfaceFormat = vk::su::pickSurfaceFormat(device->physicalDevice().getSurfaceFormatsKHR(surface));
        

        vk::SurfaceCapabilitiesKHR surfaceCapabilities = device->physicalDevice().getSurfaceCapabilitiesKHR(surface);
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
        vk::PresentModeKHR presentMode = vk::su::pickPresentMode(device->physicalDevice().getSurfacePresentModesKHR(surface));
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
            auto Image = tImage::Create(device, images[i]);
            imageList[i] = tImageView::Create(device, Image,imageViewCreateInfo);
        }
     
       
	}
}