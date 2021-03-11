#pragma once
#include"Core.h"
#include"tResource.h"
#include"tShader.h"
#include"tPipeline.h"
#include"tDescriptorPool.h"
#include"CommandBufferBase.h"
namespace tEngine {
	class tMaterial{
	public:
		DECLARE_SHARED(tMaterial)
		static SharedPtr Create(sharedDevice& device, std::shared_ptr<tDescriptorPool>& descpool) {
			return std::make_shared<tMaterial>(device,descpool);
		}
		//tMaterial(tShader* shader);
		tMaterial(sharedDevice& device,std::shared_ptr<tDescriptorPool>& descpool ) :device(device),descPool(descpool),vkpipelineBindPoint(vk::PipelineBindPoint()) {
			
		};
		
		//只是缓存，仅有拷贝操作
		void SetBuffer(std::string name, tBuffer::SharedPtr buffer) {
			assert(BindMap.count(name) != 0);
			auto group = BindMap[name];
			buffers[group.first][group.second]= buffer;
		}
		//有则返回，无则创建
		void BindDescriptorSets(CommandBuffer::SharedPtr cmdBuffer);
		//只是缓存，仅有拷贝操作
		void SetImage(std::string name, tImageView::SharedPtr image) {
			assert(BindMap.count(name) != 0);
			auto group = BindMap[name];
			images[group.first][group.second]= image;
		}
		template <typename T>
		void SetValue(std::string valueName, T value, int RangeId) {
			assert(ValueMap.count(valueName) != 0);
			auto& pa = ValueMap[valueName];
			memcpy(buffer_data[pa.set_number][pa.bind_number].data()+pa.offset,&value,sizeof(value));
		}

		void PushConstant(CommandBuffer::SharedPtr cb, tPipelineLayout::SharedPtr layout) {
			if (pushConstantBlock.size() > 0) {
				cb->pushConstants(layout, base_shader->stageFlags, 0,static_cast<uint32_t>( pushConstantBlock.size()), pushConstantBlock.data());
			}
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
		void SetShader(tShader::SharedPtr& shader);
		//Ordered descriptorSets: nullptr for there is no set
		
		//vk::PipelineLayout vkpipelineLayout;
	protected:
		vk::PipelineBindPoint vkpipelineBindPoint;
		
		struct sValueMap {
			uint32_t set_number;
			uint32_t bind_number;
			uint32_t offset;
		};
		std::unordered_map<std::string, sValueMap> ValueMap;//Value,set, binding
		std::unordered_map<std::string, std::pair<uint8_t, uint16_t>> BindMap;//Resource Name,set,bind. Tell which set and binding each image or buffer belong to
	
		//Consecutive Set,interleved binding
	//	std::vector<tDescriptorSets::SharedPtr> descriptorSets;
		std::vector<std::unordered_map<uint32_t, std::vector<char>>>  buffer_data;
		std::vector<std::unordered_map<uint32_t, tBuffer::SharedPtr>>  buffers;
		std::vector<std::unordered_map<uint32_t,	tImageView::SharedPtr>>  images;
		std::vector<char> pushConstantBlock;
	private:
		
		tShader::SharedPtr base_shader;
		std::weak_ptr<tDescriptorPool> descPool;
		weakDevice device;
		
	};

}