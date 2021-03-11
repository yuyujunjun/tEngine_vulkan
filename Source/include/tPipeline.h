#pragma once
#include"Core.h"
#include"tDescriptorPool.h"
namespace tEngine {
	class tPipelineLayout {
	public:
		DECLARE_SHARED(tPipelineLayout)
		static SharedPtr Create(sharedDevice& device,std::vector<tDescriptorSetLayout::SharedPtr>& desclayouts,GpuBlockBuffer& pushConstant,vk::ShaderStageFlags  stageFlags){
			return std::make_shared<tPipelineLayout>(device,desclayouts,pushConstant,stageFlags);
		}
		tPipelineLayout(sharedDevice& device, std::vector<tDescriptorSetLayout::SharedPtr>& desclayouts, GpuBlockBuffer& pushConstant, vk::ShaderStageFlags  stageFlags):device(device) {
			vk::PipelineLayoutCreateInfo  info;
			
			std::vector<vk::PushConstantRange> ranges(pushConstant.size());
		
			for (int idx = 0; idx < ranges.size();++idx) {
				auto& pus = pushConstant[idx];
				ranges[idx].setOffset(pus.offset);
				ranges[idx].setSize(pus.size);
				ranges[idx].setStageFlags(stageFlags);
			}
			info.setPushConstantRanges(ranges);
			std::vector<vk::DescriptorSetLayout> layouts(desclayouts.size());
			for (int i = 0; i < layouts.size(); ++i) {
				layouts[i] = desclayouts[i]->vkLayout;
			}
			info.setSetLayouts(layouts);
			vkLayout=device->createPipelineLayout(info);
		
		}
		~tPipelineLayout() {
			if (vkLayout) {
				device.lock()->destroyPipelineLayout(vkLayout);
				vkLayout = vk::PipelineLayout();
			}
		}
		vk::PipelineLayout vkLayout;
	private:
		weakDevice device;
	};
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
		const tPipelineLayout::SharedPtr& getPipelineLayout() {
			return pipelineLayout;
		}
	protected:
		tPipelineLayout::SharedPtr pipelineLayout;
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