#pragma once 
#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include"vulkan/vulkan.hpp"
#include<GLFW/glfw3.h>

#include<unordered_map>
#include"EgTypes.h"
#include"Device.h"
#include"SwapChain.h"
#include"Buffer.h"
#include"Image.h"
#include"Sampler.h"
#include"renderHelper.h"
#include"FenceSemaphore.h"
#include"FrameBuffer.h"
#include"ShaderInterface.h"
#include"Shader.h"
#include"MeshBuffer.h"
#include"AssetLoadManager.h"
#include"utils.hpp"
#include"Queue.h"
#include<functional>
#include"Camera.h"
namespace tEngine {

	
	void ContextInit();

	 std::vector<std::string> GetInstanceLayers(bool forceLayers = true);
	 	std::vector<std::string> GetInstanceExtensions();
	 GLFWwindow* createWindow(vk::Extent2D extent, std::string name = "View");
	 vk::SurfaceKHR createSurface(vk::Instance instance, GLFWwindow* gWindow);
	 SwapChainHandle createSwapChain(Device* device, vk::SurfaceKHR surface, vk::Extent2D extent, vk::SwapchainKHR oldSwapchain = {});
	 void updateSwapChain(GLFWwindow* gWindow, VkSurfaceKHR surface, SwapChainHandle& swapChain, Device* device);
	 vk::Instance createInstance();

	 vk::Device createDevice(vk::Instance instance); 
	 struct ThreadContext {
		 ThreadContext(tEngineContext* context);
		 tEngine::CommandPoolHandle cmdPool;
		 std::vector<tEngine::CommandBufferHandle> cmdBuffers;
		 ~ThreadContext() {
			 for (auto& cb : cmdBuffers) {

			 }
		 }
	 };
	class tEngineContext {
	public:
		static tEngineContext context;
		tEngineContext() = default;
		void Set(vk::Instance instance,GLFWwindow* gWindow,vk::SurfaceKHR surface, vk::Extent2D extent);
		std::unique_ptr<Device> device;
		GLFWwindow* gWindow;
		vk::SurfaceKHR surface;
		SwapChainHandle swapChain;
		vk::DebugUtilsMessengerEXT debugUtilsMessenger;
		CameraManipulator cameraManipulator;
	
		std::function<void(uint32_t)> prepareStage;
		std::function<void(uint32_t, CommandBufferHandle& cb)> loopStage;
	
		void Update(const std::function<void(uint32_t)>& f) {
			prepareStage = f;
		}
		
		void Record(const std::function<void(uint32_t,CommandBufferHandle& cb)>& f) {
			loopStage = f;
		}
		void Loop(ThreadContext* threadContext){
			SemaphoreHandle acquireSemaphore;
			SemaphoreHandle presentSemaphore;
			FenceHandle fence;
			fence = device->getFenceManager()->requestSingaledFence();
			acquireSemaphore = device->getSemaphoreManager()->requestSemaphore();
			presentSemaphore = device->getSemaphoreManager()->requestSemaphore();
			while (!glfwWindowShouldClose(gWindow)) {
				uint32_t imageIdx;
				auto currentBuffer = (VkResult)device->acquireNextImageKHR(swapChain->getVkHandle(), -1, acquireSemaphore->getVkHandle(), {}, &imageIdx);
				if (currentBuffer == VK_ERROR_OUT_OF_DATE_KHR || currentBuffer == VK_SUBOPTIMAL_KHR) {
					updateSwapChain(gWindow, surface, swapChain, device.get());
				}
				else if (currentBuffer != VK_SUCCESS) {
					throw std::runtime_error("failed to present swap chain image!");
				}

				prepareStage(imageIdx);
				auto& cb = threadContext->cmdBuffers[imageIdx];
				auto result = device->waitForFences(fence->getVkHandle(), true, -1);
				device->resetFences(fence->getVkHandle());
				cb->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
				cb->begin(vk::CommandBufferUsageFlags());
				loopStage(imageIdx,cb);
				cb->end();
				tSubmitInfo info;
				info.setCommandBuffers(cb);
				info.waitSemaphore(acquireSemaphore, vk::PipelineStageFlagBits::eFragmentShader);

				info.signalSemaphore(presentSemaphore);

				device->requestQueue(cb->getQueueFamilyIdx()).submit(info.getSubmitInfo(), fence->getVkHandle());

				auto presentInfo = vk::PresentInfoKHR(presentSemaphore->getVkHandle(), swapChain->getVkHandle(), imageIdx);
				auto presentResult = (VkResult)device->requestQueue(vk::QueueFlagBits::eGraphics).presentKHR(&presentInfo);
				if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
					updateSwapChain(gWindow, surface, swapChain, device.get());
				}
				else if (presentResult != VK_SUCCESS) {
					throw std::runtime_error("failed to present swap chain image!");
				}

				glfwPollEvents();

			}
			device->waitForFences(fence->getVkHandle(), true, -1);
			for (auto& cb : threadContext->cmdBuffers) {
				cb->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
			}
			acquireSemaphore = nullptr;
			presentSemaphore = nullptr;
			fence = nullptr;
		}
		~tEngineContext() {
	
			swapChain.reset();
			device->clearDeviceObject();
			device = nullptr;
		}

	};
	
	
	
}