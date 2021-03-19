#include "tFenceSemaphore.h"
#include"tDevice.h"
namespace tEngine {

	tFence::~tFence() {
		if (fece) {
			auto result = device->waitForFences(fece, true, static_cast<uint64_t>(-1));
			assert(result == vk::Result::eSuccess);
			device->destroyFence(fece);
			fece = VkFence(VK_NULL_HANDLE);
		}
	}
	FenceHandle FenceManager::requestSingaledFence() {
		if (signalfences.size() == 0) {
			return std::make_shared<tFence>(device, device->createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)));
		}
		else {
			auto f = signalfences.back();
			signalfences.pop_back();
			return f;
		}
	}
	SemaphoreHandle SemaphoreManager::requestSemaphore() {
		if (semaphore.size() == 0) {
			return std::make_shared<tSemaphore>(device, device->createSemaphore(vk::SemaphoreCreateInfo()));
		}
		else {
			auto f = semaphore.back();
			semaphore.pop_back();
			return f;
		}
	}
	tSemaphore::~tSemaphore() {
		if (semaphore) {

			device->destroySemaphore(semaphore);
			semaphore = VkSemaphore(VK_NULL_HANDLE);
		}
	}
}