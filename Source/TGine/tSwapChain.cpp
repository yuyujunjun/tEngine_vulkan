#include"tSwapChain.h"
#include"tResource.h"
namespace tEngine {
	vk::Format tSwapChain::getFormat() {
		return vk::Format(imageList[0]->get_format());
	}
	
	tSwapChain::tSwapChain(Device* device, vk::SwapchainKHR const& swapChain, vk::Extent2D extent) :device(device), swapChain(swapChain), extent(extent) {

	}
	
}