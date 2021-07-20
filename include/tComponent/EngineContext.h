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
#include"Log.h"
namespace tEngine {
	
	
	void ContextInit();
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
		//static GpuBlockBuffer cameraBufferBlock;
		tEngineContext() :hasInitialized_(false),gWindow(nullptr) {

		};
		void Set(vk::Instance instance,GLFWwindow* gWindow,vk::SurfaceKHR surface, vk::Extent2D extent);
		
		std::unique_ptr<Device> device;
		GLFWwindow* gWindow;
		vk::SurfaceKHR surface;
		SwapChainHandle swapChain;
		vk::Instance instance;
		vk::DebugUtilsMessengerEXT debugUtilsMessenger;
		ThreadContext* AddThreadContext(uint32_t idx=0) 
		{
			if (threadContext.size() > idx) {
				return threadContext[idx].get();
			}
			LOG(tEngine::LogLevel::Information, "Add new ThreadContext");
			threadContext.emplace_back(std::make_unique<ThreadContext>(this));
			return threadContext.back().get();
		}


		void Update(const std::function<void(double)>& f) {
			prepareStage = f;
		}
		void Record(const std::function<void(double,CommandBufferHandle& cb)>& f) {
			loopStage = f;
		}
		void Render(ThreadContext* threadContext);
		void Loop(ThreadContext* threadContext);
		~tEngineContext();
		bool hasInitialized() const { return hasInitialized_; }
		uint32_t getImageIdx() const{ return imageIdx; }
	private:
		clock_t time = 0;
		uint32_t imageIdx = -1;
		std::function<void(double)> prepareStage;
		std::function<void(double, CommandBufferHandle& cb)> loopStage;
		void coreRender(CommandBufferHandle* cb, SemaphoreHandle	acquireSemaphore
			, SemaphoreHandle presentSemaphore, FenceHandle fence);
		std::vector<std::unique_ptr<ThreadContext>> threadContext;
		bool hasInitialized_ = false;
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
	//inline void setupFrameBufferBySwapchain(RenderPassHandle Handle) {
	//	auto& context = tEngineContext::context;
	//	Handle->SetImageView("back", context.swapChain->getImage(context.imageIdx));
	////	Handle->SetImageView("depth", context.swapChain->getDepth());
	//}
	
}