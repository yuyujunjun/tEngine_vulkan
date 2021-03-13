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
		friend class tShaderInterface;
		using SharedPtr = std::shared_ptr<tShader>;
		static SharedPtr Create(uniqueDevice& device) {
			return std::make_shared<tShader>(device);
		}
		tShader(uniqueDevice& device) :device(device) {}
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
		std::unique_ptr<tShaderInterface>& getInterface() {
			if (interface == nullptr) {
				interface = std::make_unique<tShaderInterface>();
			}
		}
	private:
		void CreateShaderLayout();
		void AddShaderModule(std::string fileName, vk::ShaderStageFlags stageFlag);
		bool isCreate = false;
		std::vector<std::shared_ptr<ShaderAsset>> shaderAsset;
		std::unique_ptr<tShaderInterface> interface;
		weakDevice device;
	};
	class tShaderInterface {
	public:
		friend class CommandBufferBase_;
		//DECLARE_SHARED(tShaderInterface)
		//static SharedPtr Create(uniqueDevice& device, std::shared_ptr<tDescriptorPool>& descpool, const tShader::SharedPtr& shader) {
		//	return std::make_shared<tShaderInterface>(device,descpool, shader);
		//}
		//tShaderInterface(tShader* shader);
		tShaderInterface( const tShader::SharedPtr& shader) : base_shader(shader) {
			SetShader(shader);
		};

		//只是缓存，仅有拷贝操作
		void SetBuffer(std::string name, BufferHandle buffer) {
			assert(BindMap.count(name) != 0);
			auto group = BindMap[name];
			buffers[group.first][group.second] = buffer;
		}
		//Copy value to buffer just before binding descriptorset
		void uploadUniformBuffer() {
			for (int set_number = 0; set_number < setCount(); ++set_number) {
				for (const auto& buf : buffers[set_number]) {
					auto& data = buffer_data[set_number][buf.first];
					auto& off = offsets[set_number][buf.first];
					buf.second->deviceMemory->SetRange(data.data(), data.size(), off);
					off += data.size();
					if (off + data.size() > buf.second->deviceMemory->getSize()) {
						off = 0;
					}
				}
			}
		}
		//只是缓存，仅有拷贝操作
		void SetImage(std::string name, tImageView::SharedPtr image) {
			assert(BindMap.count(name) != 0);
			auto group = BindMap[name];
			images[group.first][group.second] = image;
		}
		template <typename T>
		void SetValue(std::string valueName, T value, int RangeId) {
			assert(ValueMap.count(valueName) != 0);
			auto& pa = ValueMap[valueName];
			memcpy(buffer_data[pa.set_number][pa.bind_number].data() + pa.offset, &value, sizeof(value));
		}

		
		template<typename T>
		void SetPushConstant(std::string name, T value) {

			bool HasSet = false;
			auto& pushConstant = base_shader->pushConstant;
			for (int i = 0; i < base_shader->pushConstant.size(); i++) {
				if (base_shader->pushConstant[i].name == name) {
					int size = sizeof(T);
					int a = sizeof(char);
					assert(size == pushConstant[i].size);
					//assert(offset == pushConstant[i].offset);
					memcpy(pushConstantBlock.data() + pushConstant[i].offset, &value, pushConstant[i].size);
					//	memcpy((char*)PushConstantBlock + PushConstant[i].offset, &value, sizeof(T));
						/*T* address = PushConstantBlock + offset;
						*address = value;*/
					HasSet = true;
					break;
				}

			}
			assert(HasSet && "Push Constant Name may be wrong or Push Constant size may be zero");
		}
		void SetShader(const tShader::SharedPtr& shader);
		const tShader::SharedPtr& getShader() const {
			return base_shader;
		}
		const std::vector<std::unordered_map<uint32_t, BufferHandle>>& getBuffers()const {
			return buffers;
		}
		const std::vector<std::unordered_map<uint32_t, uint32_t>>& getOffsets()const {
			return offsets;
		}
		const std::vector<std::unordered_map<uint32_t, tImageView::SharedPtr>>& getImages()const {
			return images;
		}
		size_t setCount() {
			return buffers.size();
		}
		//Ordered descriptorSets: nullptr for there is no set

		//vk::PipelineLayout vkpipelineLayout;
		//vk::PipelineBindPoint vkpipelineBindPoint;
	protected:


		struct sValueMap {
			uint32_t set_number;
			uint32_t bind_number;
			uint32_t offset;
		};
		std::unordered_map<std::string, sValueMap> ValueMap;//Value,set, binding
		std::unordered_map<std::string, std::pair<uint8_t, uint16_t>> BindMap;//Resource Name,set,bind. Tell which set and binding each image or buffer belong to

		//Consecutive Set,interleved binding
	//	std::vector<tDescriptorSets::SharedPtr> descriptorSets;
		std::vector<std::unordered_map<uint32_t, uint32_t>> offsets;
		std::vector<std::unordered_map<uint32_t, std::vector<char>>>  buffer_data;
		std::vector<std::unordered_map<uint32_t, BufferHandle>>  buffers;
		std::vector<std::unordered_map<uint32_t, tImageView::SharedPtr>>  images;
		std::vector<char> pushConstantBlock;
	private:

		const tShader::SharedPtr& base_shader;
	

	};
	
}