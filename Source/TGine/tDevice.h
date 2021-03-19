#pragma once

#include<unordered_map>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include"tSampler.h"
#include"vmaAllocation.h"
namespace tEngine {
	class CommandBuffer;
	using CommandBufferHandle = std::shared_ptr <CommandBuffer>;
	class tCommandPool;
	using CommandPoolHandle = std::shared_ptr<tCommandPool>;
	class tSwapChain;
	using SwapChainHandle = std::shared_ptr<tSwapChain>;
	class SemaphoreManager;
	class FenceManager;
	class tPhysicalDevice {
	public:
		tPhysicalDevice() {};
		
		void SetPhysicalDevice(vk::PhysicalDevice physicalDevice) {
			this->physicalDevice = physicalDevice;
			memoryProperties = physicalDevice.getMemoryProperties();
			deviceProperties = physicalDevice.getProperties();
			
		}
		const vk::PhysicalDeviceMemoryProperties& getMemoryProperties()const {

			return memoryProperties;
		}
		const vk::PhysicalDeviceProperties& getProperties()const {
			return deviceProperties;
		}
		const vk::FormatProperties& getFormatProperties(vk::Format format)const {
			return physicalDevice.getFormatProperties(format);
		}
		const vk::PhysicalDevice& getPhysicalDevice() const{
			return physicalDevice;
		}
		vk::PhysicalDevice& operator()() {
			return physicalDevice;
		}
		bool bUniqueQueueFamily()const {
			assert(graphicsQueuefamilyId == presentQueuefamilyId&&"our system need graphic queue equals to prsent queue");
			return graphicsQueuefamilyId == presentQueuefamilyId &&
				presentQueuefamilyId == computeQueuefamilyId &&
				computeQueuefamilyId == transferQueuefamilyId;
		}


		uint32_t graphicsQueuefamilyId=-1;
		uint32_t presentQueuefamilyId=-1;
		uint32_t computeQueuefamilyId=-1;
		uint32_t transferQueuefamilyId = -1;
		vk::PhysicalDeviceMemoryProperties memoryProperties;
		vk::PhysicalDeviceProperties deviceProperties;
		vk::PhysicalDevice physicalDevice;
	//	vk::FormatProperties formatProps;
	};

	class Device :public vk::Device{
	public:
		friend class tEngineContext;
		Device(VkDevice device,  vk::Instance instance,vk::Extent2D extent);

		void Device::initStockSamplers();
		void freeAllocation(VmaAllocation allocation)const;
		
		SamplerHandle Device::getSampler(const StockSampler& sampler)const;
		const tPhysicalDevice& getPhysicalDevice()const { return physicalDevice; }
		tPhysicalDevice& getPhysicalDevice() { return physicalDevice; }
		const VmaAllocator& getAllocator()const { return allocator; ; }
		const vk::PipelineCache& getPipelineCache()const { return pipelineCache; }
		CommandBufferHandle requestTransientCommandBuffer() const ;
		
		void clearDeviceObject();
		vk::Queue requestQueue(uint32_t familyIndx)const {
			return queues.at(familyIndx);
		}
		vk::Queue requestQueue(vk::QueueFlagBits flag)const {
			return queues.at(getQueueId(flag));
		}
		GLFWwindow* getWindow() {
			return gWindow;
		}
		SwapChainHandle swapChain;
		std::unordered_map<uint32_t, vk::Queue> queues;
		uint32_t getQueueId(vk::QueueFlagBits queueType)const;
		const std::unique_ptr<FenceManager>& getFenceManager()const { return fenceManager; }
		const std::unique_ptr<SemaphoreManager>& getSemaphoreManager()const { return semaphoreManager; }
		//vk::Device* operator->()const { return const_cast<vk::Device*>( &vkDevice); }
		//operator vk::Device() { return vkDevice; }
	private:
		vk::DebugUtilsMessengerEXT debugUtilsMessenger;
		vk::Instance instance;
		GLFWwindow* gWindow;
		vk::SurfaceKHR surface;
		std::unique_ptr<FenceManager> fenceManager;
		std::unique_ptr<SemaphoreManager> semaphoreManager;

		vk::PipelineCache pipelineCache;
		tPhysicalDevice physicalDevice;
		VmaAllocator allocator;
		CommandPoolHandle transientPool;
		std::array<SamplerHandle, static_cast<unsigned>(StockSampler::Count)> samplers;
	};
	//using const Device* = const Device*;
	using uniqueDevice = std::unique_ptr<Device>;
	

}