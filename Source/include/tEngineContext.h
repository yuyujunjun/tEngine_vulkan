#pragma once

#include"vulkan/vulkan.hpp"
#include<GLFW/glfw3.h>
#include"tDescriptorPool.h"
#include"tResource.h"
#include<unordered_map>
#include"tAssetLoadManager.h"
#include"CommandBufferBase.h"
#include"tShader.h"
#include"tMaterial.h"
namespace tEngine {

	struct tEngineContext {
	public:
		tEngineContext(vk::Instance instance, vk::Extent2D resolution) :instance(instance),surfaceData(instance, "tang", resolution) {}
		static tEngineContext* Context();
		
		//std::unordered_map<vk::QueueFlagBits, uint32_t> queueFamilyIndex;
		vk::DebugUtilsMessengerEXT debugUtilsMessenger;
		vk::Instance instance;
		sharedDevice device;
		//vk::tPhysicalDevice physicalDevice;
		vk::su::SurfaceData surfaceData;
		tSwapChain::SharedPtr swapChainData;
		vk::Queue graphicsQueue;
		vk::Queue presentQueue;
		vk::Queue computeQueue;
		
		
		~tEngineContext() {
			device->waitIdle();
			//descriptorPool.reset();
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
	struct ThreadContext {
		ThreadContext(const tEngineContext*const context);


		tDescriptorPool::SharedPtr descriptorPool;
		tEngine::tCommandPool::SharedPtr cmdPool;
		std::vector<tEngine::CommandBuffer::SharedPtr> cmdBuffers;
		~ThreadContext() {
			
		}
	private:
		const tEngineContext*const context;
		
	};
	
}