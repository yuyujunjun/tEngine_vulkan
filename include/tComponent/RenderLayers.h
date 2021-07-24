#pragma once
#include<vulkan/vulkan.hpp>
namespace tEngine {
	enum class RenderLayer :uint8_t {
		Everything = 255,//11111111
		Default = 254,//11111110
		UI = 1//00000001
	};
}