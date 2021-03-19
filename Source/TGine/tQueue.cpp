#include"tQueue.h"
namespace tEngine {
	uint32_t findQueueFamilyIndex(std::vector<vk::QueueFamilyProperties>const& queueFamilyProperties, vk::QueueFlagBits bits) {
		std::vector<vk::QueueFamilyProperties>::const_iterator graphicsQueueFamilyProperty = std::find_if(
			queueFamilyProperties.begin(), queueFamilyProperties.end(), [bits](vk::QueueFamilyProperties const& qfp) {
				return qfp.queueFlags & bits;
			});
		assert(graphicsQueueFamilyProperty != queueFamilyProperties.end());
		return static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
	}
}