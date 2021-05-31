#pragma once 
#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include"vulkan/vulkan.hpp"
#include"GLFW/glfw3.h"
#include"imgui.h"
#include"imgui_impl_vulkan.h"
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
#include<imgui/imgui.h>
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
		vk::Instance instance;
		vk::DebugUtilsMessengerEXT debugUtilsMessenger;
		CameraManipulator cameraManipulator;
		clock_t time=0;
		uint32_t imageIdx = -1;
		std::function<void(double)> prepareStage;
		std::function<void(double, CommandBufferHandle& cb)> loopStage;
		ThreadContext* AddThreadContext() { threadContext.emplace_back(std::make_unique<ThreadContext>(this)); return threadContext.back().get(); }


		void Update(const std::function<void(double)>& f) {
			prepareStage = f;
		}
		void Record(const std::function<void(double,CommandBufferHandle& cb)>& f) {
			loopStage = f;
		}
		void coreRender(ThreadContext* threadContext, SemaphoreHandle	acquireSemaphore
			, SemaphoreHandle presentSemaphore, FenceHandle fence);
		void Render(ThreadContext* threadContext);
		void Loop(ThreadContext* threadContext);
		std::vector<std::unique_ptr<ThreadContext>> threadContext;
		~tEngineContext();
	};
	struct IMGUI {
		static IMGUI m_gui;
		 RenderPassHandle uiPass;
		 DescriptorPoolHandle uiPool;
		 void Init(tEngineContext& context);
		 void Destroy() {
			uiPass = nullptr; uiPool = nullptr;
			ImGui_ImplVulkan_Shutdown();
			ImGui::DestroyContext();
		}
		
	};
	inline void setupFrameBufferBySwapchain(RenderPassHandle handle) {
		auto& context = tEngineContext::context;
		handle->SetImageView("back", context.swapChain->getImage(context.imageIdx));
		handle->SetImageView("depth", context.swapChain->getDepth());
	}
	
}