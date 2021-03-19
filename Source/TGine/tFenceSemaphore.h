#pragma once
#include"CommandBufferBase.h"
namespace tEngine {
	class tFence {
	public:
		tFence(const Device* device,vk::Fence f):device(device),fece(f){}
		vk::Fence getVkHandle() { return fece; }
		~tFence();
	private:
		const Device* device;
		vk::Fence fece;
	};
	using FenceHandle = std::shared_ptr<tFence>;


	class FenceManager {
	public:
		FenceManager(const Device* device) :device(device) {

		}
		FenceHandle requestSingaledFence();
		void recycle(const FenceHandle& f) {
			if (f) {
				signalfences.emplace_back(f);
			}
		}
		~FenceManager() {
		
		}
	private:
		std::vector<FenceHandle> signalfences;
		const Device* device;
	};
	class tSemaphore {
	public:
		tSemaphore(const Device* device, vk::Semaphore s) :device(device), semaphore(s) {}
		const vk::Semaphore& getVkHandle()const { return semaphore; }
		~tSemaphore();
	private:
		const Device* device;
		vk::Semaphore semaphore;
	};
	using SemaphoreHandle = std::shared_ptr<tSemaphore>;
	class SemaphoreManager {
	public:
		SemaphoreManager(const Device* device) :device(device) {

		}
		SemaphoreHandle requestSemaphore();
		void recycle(const SemaphoreHandle& f) {
			if (f) {
				semaphore.emplace_back(f);
			}
		}
		~SemaphoreManager() {

		}
	private:
		std::vector<SemaphoreHandle> semaphore;
		const Device* device;
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