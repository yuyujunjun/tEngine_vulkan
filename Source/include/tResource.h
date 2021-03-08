#pragma once
#include"vulkan/vulkan.h"
#include"vulkan/vulkan.hpp"
#include"Core.h"
namespace tEngine {
	/// <summary>Aligns a given number based on the given alignment</summary>
/// <param name="numberToAlign">A number ot align based alignment</param>
/// <param name="alignment">The value to which the numberToAlign will be aligned</param>
/// <returns>An aligned value</returns>
	template<typename t1, typename t2>
	inline t1 align(t1 numberToAlign, t2 alignment)
	{
		if (alignment)
		{
			t1 align1 = numberToAlign % (t1)alignment;
			if (!align1) { align1 += (t1)alignment; }
			numberToAlign += t1(alignment) - align1;
		}
		return numberToAlign;
	}
	class DeviceMemory {
	public:
		DeviceMemory():size(0),deviceMemory(vk::DeviceMemory()){}
		DeviceMemory(sharedDevice device, vk::DeviceSize size, vk::MemoryRequirements const& requirements, vk::MemoryPropertyFlags const& propertyFlags);
		~DeviceMemory() {
			if (deviceMemory) {
				if (!device.expired()) {
					device.lock()->freeMemory(deviceMemory);
					deviceMemory = vk::DeviceMemory();
				}
				else {
					reportDestroyedAfterDevice();
				}
			}
		}
		vk::DeviceMemory deviceMemory;
	private:
		inline bool bHost() {
			return (static_cast<uint32_t>(memoryProperties & vk::MemoryPropertyFlagBits::eHostCached) != 0)||
				(static_cast<uint32_t>(memoryProperties & vk::MemoryPropertyFlagBits::eHostCoherent) != 0)||
				(static_cast<uint32_t>(memoryProperties & vk::MemoryPropertyFlagBits::eHostVisible) != 0);
		}
		inline bool bHostCoherent() {
			return (static_cast<uint32_t>(memoryProperties & vk::MemoryPropertyFlagBits::eHostCached) != 0) ||
				(static_cast<uint32_t>(memoryProperties & vk::MemoryPropertyFlagBits::eHostCoherent) != 0);
		}
		void* memory;
		vk::MemoryPropertyFlags memoryProperties;
		vk::DeviceSize size;
		std::weak_ptr<vk::Device> device;
		
	};
	class tBuffer {
	public:
		using SharedPtr = std::shared_ptr<tBuffer>;
		tBuffer(sharedDevice device,vk::DeviceSize size, vk::MemoryPropertyFlags propertyFlags,vk::BufferUsageFlags usage):device(device),size(size), propertyFlags(propertyFlags),usage(usage) {
			
			CreateBufferWithMemory(device);
		}
		tBuffer(sharedDevice device, vk::DeviceSize size, vk::MemoryPropertyFlags propertyFlags);
		~tBuffer() {
			deviceMemory.reset();
			if (buffer) {
				if (!device.expired()) {
					device.lock()->destroyBuffer(buffer);
					buffer = vk::Buffer();
				}
				else {
					reportDestroyedAfterDevice();
				}
			}
		}
	private:
		inline static uint32_t minBufferAlignment(const Device* const device) {
			int alignment = std::max(static_cast<uint32_t>(
				device->getDeviceProperties().limits.minUniformBufferOffsetAlignment),
				static_cast<uint32_t>(
					device->getDeviceProperties().limits.minStorageBufferOffsetAlignment));
			return alignment;
		}
		void CreateBufferWithMemory(sharedDevice& device) {
			size = align(size, minBufferAlignment(device.get()));
			buffer = device->createBuffer(vk::BufferCreateInfo(vk::BufferCreateFlags(), size, usage));
			auto memoryRequirements = device->getBufferMemoryRequirements(buffer);
			size = memoryRequirements.size;
			deviceMemory = std::make_unique<DeviceMemory>(device, size, memoryRequirements, propertyFlags);
			device->mapMemory(deviceMemory->deviceMemory, 0, size);
			device->bindBufferMemory(buffer, deviceMemory->deviceMemory, 0);
		}
		weakDevice device;
		vk::Buffer buffer;
		std::unique_ptr<DeviceMemory> deviceMemory;
		vk::DeviceSize size;
		vk::BufferUsageFlags usage;
		vk::MemoryPropertyFlags propertyFlags;
	};
	class tSampler {
		tSampler() {}
		~tSampler() {
			if (sampler) {
				if (!device.expired()) {
					device.lock()->destroySampler(sampler); 
					sampler = vk::Sampler();
				}
				else {
					reportDestroyedAfterDevice();
				}
			}
		}
	private:
		std::weak_ptr<vk::Device> device;
		vk::Sampler sampler;
	};
	struct tImage {
	public:
		
		friend class tImageView;
		tImage(sharedDevice& device, vk::Image image) :vkImage(image), device(device) {
		
		}
		
		void AllocateMemory(sharedDevice& device, vk::MemoryPropertyFlags const& propertyFlags) {
			auto memoryRequirements = device->getImageMemoryRequirements(vkImage);
			deviceMemory = std::make_unique<DeviceMemory>(device, memoryRequirements.size, memoryRequirements, propertyFlags);
			device->bindImageMemory(vkImage, deviceMemory->deviceMemory, 0);
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
		vk::Image& VKHandle() {
			return vkImage;
		}
		vk::Image vkImage;
	private:
		
		std::weak_ptr<vk::Device> device;
		std::unique_ptr<DeviceMemory> deviceMemory;
	};
	class tImageView {
	public:
		using SharedPtr = std::shared_ptr<tImageView>;
		tImageView(sharedDevice device, vk::ImageViewCreateInfo imageViewCreateInfo) :device(device), createInfo(imageViewCreateInfo){
			image = std::make_shared<tImage>(device, createInfo.image);
			imageView = device->createImageView(createInfo);
		}
		
		~tImageView(){
			if (imageView) {
				if (!device.expired()) {
					device.lock()->destroyImageView(imageView);
					imageView = vk::ImageView();
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
	private:
		std::shared_ptr<tImage> image;
		vk::ImageViewCreateInfo createInfo;
		
		vk::ImageView imageView;
		std::weak_ptr<vk::Device> device;
	};
	inline tImageView::SharedPtr CreateImageViewWithImage(sharedDevice& device,vk::ImageCreateInfo imageInfo,vk::ImageViewCreateInfo viewInfo,vk::MemoryPropertyFlags& memoryProperties) {
		//Create Image
		auto image = device->createImage(imageInfo);
		//Create imageView
		viewInfo.image = image;
		auto imageView = std::make_shared<tImageView>(device,viewInfo);
		imageView->getImage()->AllocateMemory(device, memoryProperties);
		return imageView;
	}
	class tSwapChain {
	public:
		using SharedPtr = std::shared_ptr<tSwapChain>;
		tSwapChain(vk::PhysicalDevice const& physicalDevice,
			sharedDevice device,
			vk::SurfaceKHR const& surface,
			vk::Extent2D const& extent,
			vk::ImageUsageFlags        usage,
			vk::SwapchainKHR const& oldSwapChain,
			uint32_t                   graphicsFamilyIndex,
			uint32_t                   presentFamilyIndex);
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