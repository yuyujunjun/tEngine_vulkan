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
#include"DescriptorPool.h"
#include"imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
namespace tEngine {
	tEngineContext tEngineContext::context;
	IMGUI IMGUI:: m_gui;
	 std::vector<std::string> GetInstanceLayers(bool forceLayers)
	{
		std::vector<std::string> vulkanLayers;
		//	std::vector<pvrvk::VulkanLayer> vulkanLayers;
		if (forceLayers)
		{
			// Enable both VK_LAYER_KHRONOS_validation and the deprecated VK_LAYER_LUNARG_standard_validation as the Loader will Handle removing duplicate layers
			vulkanLayers.push_back("VK_LAYER_KHRONOS_validation");
			//	vulkanLayers.push_back("VK_LAYER_LUNARG_standard_validation");
			//	vulkanLayers.push_back("VK_LAYER_LUNARG_assistant_layer");
			vulkanLayers.push_back("VK_LAYER_LUNARG_monitor");

		}
		//	layers.setLayers(vulkanLayers);
		return vulkanLayers;
	}
	 	std::vector<std::string> GetInstanceExtensions() {

		if (!glfwInit()) //Initialize GLFW
		{
			std::cout << "GLFW not initialized.\n";
			abort();
		}
		if (!glfwVulkanSupported()) //Any Vulkan-related function requires GLFW initialized
		{
			std::cout << "Vulkan not supported.\n";
			abort();
		}
		std::vector<std::string> extensions;


#ifdef GLFW_INCLUDE_VULKAN
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		for (uint32_t i = 0; i < glfwExtensionCount; i++) {
			extensions.push_back(glfwExtensions[i]);
		}

#endif
		extensions.push_back("VK_EXT_debug_utils");
		
		return extensions;
	}
	 GLFWwindow* createWindow(vk::Extent2D extent, std::string name ) {
		//window
		glfwInit();
		glfwSetErrorCallback([](int error, const char* msg) {
			std::cerr << "glfw: "
				<< "(" << error << ") " << msg << std::endl;
			});


		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		return glfwCreateWindow(extent.width, extent.height, name.c_str(), nullptr, nullptr);
	}
	 vk::SurfaceKHR createSurface(vk::Instance instance, GLFWwindow* gWindow) {
		VkSurfaceKHR _surface;
		VkResult err = glfwCreateWindowSurface(instance, gWindow, nullptr, &_surface);
		if (err != VK_SUCCESS)
			throw std::runtime_error("Failed to create window!");
		return  vk::SurfaceKHR(_surface);
	}
	 SwapChainHandle createSwapChain(Device* device, vk::SurfaceKHR surface, vk::Extent2D extent, vk::SwapchainKHR oldSwapchain ) {
		return createSwapChain(device, surface, extent, vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst, oldSwapchain, device->getPhysicalDevice().graphicsQueuefamilyId, device->getPhysicalDevice().presentQueuefamilyId);
	}
	 void updateSwapChain(GLFWwindow* gWindow, VkSurfaceKHR surface, SwapChainHandle& swapChain, Device* device) {

		{
			//recreate swapChain

			int width, height;
			glfwGetFramebufferSize(gWindow, &width, &height);
			while (width == 0 || height == 0) {
				glfwGetFramebufferSize(gWindow, &width, &height);
				glfwWaitEvents();
			}
			//vk::Format depth_format = swapChain->getDepth()->getFormat();
			swapChain = createSwapChain(device, (vk::SurfaceKHR) surface, vk::Extent2D(width, height), swapChain->getVkHandle());
			//swapChain->createDepth(depth_format);
		}
	}
	 vk::Instance createInstance() {
		return  vk::su::createInstance("AppName", "tEngine", GetInstanceLayers(), GetInstanceExtensions(), VK_API_VERSION_1_2);
	}

	 vk::Device createDevice(vk::Instance instance) {
		auto phyDevice = instance.enumeratePhysicalDevices().front();
		vk::PhysicalDeviceFeatures features;
		features.setFillModeNonSolid(true);
		features.setLogicOp(true);
		features.setDepthBiasClamp(true);
		features.setDepthClamp(true);
		features.setDepthBounds(false);
	
		return vk::su::createDevice(phyDevice, findQueueFamilyIndex(phyDevice.getQueueFamilyProperties(), vk::QueueFlagBits::eGraphics), vk::su::getDeviceExtensions(), &features);

	}


