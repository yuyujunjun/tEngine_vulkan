#pragma once
#include"vulkan/vulkan.h"
#include"vulkan/vulkan.hpp"
#include"Core.h"
#include"tAsset.h"
namespace tEngine {
	class CommandBuffer;

	class DeviceMemory {
	public:
		DeviceMemory():size(0),vkMemory(vk::DeviceMemory()),mappedMemory((void*)0){}
		DeviceMemory(sharedDevice device, vk::DeviceSize size, vk::MemoryRequirements const& requirements, vk::MemoryPropertyFlags const& propertyFlags);
		~DeviceMemory() {
			if (vkMemory) {
				if (!device.expired()) {
					device.lock()->freeMemory(vkMemory);
					vkMemory = vk::DeviceMemory();
				}
				else {
					reportDestroyedAfterDevice();
				}
			}
		}
		
		inline bool bHost() {
			return (static_cast<uint32_t>(memoryProperties & vk::MemoryPropertyFlagBits::eHostCached) != 0) ||
				(static_cast<uint32_t>(memoryProperties & vk::MemoryPropertyFlagBits::eHostCoherent) != 0) ||
				(static_cast<uint32_t>(memoryProperties & vk::MemoryPropertyFlagBits::eHostVisible) != 0);
		}
		inline bool bHostVisible() {
			return (static_cast<uint32_t>(memoryProperties & vk::MemoryPropertyFlagBits::eHostVisible) != 0);
		}
		inline bool bHostCoherent() {
			return (static_cast<uint32_t>(memoryProperties & vk::MemoryPropertyFlagBits::eHostCached) != 0) ||
				(static_cast<uint32_t>(memoryProperties & vk::MemoryPropertyFlagBits::eHostCoherent) != 0);
		}
		void* Data() {
			return mappedMemory;
		}
		vk::DeviceSize getSize() {
			return size;
		}
		void SetRange(const void* data, size_t size, int offset = 0) {
			memcpy((char*)mappedMemory + offset, (const char*)data, size);
			
		}
		void Flush(vk::DeviceSize offset = 0, vk::DeviceSize size = VK_WHOLE_SIZE) {
			if (bHostVisible()) {
				flushRange(offset, size);
			}
		}
		vk::DeviceMemory vkMemory;
	private:
		void flushRange(vk::DeviceSize offset = 0, vk::DeviceSize size = VK_WHOLE_SIZE) {
			VkMappedMemoryRange range = {};
			range.sType = static_cast<VkStructureType>(vk::StructureType::eMappedMemoryRange);
			range.memory = vkMemory;
			range.offset = offset;
			range.size = size;
			device.lock()->flushMappedMemoryRanges({ range });
		}
		void* mappedMemory;
		vk::MemoryPropertyFlags memoryProperties;
		vk::DeviceSize size;
		std::weak_ptr<vk::Device> device;
		
	};
	class tBuffer {
	public:
		using SharedPtr = std::shared_ptr<tBuffer>;
		static SharedPtr Create(sharedDevice& device, vk::DeviceSize size, vk::MemoryPropertyFlags propertyFlags, vk::BufferUsageFlags usage) {
			return std::make_shared<tBuffer>(device,size,propertyFlags,usage);
		}
		tBuffer(sharedDevice device,vk::DeviceSize size, vk::MemoryPropertyFlags propertyFlags,vk::BufferUsageFlags usage):device(device),size(size), propertyFlags(propertyFlags),usage(usage) {
			
			CreateBufferWithMemory(device);
		}
		tBuffer(sharedDevice device, vk::DeviceSize size, vk::MemoryPropertyFlags propertyFlags);
		~tBuffer() {
			deviceMemory.reset();
			if (vkbuffer) {
				if (!device.expired()) {
					device.lock()->destroyBuffer(vkbuffer);
					vkbuffer = vk::Buffer();
				}
				else {
					reportDestroyedAfterDevice();
				}
			}
		}
		DeviceMemory* getMemory() {
			return deviceMemory.get();
		}
		vk::Buffer& getBuffer() {
			return vkbuffer;
		}
		vk::Buffer vkbuffer;
		std::unique_ptr<DeviceMemory> deviceMemory;
	private:
		inline static uint32_t minBufferAlignment(const Device* const device) {
			int alignment = std::max(static_cast<uint32_t>(
				device->physicalDevice.getDeviceProperties().limits.minUniformBufferOffsetAlignment),
				static_cast<uint32_t>(
					device->physicalDevice.getDeviceProperties().limits.minStorageBufferOffsetAlignment));
			return alignment;
		}
		void CreateBufferWithMemory(sharedDevice& device) {
			size = align(size, minBufferAlignment(device.get()));
			vkbuffer = device->createBuffer(vk::BufferCreateInfo(vk::BufferCreateFlags(), size, usage));
			auto memoryRequirements = device->getBufferMemoryRequirements(vkbuffer);
			size = memoryRequirements.size;
			deviceMemory = std::make_unique<DeviceMemory>(device, size, memoryRequirements, propertyFlags);
			
			device->bindBufferMemory(vkbuffer, deviceMemory->vkMemory, 0);
		}
		weakDevice device;
		
		
		vk::DeviceSize size;
		vk::BufferUsageFlags usage;
		vk::MemoryPropertyFlags propertyFlags;
	};
	class tSampler {
	public:
		DECLARE_SHARED(tSampler);
		static SharedPtr Create(sharedDevice& device) {
			return std::make_shared<tSampler>(device);
		}
		tSampler(sharedDevice& device):device(device) {
			vk::SamplerCreateInfo info;
			vksampler = device->createSampler(info);
		
		}
		~tSampler() {
			if (vksampler) {
				if (!device.expired()) {
					device.lock()->destroySampler(vksampler); 
					vksampler = vk::Sampler();
				}
				else {
					reportDestroyedAfterDevice();
				}
			}
		}
		vk::Sampler vksampler;
	private:
		std::weak_ptr<vk::Device> device;
		
	};
	struct tImage {
	public:
		
