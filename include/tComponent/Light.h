#pragma once
#include"tTransform.h"
#include"glm/glm.hpp"
#include"glm/matrix.hpp"
#include<memory>
namespace tEngine {
	class tImage;
	using ImageHandle = std::shared_ptr<tImage>;
	class BufferRangeManager;
	class tBuffer;
	using BufferHandle = std::shared_ptr<tBuffer>;
	class Device;
	struct Light {
		Light(Device* device);
		int halfblockSize = 8;
		int maxKernelSize = 8;
		float lightIntensity = 1;
		float lightArea = 5;
		glm::mat4 world_to_shadow;
		ImageHandle shadowMap;
		BufferHandle lightPropertyBuffer;
		uint32_t offset;
		void updateBuffer();
		
		const Device* device;
	};
	std::shared_ptr<BufferRangeManager> requestLightPropertyBuffer(const Device* device);
}
