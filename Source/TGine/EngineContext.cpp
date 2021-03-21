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
	tEngineContext tEngineContext::context;
	 std::vector<std::string> GetInstanceLayers(bool forceLayers)
	{
		std::vector<std::string> vulkanLayers;
		//	std::vector<pvrvk::VulkanLayer> vulkanLayers;
		if (forceLayers)
		{
			// Enable both VK_LAYER_KHRONOS_validation and the deprecated VK_LAYER_LUNARG_standard_validation as the Loader will handle removing duplicate layers
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
			swapChain = createSwapChain(device, (vk::SurfaceKHR) surface, vk::Extent2D(width, height), swapChain->getVkHandle());
			swapChain->createDepth();
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
		CameraManipulator::MouseButton mouseButton =
			(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
			? CameraManipulator::MouseButton::Left
			: (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
			? CameraManipulator::MouseButton::Middle
			: (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
			? CameraManipulator::MouseButton::Right
			: CameraManipulator::MouseButton::None;
		if (mouseButton != CameraManipulator::MouseButton::None)
		{
			CameraManipulator::ModifierFlags modifiers = 0;
			if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
			{
				modifiers |= static_cast<uint32_t>(CameraManipulator::ModifierFlagBits::Alt);
			}
			if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			{
				modifiers |= static_cast<uint32_t>(CameraManipulator::ModifierFlagBits::Ctrl);
			}
			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			{
				modifiers |= static_cast<uint32_t>(CameraManipulator::ModifierFlagBits::Shift);
			}


			tEngineContext::context.cameraManipulator.mouseMove(
				glm::ivec2(static_cast<int>(xpos), static_cast<int>(ypos)), mouseButton, modifiers);
		}
	}
	static void mouseButtonCallback(GLFWwindow* window, int /*button*/, int /*action*/, int /*mods*/)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		tEngineContext::context.cameraManipulator.setMousePosition(glm::ivec2(static_cast<int>(xpos), static_cast<int>(ypos)));
	}
	static void scrollCallback(GLFWwindow* window, double /*xoffset*/, double yoffset)
	{

		tEngineContext::context.cameraManipulator.wheel(static_cast<int>(yoffset));
	}
	void ContextInit() {
		const int width = 1200;
		const int height = 800;
		auto instance = createInstance();
		auto gWindow = createWindow(vk::Extent2D(width, height));
		auto surface = createSurface(instance, gWindow);
	//	auto vkDevice = createDevice(instance);
		tEngineContext::context.Set(instance, gWindow, surface, vk::Extent2D(width, height));
		tEngineContext::context.swapChain->createDepth(vk::Format::eD32Sfloat);
		glfwSetCursorPosCallback(gWindow, cursor_position_callback);
		glfwSetMouseButtonCallback(gWindow, mouseButtonCallback);
		glfwSetScrollCallback(gWindow, scrollCallback); 
		tEngineContext::context.cameraManipulator.setWindowSize(glm::u32vec2(width, height));
	}
	
}