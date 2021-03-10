#pragma once
#include"Core.h"
namespace tEngine {

	class tPipeline {
	public:
		tPipeline(vk::PipelineBindPoint bindPoint):bindPoint(bindPoint) {}
		~tPipeline() {
			if (vkpipeline) {
				if (!device.expired()) {
					device.lock()->destroyPipeline(vkpipeline);
				}
				else {
					reportDestroyedAfterDevice();
				}
			}
		}
		vk::Pipeline& VkHandle() {
			return vkpipeline;
		}
		const vk::PipelineBindPoint& getBindPoint() {
			return bindPoint;
		}
		const vk::PipelineLayout& getPipelineLayout() {
			return pipelineLayout;
		}
	protected:
		vk::PipelineLayout pipelineLayout;
		vk::PipelineBindPoint bindPoint;
		vk::Pipeline vkpipeline;
	private:
		weakDevice device;
		
	};
	class GraphicsPipeline :public tPipeline {
	public:
		using SharedPtr = std::shared_ptr<GraphicsPipeline>;
		GraphicsPipeline():tPipeline(vk::PipelineBindPoint::eGraphics) {}
		
		
	};
	class ComputePipeline :public tPipeline {
	public:
		using SharedPtr = std::shared_ptr<ComputePipeline>;
		ComputePipeline() :tPipeline(vk::PipelineBindPoint::eCompute) {}
		
	};
}