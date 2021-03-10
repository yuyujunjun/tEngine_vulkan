#include"tShader.h"
#include"Reflector.h"
#include"tAssetLoadManager.h"
namespace tEngine {
	void tShader::AddShaderModule(std::string fileName, vk::ShaderStageFlags stageFlag) {
		auto& d = device.lock();
		shaderAsset=LoadShader(fileName);
		Reflector::reflectionShader(shaderAsset->shaderReflection, setsData, pushConstant, stageFlag);
		stageFlags.push_back(stageFlag);
		vk::ShaderModuleCreateInfo info;
		info.setCode( shaderAsset->shaderSource);
		shaderModule.push_back(d->createShaderModule(info));
	}
}