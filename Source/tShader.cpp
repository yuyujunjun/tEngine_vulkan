#include"tShader.h"
#include"Reflector.h"
#include"tAssetLoadManager.h"
namespace tEngine {
	void tShader::SetShaderModule(const vk::ArrayProxy<const std::string>& fileName, vk::ShaderStageFlags stageFlag) {
		for (uint32_t i = 0; i < fileName.size(); ++i) {
			AddShaderModule(reinterpret_cast<const std::string*>(fileName.data())[i], stageFlag);
		}
		MergeSet(setInfos);
		CreateShaderLayout();
	}
	void tShader::CreateUniformBuffers() {
		uint32_t maxSet = 0;
		for (auto& set :setInfos) {
			maxSet = maxSet < set.set_number ? set.set_number : maxSet;
		}
		maxSet++;
		uniformBuffers.resize(maxSet);
		for (auto& set : setInfos) {
			uint32_t set_number = set.set_number;

			
			BufferCreateInfo bcif;
			bcif.domain = BufferDomain::Host;
			bcif.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

			for (auto& bindType : set.data.bindings) {
				switch (bindType.descriptorType) {
				case vk::DescriptorType::eUniformBufferDynamic:
					bcif.size = set.blockBuffers[bindType.binding].ByteSize();
					uniformBuffers[set_number][bindType.binding].SetBufferOffset( device->createBuffer(bcif),0);
					break;
				case vk::DescriptorType::eSampledImage:
					break;
				}

			}
		}
	}
	void tShader::CreateShaderLayout() {
		auto& d = device;
		if (setAllocator.size() == 0) {
			for (int i = 0; i < setInfos.size(); ++i) {
				setAllocator.emplace_back(tDescriptorSetAllocatorManager::manager.requestSetAllocator(d, setInfos[i].data));
			}
		}
		if (pipelinelayout == nullptr) {
			std::vector<DescriptorSetLayoutHandle> layouts;
			for (auto& set : setAllocator) {
				layouts.emplace_back(set->getLayout());
			}
			pipelinelayout = d->createPipelineLayout( layouts, pushConstant, allstageFlags);

		}
		isCreate = true;
	}
	void tShader::AddShaderModule(std::string fileName, vk::ShaderStageFlags stageFlag) {
		assert(!isCreate && "Can't change shader after set it to material");
		if (isCreate)throw("Can't change shader after set it to material");
		pipelinelayout = nullptr;
		setAllocator.clear();
		auto& d = device;
		shaderAsset.push_back( LoadShader(fileName));
		Reflector::reflectionShader(shaderAsset.back()->shaderReflection, setInfos, pushConstant, stageFlag);
		allstageFlags |= stageFlag;
		//To check stage Flags
		vk::ShaderModuleCreateInfo info;
		info.setCode( shaderAsset.back()->shaderSource);
		shaderModule.push_back(d->createShaderModule(info));
	}
	tShaderInterface tShader::getInterface() {

		return tShaderInterface(this);

	}
	tShaderInterface::tShaderInterface(const tShader* shader) : base_shader(shader) {
		uint32_t maxSet = 0;
		for (auto& set : shader->setInfos) {
			maxSet = maxSet < set.set_number ? set.set_number : maxSet;
		}
		maxSet++;
		BindingInfo.resize(maxSet);

		for (auto& set : shader->setInfos) {
			uint32_t set_number = set.set_number;
		
			for (auto& block : set.blockBuffers) {
				uint32_t bindIdx = block.first;
				BindMap[block.second.name] = {set_number,bindIdx};
				for (auto& value : block.second) {
					ValueMap[value.name] = {set_number,bindIdx,value.offset};
				}
				BindingInfo[set_number].buffer_data[bindIdx].resize(block.second.ByteSize());
				BindingInfo[set_number].offsets[bindIdx] = 0;
			
			}
			
			auto& resSetBinding = BindingInfo[set_number].resSetBinding;
			for (auto& bindType : set.data.bindings) {
				switch (bindType.descriptorType) {
				case vk::DescriptorType::eUniformBufferDynamic:
					BindingInfo[set_number].offsets[bindType.binding] = 0;
					resSetBinding.getBuffers()[bindType.binding].buffer = shader->uniformBuffers[set_number].at(bindType.binding).buffer();
					break;
				case vk::DescriptorType::eSampledImage:
					break;
				}
				
			}
		}
	};

}