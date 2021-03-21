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
		clock_t time=0;
		uint32_t imageIdx = -1;
		std::function<void(double)> prepareStage;
		std::function<void(double, CommandBufferHandle& cb)> loopStage;
	
		void Update(const std::function<void(double)>& f) {
			prepareStage = f;
		}
		
		void Record(const std::function<void(double,CommandBufferHandle& cb)>& f) {
			loopStage = f;
		}
		void Render(ThreadContext* threadContext) {
			double deltaTime = time==0?0:(static_cast<double>(clock()) - time) / 1e3;
			time = clock();
			SemaphoreHandle	acquireSemaphore = device->getSemaphoreManager()->requestSemaphore();
			SemaphoreHandle presentSemaphore = device->getSemaphoreManager()->requestSemaphore();
			FenceHandle fence = device->getFenceManager()->requestSingaledFence();
			
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
			loopStage(imageIdx, cb);
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
			device->waitForFences(fence->getVkHandle(), true, -1);
			threadContext->cmdBuffers[imageIdx]->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
			device->getFenceManager()->recycle(fence);
			device->getSemaphoreManager()->recycle(acquireSemaphore);
			device->getSemaphoreManager()->recycle(presentSemaphore);
		}
		void Loop(ThreadContext* threadContext){
			SemaphoreHandle	acquireSemaphore = device->getSemaphoreManager()->requestSemaphore();
			SemaphoreHandle presentSemaphore = device->getSemaphoreManager()->requestSemaphore();
			FenceHandle fence = device->getFenceManager()->requestSingaledFence();
			time = clock();
			while (!glfwWindowShouldClose(gWindow)) {
				double deltaTime = (static_cast<double>(clock()) - time) / 1e3;
				time = clock();
				auto currentBuffer = (VkResult)device->acquireNextImageKHR(swapChain->getVkHandle(), -1, acquireSemaphore->getVkHandle(), {}, &imageIdx);
				if (currentBuffer == VK_ERROR_OUT_OF_DATE_KHR || currentBuffer == VK_SUBOPTIMAL_KHR) {
					updateSwapChain(gWindow, surface, swapChain, device.get());
				}
				else if (currentBuffer != VK_SUCCESS) {
					throw std::runtime_error("failed to present swap chain image!");
				}
				
				prepareStage(deltaTime);
				auto& cb = threadContext->cmdBuffers[imageIdx];
				auto result = device->waitForFences(fence->getVkHandle(), true, -1);
				device->resetFences(fence->getVkHandle());
				cb->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
				cb->begin(vk::CommandBufferUsageFlags());
				loopStage(deltaTime,cb);
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
			device->getFenceManager()->recycle(fence);
			device->getSemaphoreManager()->recycle(acquireSemaphore);
			device->getSemaphoreManager()->recycle(presentSemaphore);
		}
		~tEngineContext() {
	
			swapChain.reset();
			device->clearDeviceObject();
			device = nullptr;
		}

	};
	
	inline void setupFrameBufferBySwapchain(RenderPassHandle handle) {
		auto& context = tEngineContext::context;
		handle->SetImageView("back", context.swapChain->getImage(context.imageIdx));
		handle->SetImageView("depth", context.swapChain->getDepth());
	}
	
}