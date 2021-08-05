#include"Material.h"
#include"Buffer.h"
#include"ShaderInterface.h"
#include"Shader.h"
#include<set>
namespace tEngine {
	//Return CPU Block
	GpuBlockBuffer findBlockByVarName(std::string name,const tShader* shader) {
		for (auto s : shader->setInfos) {
			for (auto b : s.blockBuffers) {
				for (auto m : b.second) {
					if (m.name == name) {
						return b.second;
					}
				}
			}
		}
		assert(false);
	}
	void Material::SetValue(const std::string& name, const void* value, size_t size) {
		bool hasName = false;
		//size_t offset = shader->getShader()->getVarSetBinding(name).offset;

		if (storedValue.count(name) == 0) {
			value_offset vo;
			vo.data = std::vector<uint8_t>(size);
		//	vo.offset = offset;
			vo.bufferName = findBlockByVarName(name,shader->getShader()).name;
			storedValue[name] = vo;
		}
		assert(storedValue[name].data.size() >= size);
		memcpy(storedValue[name].data.data(), (value), size);
	}
	void Material::flushBuffer(){
		std::unordered_map<std::string,BufferRangeManager*> bufferName;
		//for value set by (SetValue()), request buffer and copy value to buffer
		for (const auto& d : this->storedValue) {
			//request buffer range
			auto bufferRange = shader->getShader()->requestBufferRange(d.second.bufferName);
			if (bufferName.count(d.second.bufferName) == 0) {
				bufferRange->NextRangenoLock();
				bufferName[d.second.bufferName] = bufferRange;
			}
			//auto buffer = bufferRange->buffer();
			//auto offset = bufferRange->getOffset();
			//buffer->setRange(d.second.data.data(), d.second.offset + offset, d.second.data.size());
			
			
		}
		//Update DescriptorSet Info(don't update real descriptorSet)
		for (auto& b : bufferName) {
			SetBuffer(b.first, b.second->buffer(), b.second->getOffset());
		}
		for (const auto& d : this->storedValue) {
			shader->SetValueOnBuffer(d.first, d.second.data.size(),d.second.data.data());
		}

	}
	void Material::SetBuffer(const std::string& name, const BufferHandle& buffer, size_t offset ) {
		
		shader->SetBuffer(name, buffer, offset);
	}
	void Material::SetImage(std::string name, ImageHandle image, vk::ImageView vkView , StockSampler sampler) {
		shader->SetImage(name, image, vkView, sampler);
	}
	Material::Material(tShader* shader):shader(new tShaderInterface(shader)) {
		
	}
}