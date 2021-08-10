#include "Light.h"
#include"Buffer.h"
#include"Device.h"
#include"Image.h"

namespace tEngine {

	std::shared_ptr<BufferRangeManager> requestLightPropertyBuffer(const Device* device) {
		static std::shared_ptr<BufferRangeManager> lightPropertyBuffer;
		if (lightPropertyBuffer == nullptr) {
			GpuBlockBuffer lightBufferBlock;
			lightBufferBlock.AddElement("lightPos", sizeof(glm::vec3));
			lightBufferBlock.AddElement("halfblockSize", sizeof(int));
			lightBufferBlock.AddElement("world_to_shadow", sizeof(glm::mat4));
			lightBufferBlock.AddElement("depthMapSize", sizeof(glm::vec2));
			lightBufferBlock.AddElement("maxKernelSize", sizeof(int));
			lightBufferBlock.AddElement("lightIntensity", sizeof(float));
			lightBufferBlock.AddElement("lightPosArea", sizeof(float));
			lightPropertyBuffer = createBufferFromBlock(device, lightBufferBlock, 30);
		}
		return lightPropertyBuffer;
	}
	Light::Light(Device* device) {
		if (shadowMap == nullptr) {
			auto shadowMapCreateInfo = ImageCreateInfo::render_target(1024, 1024, (VkFormat)vk::Format::eR32Sfloat);
			shadowMapCreateInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
			shadowMap = createImage(device, shadowMapCreateInfo);
		}
		this->device = device;
	}
	void Light::updateBuffer() {
		auto bufferRange = requestLightPropertyBuffer(device);
		bufferRange->NextRangenoLock();
		offset = bufferRange->getOffset();
		lightPropertyBuffer = bufferRange->buffer();
	}
}
