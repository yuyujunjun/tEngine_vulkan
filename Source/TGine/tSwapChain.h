#pragma once
#include"Tgine.h"
namespace tEngine {
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
		weakDevice device;
		vk::SwapchainKHR swapChain;
		vk::Extent2D extent;
		std::vector<ImageHandle> imageList;
	};
	SwapChainHandle createSwapChain(Device* device, vk::SurfaceKHR const& surface, vk::Extent2D const& extent, vk::ImageUsageFlags usage, vk::SwapchainKHR const& oldSwapChain, uint32_t graphicsQueueFamilyIndex, uint32_t presentQueueFamilyIndex);
}