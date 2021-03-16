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
#include"CTPL/ctpl_stl.h"
namespace tEngine {
	struct ThreadContext;
	struct tEngineContext {
	public:
		tEngineContext(vk::Instance instance, vk::Extent2D resolution) :instance(instance),surfaceData(instance, "tang", resolution) {}
		static tEngineContext* Context();
	
		//std::unordered_map<vk::QueueFlagBits, uint32_t> queueFamilyIndex;
		vk::DebugUtilsMessengerEXT debugUtilsMessenger;
		vk::Instance instance;
		uniqueDevice device;
		//vk::tPhysicalDevice physicalDevice;
		vk::su::SurfaceData surfaceData;
		
		vk::Queue graphicsQueue;
		vk::Queue presentQueue;
		vk::Queue computeQueue;
		vk::PipelineCache pipelineCache;
		std::unordered_map<uint32_t, uint32_t> threadID_to_index;
		
		~tEngineContext() {
			device->destroyPipelineCache(pipelineCache);
			device->waitIdle();
			//descriptorPool.reset();
			//swapChainData.reset();
		
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


		//tDescriptorPool::SharedPtr descriptorPool;
		//std::unique_ptr<currentDescriptorTable> table;
		tEngine::tCommandPool::SharedPtr cmdPool;
		std::vector<tEngine::CommandBuffer::SharedPtr> cmdBuffers;
		~ThreadContext() {
			
		}
	private:
		const tEngineContext*const context;
		
	};
	
}