#include"EngineContext.h"

#include"vulkan/vulkan.h"

#include"vulkan/vulkan.hpp"
#include"vulkan/vulkan_core.h"
#include"utils.hpp"
#include<mutex>

#include"Queue.h"
#include"Device.h"
#include"SwapChain.h"
#include"CommandBufferBase.h"
namespace tEngine {

	


	tEngineContext::tEngineContext(vk::Extent2D extent) {
		vk::Instance instance =
			vk::su::createInstance("AppName", "tEngine", GetInstanceLayers(), GetInstanceExtensions(), VK_API_VERSION_1_2);
		//	context = std::make_unique<tEngineContext>(instance, vk::Extent2D(width, height));
#if !defined( NDEBUG )
		auto debugUtilsMessenger =
			static_cast<vk::Instance>(instance).createDebugUtilsMessengerEXT(vk::su::makeDebugUtilsMessengerCreateInfoEXT());

#endif
		auto phyDevice = instance.enumeratePhysicalDevices().front();
		vk::PhysicalDeviceFeatures features;
		features.setFillModeNonSolid(true);
		features.setLogicOp(true);
		vk::Device vkdevice = vk::su::createDevice(phyDevice, findQueueFamilyIndex(phyDevice.getQueueFamilyProperties(), vk::QueueFlagBits::eGraphics), vk::su::getDeviceExtensions(), &features);
		device = std::make_unique<Device>(vkdevice, instance, extent);
	}


	ThreadContext::ThreadContext(Device* device) {
		//DescriptorPool
		
		//table = std::make_unique<currentDescriptorTable>(descriptorPool);
		//CommandPool
		vk::CommandPoolCreateInfo poolInfo = {};
		poolInfo.queueFamilyIndex = device->getPhysicalDevice().graphicsQueuefamilyId;
		poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		
		cmdPool = std::make_shared<tEngine::tCommandPool>(device,device->createCommandPool(poolInfo),poolInfo.queueFamilyIndex);
		//CommandBuffer
		vk::CommandBufferAllocateInfo cbInfo;
		cbInfo.commandBufferCount = static_cast<uint32_t>(device->swapChain->getSwapchainLength());
		cbInfo.commandPool = cmdPool->vkCommandPool;
		auto& cbs = device->allocateCommandBuffers(cbInfo);

		for (const auto& cb : cbs) {
			cmdBuffers.emplace_back( std::make_shared<CommandBuffer>(device,cb,cmdPool));
		}
		
	}

}