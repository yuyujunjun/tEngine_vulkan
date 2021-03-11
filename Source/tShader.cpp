#include"tShader.h"
#include"Reflector.h"
#include"tAssetLoadManager.h"
namespace tEngine {
	void tShader::AddShaderModule(const vk::ArrayProxy<const std::string>& fileName, vk::ShaderStageFlags stageFlag) {
		for (uint32_t i = 0; i < fileName.size(); ++i) {
			AddShaderModule(reinterpret_cast<const std::string*>(fileName.data())[i], stageFlag);
		}
		CreateShaderLayout();
	}
	void tShader::CreateShaderLayout() {
		auto& d = device.lock();
		if (setlayouts.size() == 0) {
			for (int i = 0; i < setsData.size(); ++i) {
				setlayouts.emplace_back(tDescriptorSetLayout::Create(d, setsData[i]));
			}
		}
		if (pipelinelayout == nullptr) {
			pipelinelayout = tPipelineLayout::Create(d, setlayouts, pushConstant, stageFlags);

		}
		isCreate = true;
	}
	void tShader::AddShaderModule(std::string fileName, vk::ShaderStageFlags stageFlag) {
		assert(!isCreate && "Can't change shader after set it to material");
		if (isCreate)throw("Can't change shader after set it to material");
		pipelinelayout = nullptr;
		setlayouts.clear();
		auto& d = device.lock();
		shaderAsset.push_back( LoadShader(fileName));
		Reflector::reflectionShader(shaderAsset.back()->shaderReflection, setsData, pushConstant, stageFlags);
		//To check stage Flags
		vk::ShaderModuleCreateInfo info;
		info.setCode( shaderAsset.back()->shaderSource);
		shaderModule.push_back(d->createShaderModule(info));
	}
}