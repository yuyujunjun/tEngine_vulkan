#pragma once
#include"vulkan/vulkan.hpp"
namespace tEngine {
	uint32_t findQueueFamilyIndex(std::vector<vk::QueueFamilyProperties>const& queueFamilyProperties, vk::QueueFlagBits bits);
}