#include"tEngineContext.h"

#include"vulkan/vulkan.h"

#include"vulkan/vulkan.hpp"
#include"vulkan/vulkan_core.h"
#include"utils.hpp"
#include<mutex>


namespace tEngine {
	 std::unique_ptr<tEngineContext> tEngineContext::context;
	
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

#ifdef DEBUG
		// if the build is Debug then attempt to enable the VK_EXT_debug_report extension to aid with debugging
#if defined(VK_EXT_debug_report) && !defined(VK_USE_PLATFORM_MACOS_MVK)
		extensions.push_back(pvrvk::VulkanExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, (uint32_t)-1));
#endif
		// if the build is Debug then attempt to enable the VK_EXT_debug_utils extension to aid with debugging
#if defined(VK_EXT_debug_utils) && !defined(VK_USE_PLATFORM_MACOS_MVK)
		extensions.push_back(pvrvk::VulkanExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, (uint32_t)-1));
#endif
		// if the build is Debug then attempt to enable the VK_EXT_validation_features extension to aid with debugging
#ifdef VK_EXT_validation_features
		extensions.push_back(pvrvk::VulkanExtension(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME, (uint32_t)-1));
#endif
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
			context->graphicsQueuefamilyId = pair.first;
			context->presentQueuefamilyId = pair.second;
			std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
			context->computeQueuefamilyId = findQueueFamilyIndex(queueFamilyProperties, vk::QueueFlagBits::eCompute);

			context->device =
				std::make_shared<Device>(vk::su::createDevice(physicalDevice, context->graphicsQueuefamilyId, vk::su::getDeviceExtensions()));
			context->device->getPhysicalDevice() = physicalDevice;
			auto& device = context->device;
			
		

		

			context->descriptorPool = std::make_shared<tDescriptorPool>(context->device);
			context->descriptorPool->addDescriptorInfo(vk::DescriptorType::eCombinedImageSampler, 10);
			context->descriptorPool->addDescriptorInfo(vk::DescriptorType::eUniformBufferDynamic, 10);
			context->descriptorPool->addDescriptorInfo(vk::DescriptorType::eStorageBufferDynamic, 10);
			context->swapChainData = std::make_shared<tSwapChain>(device->getPhysicalDevice(), context->device, context->surfaceData.surface, context->surfaceData.extent,
				vk::ImageUsageFlagBits::eColorAttachment |
				vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst, vk::SwapchainKHR(), context->graphicsQueuefamilyId, context->presentQueuefamilyId);

		}
		return context.get();
	}

}