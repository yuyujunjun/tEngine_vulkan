#include"tEngineContext.h"

#include"vulkan/vulkan.h"

#include"vulkan/vulkan.hpp"
#include"vulkan/vulkan_core.h"
#include"utils.hpp"
#include<mutex>


namespace tEngine {
	std::unique_ptr<tEngineContext> tEngineContext::context;
	tDescriptorSetAllocatorManager tDescriptorSetAllocatorManager::manager;
	uint32_t findQueueFamilyIndex(std::vector<vk::QueueFamilyProperties>const& queueFamilyProperties, vk::QueueFlagBits bits) {
		std::vector<vk::QueueFamilyProperties>::const_iterator graphicsQueueFamilyProperty = std::find_if(
			queueFamilyProperties.begin(), queueFamilyProperties.end(), [bits](vk::QueueFamilyProperties const& qfp) {
				return qfp.queueFlags & bits;
			});
		assert(graphicsQueueFamilyProperty != queueFamilyProperties.end());
		return static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
	}
	
	std::vector<std::string> GetInstanceLayers(bool forceLayers = true)
	{
		
		std::vector<std::string> vulkanLayers;
		if (forceLayers)
		{
			// Enable both VK_LAYER_KHRONOS_validation and the deprecated VK_LAYER_LUNARG_standard_validation as the Loader will handle removing duplicate layers
			vulkanLayers.push_back("VK_LAYER_KHRONOS_validation");
			//vulkanLayers.push_back("VK_LAYER_LUNARG_standard_validation");
			//vulkanLayers.push_back("VK_LAYER_LUNARG_assistant_layer");
			//vulkanLayers.push_back("VK_LAYER_LUNARG_monitor");
		}

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

#ifdef VK_KHR_surface
		extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
		extensions.push_back(pvrvk::VulkanExtension(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME, (uint32_t)-1));
#elif defined VK_USE_PLATFORM_WIN32_KHR
		extensions.push_back(pvrvk::VulkanExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, (uint32_t)-1));
#elif defined(VK_USE_PLATFORM_XCB_KHR)
		extensions.push_back(pvrvk::VulkanExtension(VK_KHR_XCB_SURFACE_EXTENSION_NAME, (uint32_t)-1));
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
		extensions.push_back(pvrvk::VulkanExtension(VK_KHR_XLIB_SURFACE_EXTENSION_NAME, (uint32_t)-1));
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
		extensions.push_back(pvrvk::VulkanExtension(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME, (uint32_t)-1));
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
		extensions.push_back(pvrvk::VulkanExtension(VK_MVK_MACOS_SURFACE_EXTENSION_NAME, (uint32_t)-1));
#elif defined(VK_KHR_display) // NullWS
		extensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#endif
#ifdef VK_KHR_get_physical_device_properties2
		extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif


		extensions.push_back("VK_KHR_win32_surface");
	
		return extensions;
	}
	tEngineContext* tEngineContext::Context() {
		if (context == nullptr) {
			
	
			context = std::make_unique<tEngineContext>(vk::su::createInstance("AppName", "tEngine",GetInstanceLayers(),GetInstanceExtensions()), vk::Extent2D(600, 800));
#if !defined( NDEBUG )
				context->debugUtilsMessenger =
				context->instance.createDebugUtilsMessengerEXT(vk::su::makeDebugUtilsMessengerCreateInfoEXT());
#endif

			
			auto physicalDevice = context->instance.enumeratePhysicalDevices().front();
			auto pair = vk::su::findGraphicsAndPresentQueueFamilyIndex(physicalDevice, context->surfaceData.surface);
			
			vk::Device vkdevice = vk::su::createDevice(physicalDevice, pair.first, vk::su::getDeviceExtensions());
			VmaAllocatorCreateInfo allocatorInfo = {};
			allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
			allocatorInfo.physicalDevice = physicalDevice;
			allocatorInfo.device = vkdevice;
			allocatorInfo.instance = context->instance;
			VmaAllocator allocator;
			vmaCreateAllocator(&allocatorInfo, &allocator);
			context->device =std::make_unique<Device>(vkdevice, physicalDevice,allocator);

			auto& device = context->device;
			device->initStockSamplers();
			device->getPhysicalDevice().graphicsQueuefamilyId = pair.first;
			device->getPhysicalDevice().presentQueuefamilyId = pair.second;
			auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
			device->getPhysicalDevice().computeQueuefamilyId = findQueueFamilyIndex(queueFamilyProperties, vk::QueueFlagBits::eCompute);
			device->getPhysicalDevice().transferQueuefamilyId = findQueueFamilyIndex(queueFamilyProperties, vk::QueueFlagBits::eTransfer);

			context->graphicsQueue = device->getQueue(device->getPhysicalDevice().graphicsQueuefamilyId,0);
			context->computeQueue = device->getQueue(device->getPhysicalDevice().computeQueuefamilyId, 0);
			context->presentQueue = device->getQueue(device->getPhysicalDevice().presentQueuefamilyId, 0);
			device->swapChain = createSwapChain(context->device.get(), context->surfaceData.surface, context->surfaceData.extent, vk::ImageUsageFlagBits::eColorAttachment |
					vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst, vk::SwapchainKHR(), device->getPhysicalDevice().graphicsQueuefamilyId, device->getPhysicalDevice().presentQueuefamilyId);

			context->pipelineCache = device->createPipelineCache(vk::PipelineCacheCreateInfo());
		}
		return context.get();
	}
	ThreadContext::ThreadContext(const tEngineContext* const context):context(context) {
		//DescriptorPool
		
		//table = std::make_unique<currentDescriptorTable>(descriptorPool);
		//CommandPool
		vk::CommandPoolCreateInfo poolInfo;
		poolInfo.queueFamilyIndex = context->device->getPhysicalDevice().graphicsQueuefamilyId;
		
		cmdPool = tCommandPool::Create(context->device.get(), context->device->createCommandPool(poolInfo));
		//CommandBuffer
		vk::CommandBufferAllocateInfo cbInfo;
		cbInfo.commandBufferCount = static_cast<uint32_t>(context->device->swapChain->getSwapchainLength());
		cbInfo.commandPool = cmdPool->vkCommandPool;
		auto& cbs = context->device->allocateCommandBuffers(cbInfo);

		for (const auto& cb : cbs) {
			cmdBuffers.emplace_back( CommandBuffer::Create(context->device.get(),cb));
		}
		
	}

}