	void tEngineContext::Set(vk::Instance instance, GLFWwindow* gWindow, vk::SurfaceKHR surface,vk::Extent2D extent){
		this->gWindow = (gWindow);
		this->surface = (surface);
		//	context = std::make_unique<tEngineContext>(instance, vk::Extent2D(width, height));
#if !defined( NDEBUG )
		this-> debugUtilsMessenger =
			static_cast<vk::Instance>(instance).createDebugUtilsMessengerEXT(vk::su::makeDebugUtilsMessengerCreateInfoEXT());

#endif
		auto vkdevice=createDevice(instance);
		this->device = std::make_unique<Device>(vkdevice, instance,surface);
		this->swapChain = createSwapChain(device.get(), surface, extent);
		hasInitialized_ = true;
				
	}


	ThreadContext::ThreadContext(tEngineContext* context) {
		//DescriptorPool
		
		//table = std::make_unique<currentDescriptorTable>(descriptorPool);
		//CommandPool
		vk::CommandPoolCreateInfo poolInfo = {};
		poolInfo.queueFamilyIndex = context->device->getPhysicalDevice().graphicsQueuefamilyId;
		poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		
		cmdPool = std::make_shared<tEngine::tCommandPool>(context->device.get(), context->device->createCommandPool(poolInfo),poolInfo.queueFamilyIndex);
		//CommandBuffer
		vk::CommandBufferAllocateInfo cbInfo;
		cbInfo.commandBufferCount = static_cast<uint32_t>(context->swapChain->getSwapchainLength());
		cbInfo.commandPool = cmdPool->vkCommandPool;
		auto& cbs = context->device->allocateCommandBuffers(cbInfo);

		for (const auto& cb : cbs) {
			cmdBuffers.emplace_back( std::make_shared<CommandBuffer>(context->device.get(),cb,cmdPool));
		}
		
	}
	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
	{
		auto& io = ImGui::GetIO();
	
		if (io.WantCaptureMouse)return;
		
	
	}
	static void mouseButtonCallback(GLFWwindow* window, int /*button*/, int /*action*/, int /*mods*/)
	{
		auto& io = ImGui::GetIO();
		if (io.WantCaptureMouse)return;
		
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		//tEngineContext::context.cameraManipulator.setMousePosition(glm::ivec2(static_cast<int>(xpos), static_cast<int>(ypos)));
	}
	static void scrollCallback(GLFWwindow* window, double /*xoffset*/, double yoffset)
	{
		
		auto& io = ImGui::GetIO();
		if (io.WantCaptureMouse)return;
		//tEngineContext::context.cameraManipulator.wheel(static_cast<int>(yoffset));
	}
	void IMGUI::Init(tEngineContext& context) {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.WantCaptureMouse=true;
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForVulkan(context.gWindow, true);
		
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = context.instance;
		init_info.PhysicalDevice = context.device->getPhysicalDevice().physicalDevice;
		init_info.Device = *context.device.get();
		init_info.QueueFamily = context.device->getPhysicalDevice().graphicsQueuefamilyId;
		init_info.Queue = context.device->requestQueue(init_info.QueueFamily);
		init_info.PipelineCache = context.device->getPipelineCache();
		auto& device = context.device;
		std::vector<vk::DescriptorPoolSize> pool_sizes =
		{
			{ vk::DescriptorType::eSampler, 1000 },
			{ vk::DescriptorType::eCombinedImageSampler, 1000 },
			{ vk::DescriptorType::eSampledImage, 1000 },
			{  vk::DescriptorType::eStorageImage, 1000 },
			{  vk::DescriptorType::eUniformTexelBuffer, 1000 },
			{  vk::DescriptorType::eUniformBuffer, 1000 },
			{  vk::DescriptorType::eUniformBufferDynamic, 1000 },
			{  vk::DescriptorType::eStorageBuffer, 1000 },
			{  vk::DescriptorType::eStorageBufferDynamic, 1000 },
			{  vk::DescriptorType::eStorageImage, 1000 },
			{  vk::DescriptorType::eInputAttachment, 1000 }
		};
		vk::DescriptorPoolCreateInfo setInfo;
		setInfo.setPoolSizes(pool_sizes);
		setInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
		setInfo.setMaxSets(1000);
		auto desciptorPool = device->createDescriptorPool(setInfo);
		IMGUI::uiPool = std::make_shared<tDescriptorPool>(device.get(), desciptorPool);
		init_info.DescriptorPool = desciptorPool;
		init_info.Allocator = nullptr;
		init_info.MinImageCount = 2;
		init_info.ImageCount = context.swapChain->getSwapchainLength();
		init_info.CheckVkResultFn = nullptr;

		IMGUI::uiPass = getUIRenderpass(context.device.get(), context.swapChain->getFormat());
		
		ImGui_ImplVulkan_Init(&init_info, uiPass->getVkHandle());
		auto cmd = device->requestTransientCommandBuffer();
		oneTimeSubmit(cmd, device->requestQueue(cmd->getQueueFamilyIdx()), [&](CommandBufferHandle& cmd) {ImGui_ImplVulkan_CreateFontsTexture(cmd->getVkHandle()); });

		
	}
	void ContextInit() {
		auto& context = tEngineContext::context;
		if (!context.hasInitialized()) {
			const int width = 1200;
			const int height = 800;
			context.instance = createInstance();
			auto gWindow = createWindow(vk::Extent2D(width, height));
			auto surface = createSurface(context.instance, gWindow);
			//	auto vkDevice = createDevice(instance);
			tEngineContext::context.Set(context.instance, gWindow, surface, vk::Extent2D(width, height));
			IMGUI::m_gui.Init(context);
			
		}
	}
	void tEngineContext::coreRender(CommandBufferHandle* cb_list, SemaphoreHandle	acquireSemaphore
	, SemaphoreHandle presentSemaphore, FenceHandle fence) {
		glfwPollEvents();
		double deltaTime = time == 0 ? 0 : (static_cast<double>(clock()) - time) / 1e3;
		time = clock();

		auto currentBuffer = (VkResult)device->acquireNextImageKHR(swapChain->getVkHandle(), -1, acquireSemaphore->getVkHandle(), {}, &imageIdx);
		if (currentBuffer == VK_ERROR_OUT_OF_DATE_KHR || currentBuffer == VK_SUBOPTIMAL_KHR) {
			updateSwapChain(gWindow, surface, swapChain, device.get());
		}
		else if (currentBuffer != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}
		//IMGUI
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		int w, h;
		w = swapChain->getExtent().width;
		h = swapChain->getExtent().height;
		IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");
		io.DisplaySize = ImVec2((float)w, (float)h);
		if (w > 0 && h > 0)
			io.DisplayFramebufferScale = ImVec2((float)1, (float)1);
		
		
		auto& uiPass = IMGUI::m_gui.uiPass;
		uiPass->SetImageView("back", swapChain->getImage(imageIdx));
		//uiPass->SetImageView("depth", swapChain->getDepth());
		//uiPass->setClearValue("back", { 0,0,0,1 });
		//uiPass->setDepthStencilValue("depth", 1);
		
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		prepareStage(deltaTime);
	

		//
		//auto& cb = threadContext->cmdBuffers[imageIdx];
		auto result = device->waitForFences(fence->getVkHandle(), true, -1);
		device->resetFences(fence->getVkHandle());
		auto& cb = cb_list[imageIdx];
		cb->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
		cb->begin(vk::CommandBufferUsageFlags());
		loopStage(deltaTime, cb);

		

		cb->beginRenderPass(IMGUI::m_gui.uiPass, IMGUI::m_gui.uiPass->requestFrameBuffer(), true);
		ImGui::Render();
		ImDrawData* draw_data = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(draw_data, cb->getVkHandle());
		cb->endRenderPass();

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
		
	
	}
	void tEngineContext::Render(ThreadContext* threadContext) {

		SemaphoreHandle	acquireSemaphore = device->getSemaphoreManager()->requestSemaphore();
		SemaphoreHandle presentSemaphore = device->getSemaphoreManager()->requestSemaphore();
		FenceHandle fence = device->getFenceManager()->requestSingaledFence();

		coreRender(threadContext->cmdBuffers.data(),acquireSemaphore,presentSemaphore,fence);
		device->waitForFences(fence->getVkHandle(), true, -1);
		threadContext->cmdBuffers[imageIdx]->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
		device->getFenceManager()->recycle(fence);
		device->getSemaphoreManager()->recycle(acquireSemaphore);
		device->getSemaphoreManager()->recycle(presentSemaphore);
	}
	void tEngineContext::Loop(ThreadContext* threadContext) {
		SemaphoreHandle	acquireSemaphore = device->getSemaphoreManager()->requestSemaphore();
		SemaphoreHandle presentSemaphore = device->getSemaphoreManager()->requestSemaphore();
		FenceHandle fence = device->getFenceManager()->requestSingaledFence();
		time = clock();
		while (!glfwWindowShouldClose(gWindow)) {
			coreRender(threadContext->cmdBuffers.data(), acquireSemaphore, presentSemaphore, fence);
		}
		device->waitForFences(fence->getVkHandle(), true, -1);
		for (auto& cb : threadContext->cmdBuffers) {
			cb->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
		}
		device->getFenceManager()->recycle(fence);
		device->getSemaphoreManager()->recycle(acquireSemaphore);
		device->getSemaphoreManager()->recycle(presentSemaphore);
	}
	tEngineContext::~tEngineContext() {
		IMGUI::m_gui.Destroy();
		swapChain.reset();
		threadContext.clear();
		device->clearDeviceObject();
		device = nullptr;
	}
	
}