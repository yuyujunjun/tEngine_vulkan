#pragma once 
#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include"vulkan/vulkan.hpp"
#include<GLFW/glfw3.h>
#include"tDescriptorPool.h"
#include"tResource.h"
#include<unordered_map>
#include"tAssetLoadManager.h"
#include"CommandBufferBase.h"
#include"tShader.h"
#include"../MeshBuffer.h"
#include"CTPL/ctpl_stl.h"
#include"GLFW/glfw3.h"
#include"tFenceSemaphore.h"
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

	struct tEngineContext {
	public:
		static tEngineContext context;
		tEngineContext();
		uniqueDevice device;
		
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