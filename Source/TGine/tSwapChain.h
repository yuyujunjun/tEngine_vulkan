#pragma once
#include"vulkan/vulkan.hpp"
namespace tEngine {
	class Device;
	class tImage;
	using ImageHandle = std::shared_ptr<tImage>;
	
	class tSwapChain {
	public:
		using SharedPtr = std::shared_ptr<tSwapChain>;
		tSwapChain(Device* device, vk::SwapchainKHR const& surface, vk::Extent2D extent);
		void setImages(std::vector<ImageHandle>& images) {
			imageList = images;
		}
		size_t getSwapchainLength() {
			return imageList.size();
		}
		vk::Format getFormat();
		~tSwapChain();
		vk::Extent2D getExtent() {
			return extent;
		}
		const vk::SwapchainKHR& getVkHandle()const {
			return swapChain;
		}
		const ImageHandle& getImage(uint32_t imageIdx)const {
			return imageList[imageIdx];
		}
	private:
		const Device* device;
		vk::SwapchainKHR swapChain;
		vk::Extent2D extent;
		std::vector<ImageHandle> imageList;
	};
	using SwapChainHandle = std::shared_ptr<tSwapChain>;
	SwapChainHandle createSwapChain(Device* device, vk::SurfaceKHR const& surface, vk::Extent2D const& extent, vk::ImageUsageFlags usage, vk::SwapchainKHR const& oldSwapChain, uint32_t graphicsQueueFamilyIndex, uint32_t presentQueueFamilyIndex);
	VkFormat default_depth_format(VkPhysicalDevice physicalDevice);
	VkFormat default_depth_stencil_format(VkPhysicalDevice physicalDevice);

}