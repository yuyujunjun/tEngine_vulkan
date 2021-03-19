#pragma once 
#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include"vulkan/vulkan.hpp"
#include<GLFW/glfw3.h>

#include<unordered_map>
#include"EgTypes.h"
#include"tDevice.h"
#include"tSwapChain.h"
#include"tBuffer.h"
#include"tImage.h"
#include"tSampler.h"
#include"renderHelper.h"
#include"tFenceSemaphore.h"
#include"tFrameBuffer.h"
#include"tShaderInterface.h"
#include"tShader.h"
#include"MeshBuffer.h"
#include"tAssetLoadManager.h"
namespace tEngine {

	inline std::vector<std::string> GetInstanceLayers(bool forceLayers = true)
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
	inline 	std::vector<std::string> GetInstanceExtensions() {

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

	class tEngineContext {
	public:
		//static tEngineContext context;
		tEngineContext(vk::Extent2D extent);
		std::unique_ptr<Device> device;
		
	};
	
	struct ThreadContext {
		ThreadContext(Device* device);
		tEngine::CommandPoolHandle cmdPool;
		std::vector<tEngine::CommandBufferHandle> cmdBuffers;
		~ThreadContext() {
			for (auto& cb : cmdBuffers) {
				
			}
		}
	};
	
}