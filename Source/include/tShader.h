#pragma once
#include"Core.h"
#include"tDescriptorPool.h"
#include<unordered_map>
#include"Reflector.h"

namespace tEngine {
	struct ShaderAsset;
	struct tShader {
		using SharedPtr = std::shared_ptr<tShader>;
		static SharedPtr Create(sharedDevice& device) {
			return std::make_shared<tShader>(device);
		}
		tShader(sharedDevice& device) :device(device) {}
		void AddShaderModule(std::string fileName,vk::ShaderStageFlags stageFlag);
		std::vector<vk::ShaderModule> shaderModule;
		std::vector<vk::ShaderStageFlags> stageFlags;
		std::vector<tDescSetsData> setsData;
		GpuBlockBuffer pushConstant;
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
	private:
		std::shared_ptr<ShaderAsset> shaderAsset;
		weakDevice device;
	};
	
}