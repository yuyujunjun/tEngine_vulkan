#pragma once
#include"TGine.h"
namespace tEngine {
	class tFence {
	public:
		tFence(weakDevice device,vk::Fence f):device(device),fece(f){}
		vk::Fence getVkHandle() { return fece; }
		~tFence() {
			if (fece) {
				auto result= device->waitForFences(fece,true,static_cast<uint64_t>(-1));
				assert(result == vk::Result::eSuccess);
				device->destroyFence(fece);
				fece = VkFence(VK_NULL_HANDLE);
			}
		}
	private:
		weakDevice device;
		vk::Fence fece;
	};
	using FenceHandle = std::shared_ptr<tFence>;


	class FenceManager {
	public:
		FenceManager(weakDevice device) :device(device) {

		}
		FenceHandle requestSingaledFence(){
			if (signalfences.size() == 0) {
				return std::make_shared<tFence>(device, device->createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)));
			}
			else {
				auto f= signalfences.back();
				signalfences.pop_back();
				return f;
			}
		}
		void recycle(const FenceHandle& f) {
			if (f) {
				signalfences.emplace_back(f);
			}
		}
		~FenceManager() {
		
		}
	private:
		std::vector<FenceHandle> signalfences;
		weakDevice device;
	};
	class tSemaphore {
	public:
		tSemaphore(weakDevice device, vk::Semaphore s) :device(device), semaphore(s) {}
		const vk::Semaphore& getVkHandle()const { return semaphore; }
		~tSemaphore() {
			if (semaphore) {
				
				device->destroySemaphore(semaphore);
				semaphore = VkSemaphore(VK_NULL_HANDLE);
			}
		}
	private:
		weakDevice device;
		vk::Semaphore semaphore;
	};
	using SemaphoreHandle = std::shared_ptr<tSemaphore>;
	class SemaphoreManager {
	public:
		SemaphoreManager(weakDevice device) :device(device) {

		}
		SemaphoreHandle requestSemaphore() {
			if (semaphore.size() == 0) {
				return std::make_shared<tSemaphore>(device, device->createSemaphore(vk::SemaphoreCreateInfo()));
			}
			else {
				auto f = semaphore.back();
				semaphore.pop_back();
				return f;
			}
		}
		void recycle(const SemaphoreHandle& f) {
			if (f) {
				semaphore.emplace_back(f);
			}
		}
		~SemaphoreManager() {

		}
	private:
		std::vector<SemaphoreHandle> semaphore;
		weakDevice device;
	};
	class tSubmitInfo {
	public:
		void waitSemaphore(SemaphoreHandle& semaphore, vk::PipelineStageFlags dstStage) {
			waitSemaphores.emplace_back(semaphore->getVkHandle());
			waitStages.emplace_back(dstStage);
		}
		void signalSemaphore(SemaphoreHandle& semaphore) {
			signal.emplace_back(semaphore->getVkHandle());
		}
		void setCommandBuffers(const std::vector<CommandBufferHandle>& cbs) {
			for (auto& cb : cbs) {
				cmdBufers.emplace_back(cb->getVkHandle());
			}
		
		}
		void setCommandBuffers(const CommandBufferHandle& cb) {
			
				cmdBufers.emplace_back(cb->getVkHandle());
			

		}
		vk::SubmitInfo getSubmitInfo() {
			vk::SubmitInfo info;
			info.setCommandBuffers(cmdBufers);
			info.setWaitDstStageMask(waitStages);
			info.setSignalSemaphores(signal);
			info.setWaitSemaphores(waitSemaphores);
			return info;
		}
	private:
		std::vector<vk::Semaphore> waitSemaphores;
		std::vector<vk::PipelineStageFlags> waitStages;
		std::vector<vk::CommandBuffer> cmdBufers;
		std::vector<vk::Semaphore> signal;
	};

}