#pragma once
#include"Core.h"
#include"tDescriptorPool.h"
#include"tPipeline.h"
#include<unordered_map>
#include"Reflector.h"

namespace tEngine {
	struct ShaderAsset;
	class tShader {
	public:
		using SharedPtr = std::shared_ptr<tShader>;
		static SharedPtr Create(sharedDevice& device) {
			return std::make_shared<tShader>(device);
		}
		tShader(sharedDevice& device) :device(device) {}
		void SetShaderModule(const vk::ArrayProxy<const std::string>& fileName, vk::ShaderStageFlags stageFlag);
		
		~tShader() {
			if (shaderModule.size() > 0) {
				auto& d = device.lock();
				for (int i = 0; i < shaderModule.size(); ++i) {
					if (shaderModule[i]) {
						d->destroyShaderModule(shaderModule[i]);
						shaderModule[i] = vk::ShaderModule();
					}
				}
			}
			
		}

		std::vector<vk::ShaderModule> shaderModule;
		vk::ShaderStageFlags stageFlags;

		std::vector<tDescSetsDataWithSetNumber> setsnumberData;//setsnumberData coresspond to setlayouts
		std::vector<tDescriptorSetLayout::SharedPtr> setlayouts;

		tPipelineLayout::SharedPtr pipelinelayout;
		GpuBlockBuffer pushConstant;
	private:
		void CreateShaderLayout();
		void AddShaderModule(std::string fileName, vk::ShaderStageFlags stageFlag);
		bool isCreate = false;
		std::vector<std::shared_ptr<ShaderAsset>> shaderAsset;
		weakDevice device;
	};
	
}