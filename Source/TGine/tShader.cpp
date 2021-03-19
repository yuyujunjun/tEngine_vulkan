#include"tShader.h"
#include"Reflector.h"

#include"tDescriptorShared.h"
namespace tEngine {
	
	std::vector<ResSetBinding> getResourceBindingInfo(const tShader* shader) {
		std::vector<ResSetBinding> result(shader->setCount());
		for (const auto& set : shader->setInfos) {
			result[set.set_number].resize(set.getBindingCount());
			for (uint32_t i = 0; i < set.data.bindings.size(); i++) {
				auto bId = set.data.bindings[i].binding;
				auto type = set.data.bindings[i].descriptorType;
				auto range = set.blockBuffers.at(bId).ByteSize();
				result[set.set_number][bId]= BindingResourceInfo(bId,type,{},{},{},{},0,range);
			}
			
		}
		return result;
	}

	void tShader::SetShaderModule(const vk::ArrayProxy<const std::string>& fileName, const vk::ArrayProxy<const vk::ShaderStageFlagBits>& stageFlag) {
		for (uint32_t i = 0; i < fileName.size(); ++i) {
			AddShaderModule(reinterpret_cast<const std::string*>(fileName.data())[i], reinterpret_cast<const vk::ShaderStageFlagBits*>(stageFlag.data())[i]);

		}
		MergeSet(setInfos);
		CreateShaderLayout();
		BuildNameBindingInfo();
		
	}
	void tShader::BuildNameBindingInfo() {
		for (auto& set : setInfos) {
			uint32_t set_number = set.set_number;
			for (auto& block : set.blockBuffers) {
				uint32_t bindIdx = block.first;
				blockToSetBinding[block.second.name] = { set_number,bindIdx };
				for (auto& value : block.second) {
					valueToSetBinding[value.name] = { set_number,bindIdx,value.offset };
				}
			}
		}
	}


	std::shared_ptr<tShaderInterface> tShader::getInterface() {

		return std::make_shared<tShaderInterface>(this);

	}
	tShaderInterface::tShaderInterface(const tShader* shader) : base_shader(shader) {
		bindResources = getResourceBindingInfo(shader);
	};



	

	

}