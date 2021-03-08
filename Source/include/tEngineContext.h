#pragma once

#include"vulkan/vulkan.hpp"
#include<GLFW/glfw3.h>
#include"tDescriptorPool.h"
#include"tResource.h"
#include<unordered_map>
#include"tAssetLoadManager.h"
namespace tEngine {

	struct tEngineContext {
	public:
		tEngineContext(vk::Instance instance, vk::Extent2D resolution) :instance(instance),surfaceData(instance, "tang", resolution) {}
		static tEngineContext* Context();
		
		std::unordered_map<vk::QueueFlagBits, uint32_t> queueFamilyIndex;
		vk::DebugUtilsMessengerEXT debugUtilsMessenger;
		vk::Instance instance;
		sharedDevice device;
		//vk::PhysicalDevice physicalDevice;
		vk::su::SurfaceData surfaceData;
		tSwapChain::SharedPtr swapChainData;
		tDescriptorPool::SharedPtr descriptorPool;
		uint32_t graphicsQueuefamilyId;
		uint32_t presentQueuefamilyId;
		uint32_t computeQueuefamilyId;
		
		~tEngineContext() {
			descriptorPool.reset();
			swapChainData.reset();
			
			device->destroy();
			instance.destroySurfaceKHR(surfaceData.surface);
#if !defined( NDEBUG )
			instance.destroyDebugUtilsMessengerEXT(debugUtilsMessenger);
#endif
			instance.destroy();
		}
	private:
		static std::unique_ptr<tEngineContext> context;

	};
	
	
}