		friend class tImageView;
		using SharedPtr = std::shared_ptr<tImage>;
		static SharedPtr Create(sharedDevice& device, vk::Image image) {
			return std::make_shared<tImage>(device, image);
		}
		static SharedPtr Create(sharedDevice& device, vk::ImageCreateInfo info) {
			return std::make_shared<tImage>(device, info);
		}
		tImage(sharedDevice& device, vk::Image image) :vkImage(image), device(device), imageLayout(vk::ImageLayout()){
		
		}
		tImage(sharedDevice& device, vk::ImageCreateInfo info):device(device), imageLayout(vk::ImageLayout()) {
			vkImage = device->createImage(info);
			extent3D = info.extent;
		}
		void setImageLayout(vk::ImageLayout layout) {
			this->imageLayout = layout;
		}
		void AllocateMemory(sharedDevice& device, vk::MemoryPropertyFlags const& propertyFlags) {
			auto memoryRequirements = device->getImageMemoryRequirements(vkImage);
			deviceMemory = std::make_unique<DeviceMemory>(device, memoryRequirements.size, memoryRequirements, propertyFlags);
			device->bindImageMemory(vkImage, deviceMemory->vkMemory, 0);
		}
		~tImage() {
			deviceMemory.reset();
			if (vkImage) {
				if (!device.expired()) {

					device.lock()->destroyImage(vkImage);
					vkImage = vk::Image();
				}
				else {
					reportDestroyedAfterDevice();

				}
			}

		}
		vk::Extent3D getExtent3D() {
			return extent3D;
		}
		vk::Image& VKHandle() {
			return vkImage;
		}
		DeviceMemory* getMemory() {
			return deviceMemory.get();
		}
		vk::Image vkImage;
		void _bakeImageAsset(std::shared_ptr<ImageAsset>& asset) {
			this->asset = asset;
		}
	private:
		vk::ImageLayout imageLayout;
		vk::Extent3D extent3D;
		std::weak_ptr<vk::Device> device;
		std::unique_ptr<DeviceMemory> deviceMemory;
		std::shared_ptr<ImageAsset> asset;
		
	};
	class tImageView {
	public:
		using SharedPtr = std::shared_ptr<tImageView>;
		static SharedPtr Create(sharedDevice& device, std::shared_ptr<tImage>& image, const vk::ImageViewCreateInfo& info){
			return std::make_shared<tImageView>(device,image,info);
		}
		/*tImageView(sharedDevice device, std::shared_ptr<tImage>& image) :device(device),image(image) {

		}*/
		tImageView(sharedDevice& device, std::shared_ptr<tImage>& image, vk::ImageViewCreateInfo info) :device(device), image(image) {
			info.image = image->vkImage;
			vkimageView = device->createImageView(info);
			sampler = tSampler::Create(device);
		}
		~tImageView(){
			if (vkimageView) {
				if (!device.expired()) {
					device.lock()->destroyImageView(vkimageView);
					vkimageView = vk::ImageView();
				}
				else {
					reportDestroyedAfterDevice();
				}
			}
		}
		vk::Format getFormat() {
			return createInfo.format;
		}
		
		void SetImageDevice(vk::Image& image, sharedDevice device) {
			this->device = device;
			this->image->vkImage = image;
			this->image->device = device;
			
		}
		std::shared_ptr<tImage>& getImage() {
			return image;
		}
		tSampler::SharedPtr sampler;
		vk::ImageView vkimageView;
		std::shared_ptr<tImage> image;
	private:
		
	
		vk::ImageViewCreateInfo createInfo;
		
		std::weak_ptr<vk::Device> device;
	};
	
	tImageView::SharedPtr CreateImageViewWithImage(sharedDevice& device, vk::ImageCreateInfo imageInfo, vk::ImageViewCreateInfo viewInfo, vk::MemoryPropertyFlags& memoryProperties);
	tImageView::SharedPtr CreateImageViewWithImage(sharedDevice& device, std::shared_ptr<ImageAsset>& asset,const std::shared_ptr<CommandBuffer>& cb);
	void CopyBufferToImage(const std::shared_ptr<CommandBuffer>& cb, tBuffer::SharedPtr buffer, tImageView::SharedPtr imageView, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout);

	class tSwapChain {
	public:
		using SharedPtr = std::shared_ptr<tSwapChain>;
		tSwapChain(
			sharedDevice device,
			vk::SurfaceKHR const& surface,
			vk::Extent2D const& extent,
			vk::ImageUsageFlags        usage,
			vk::SwapchainKHR const& oldSwapChain,
			uint32_t                   graphicsFamilyIndex,
			uint32_t                   presentFamilyIndex);
		size_t getSwapchainLength() {
			return imageList.size();
		}
		~tSwapChain() {
			if (swapChain) {
				
				if (!device.expired()) {

					device.lock()->destroySwapchainKHR(swapChain);
					for (auto& iv : imageList) {
						iv->getImage()->VKHandle() = vk::Image();
					}
					swapChain = vk::SwapchainKHR();
				}
				else {
					reportDestroyedAfterDevice();
				}
			}
		}
	private:
		weakDevice device;
		vk::SwapchainKHR swapChain;
		std::vector<tImageView::SharedPtr> imageList;
	};

}