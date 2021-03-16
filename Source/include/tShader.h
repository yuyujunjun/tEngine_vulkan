#pragma once
#include"Core.h"
#include"tDescriptorPool.h"
#include"tPipeline.h"
#include<unordered_map>
#include"Reflector.h"

namespace tEngine {
	
	struct BufferRangeManager {
		BufferRangeManager() {}
		BufferRangeManager(BufferHandle handle, size_t initialOff = 0):handle(handle),initialOff(initialOff),offset(initialOff) {

		}
		size_t SetRangeIncremental(void* data, size_t size) {
			std::lock_guard<std::mutex> m(mtx);
			return SetRangeIncrementalNoLock(data, size);
		}
		size_t SetRangeIncrementalNoLock(void* data,size_t size) {

			size_t off = offset;
			if (off + size >= handle->getSize()) {
				off = initialOff;
			}
			assert(off+size<handle->getSize());
			memcpy(static_cast<uint8_t*>(handle->getAllocation()->GetMappedData())+off, data, size);
			offset += size;
			return off;
		}
		void SetBufferOffset(BufferHandle& handl,const size_t off) {
			initialOff = offset = off;
			handle = handl;
		}
		const  BufferHandle& buffer() const{
			return handle;
		}
	private:
		std::mutex mtx;
		size_t initialOff = 0;
		size_t offset = 0;
		BufferHandle handle;
	};
	struct ShaderAsset;
	class tShader {
	public:
		friend class tShaderInterface;
		using SharedPtr = std::shared_ptr<tShader>;
		static SharedPtr Create(uniqueDevice& device) {
			return std::make_shared<tShader>(device);
		}
		tShader(Device* device) :device(device) {}
		void SetShaderModule(const vk::ArrayProxy<const std::string>& fileName, vk::ShaderStageFlags stageFlag);
		
		~tShader() {
			if (shaderModule.size() > 0) {
				
				for (int i = 0; i < shaderModule.size(); ++i) {
					if (shaderModule[i]) {
						device->destroyShaderModule(shaderModule[i]);
						shaderModule[i] = vk::ShaderModule();
					}
				}
			}
			
		}
		VkPipelineBindPoint getPipelineBindPoint()const {
			if (static_cast<uint32_t>(allstageFlags & vk::ShaderStageFlagBits::eAllGraphics) != 0) {
				return VK_PIPELINE_BIND_POINT_GRAPHICS;
			}
			else {
				return VK_PIPELINE_BIND_POINT_COMPUTE;
			}
		}
		tShaderInterface getInterface();
		
		std::vector<vk::ShaderModule> shaderModule;
		std::vector<vk::ShaderStageFlagBits> shaderStage;
		vk::ShaderStageFlags allstageFlags;

		std::vector<tDescSetsDataWithSetNumber> setInfos;//setInfos coresspond to setAllocator
		std::vector<DescSetAllocHandle> setAllocator;

		tPipelineLayout::SharedPtr pipelinelayout;
		GpuBlockBuffer pushConstant;
	private:
		void CreateShaderLayout();
		void CreateUniformBuffers();
		void AddShaderModule(std::string fileName, vk::ShaderStageFlags stageFlag);
		bool isCreate = false;
		std::vector<std::shared_ptr<ShaderAsset>> shaderAsset;
		std::vector<std::unordered_map<uint32_t, BufferRangeManager>> uniformBuffers;
//		std::shared_ptr<tShaderInterface> interface;
		Device* device;
	};
	class tShaderInterface {
	public:
		struct PipelineBinding {
			ResSetBinding resSetBinding;
			std::unordered_map<uint32_t, uint32_t> offsets;
			std::unordered_map<uint32_t, std::vector<char>>buffer_data;
			bool isEmpty() {
				return resSetBinding.getBuffers().size() == 0 && resSetBinding.getImages().size() == 0;
			}
		};
		friend class CommandBufferBase_;
		tShaderInterface(const tShader* shader);

		

		//只是缓存，仅有拷贝操作
		void SetBuffer(std::string name, BufferHandle buffer) {
			assert(BindMap.count(name) != 0);
			auto group = BindMap[name];
			BindingInfo[group.first].resSetBinding.SetBuffer(group.second, buffer);
			BindingInfo[group.first].offsets[group.second] = 0;
		}
		//Copy value to buffer just before binding descriptorset
		void uploadUniformBuffer() {
			for (int set_number = 0; set_number < setCount(); ++set_number) {
				auto& buffers = const_cast<tShader*>(base_shader)->uniformBuffers[set_number];
				auto& buffer_data = BindingInfo[set_number].buffer_data;
				auto& offsets = BindingInfo[set_number].offsets;
				for (const auto& buf : buffers) {
					auto& data = buffer_data[buf.first];
					offsets[buf.first]= buffers.at(buf.first).SetRangeIncremental(data.data(),data.size());
					buf.second.buffer()->Flush();
				
				}
			}
		}
		//只是缓存，仅有拷贝操作

		//fake viewInfo
		void SetImage(std::string name, ImageviewHandle image, StockSampler sampler=StockSampler::LinearClamp, uint32_t viewInfo=0) {
			assert(BindMap.count(name) != 0);
			auto group = BindMap[name];
			BindingInfo[group.first].resSetBinding.SetImage(group.second, image,image->get_unorm_view(),sampler);
		}
		template <typename Attribute>
		void SetValue(std::string valueName, Attribute value, int RangeId) {
			assert(ValueMap.count(valueName) != 0);
			auto& pa = ValueMap[valueName];
			memcpy(BindingInfo[pa.set_number].buffer_data[pa.bind_number].data() + pa.offset, &value, sizeof(value));
		}

		
		template<typename Attribute>
		void SetPushConstant(std::string name, Attribute value) {

			bool HasSet = false;
			auto& pushConstant = base_shader->pushConstant;
			for (int i = 0; i < base_shader->pushConstant.size(); i++) {
				if (base_shader->pushConstant[i].name == name) {
					int size = sizeof(Attribute);
					int a = sizeof(char);
					assert(size == pushConstant[i].size);
					memcpy(pushConstantBlock.data() + pushConstant[i].offset, &value, pushConstant[i].size);
					HasSet = true;
					break;
				}

			}
			assert(HasSet && "Push Constant Name may be wrong or Push Constant size may be zero");
		}
		
		const tShader* getShader() const {
			return base_shader;
		}
		
		const std::vector<PipelineBinding>& getBindings()const {
			return BindingInfo;
		}
		size_t setCount()const {
			return BindingInfo.size();
		}
		const tShader* getShader() {
			return base_shader;
		}
		const DescSetAllocHandle& getDescSetAllocator(uint32_t set_number)const {
			return base_shader->setAllocator[set_number];
		}
		bool set_isEmpty(size_t idx) {
			return BindingInfo[idx].isEmpty();
		}
		const std::vector<uint8_t>& getPushConstantBlock()const {
			return pushConstantBlock;
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
	
		std::vector<PipelineBinding> BindingInfo;
		std::vector<uint8_t> pushConstantBlock;
	private:

		 const tShader* base_shader;
	

	};
	
	void collectDescriptorSets(std::vector<vk::DescriptorSet>& bindedSets, std::vector<uint32_t>& offsets,
		const tShaderInterface::PipelineBinding& setBindings, const DescSetAllocHandle& setAllocator) 
	{
		if (0 == setBindings.resSetBinding.getBuffers().size() || 0 == setBindings.resSetBinding.getImages().size()) {
			return;
		}
		bindedSets.emplace_back(setAllocator->requestDescriptorSet(setBindings.resSetBinding));
		for (const auto& bindBufer : setBindings.resSetBinding.getBuffers()) {
			offsets.emplace_back(setBindings.offsets.at(bindBufer.binding));
		}
	}
	void flushDescriptorSet(const CommandBufferHandle& cb,tShaderInterface& state) {
		state.uploadUniformBuffer();
		std::vector<vk::DescriptorSet> bindedSets;
		std::vector<uint32_t> offsets;
		uint32_t firstSet = 0;
		auto bindNow = [&bindedSets,&cb,&offsets,&firstSet,&state]() {
			if (bindedSets.size() != 0) {
				cb->bindDescriptorSet(static_cast<vk::PipelineBindPoint>(state.getShader()->getPipelineBindPoint()),
					state.getShader()->pipelinelayout->getVkHandle(), firstSet, bindedSets, offsets);
				bindedSets.clear();
				offsets.clear();
			}
		};
		for (uint32_t i = 0; i < state.setCount(); ++i,++firstSet) {
			if (state.set_isEmpty(i)) {
				bindNow();
			}
			collectDescriptorSets(bindedSets,offsets,state.getBindings()[i],state.getDescSetAllocator(i));
		}
		bindNow();
	}
	void flushGraphicsPipeline(const CommandBufferHandle& cb, tShaderInterface& state,tRenderPass* renderPass,uint32_t subpass) {
		auto createInfo=getDefaultPipelineCreateInfo(&state, renderPass, subpass, renderPass->requestFrameBuffer());
		cb->bindPipeline(*state.getShader()->pipelinelayout->requestGraphicsPipeline(createInfo));
		cb->pushConstants(state.getShader()->pipelinelayout->getVkHandle(),(VkShaderStageFlags)state.getShader()->allstageFlags,state.getPushConstantBlock());
	}
	void flushComptuePipeline(const CommandBufferHandle& cb, tShaderInterface& state) {
		ComputePipelineCreateInfo info;
		info.setShader(state.getShader()->shaderModule[0], state.getShader()->shaderStage[0]);
		info.setLayout(state.getShader()->pipelinelayout->getVkHandle());
		cb->bindPipeline(*state.getShader()->pipelinelayout->requestComputePipeline(info));
		cb->pushConstants(state.getShader()->pipelinelayout->getVkHandle(), (VkShaderStageFlags)state.getShader()->allstageFlags, state.getPushConstantBlock());

	}
	void flushGraphicsShaderState(tShaderInterface& state,CommandBufferHandle& cb,tRenderPass* renderPass, uint32_t subpass) {
		flushGraphicsPipeline(cb, state, renderPass, subpass);
		flushDescriptorSet(cb,state);
	}
	void flushComputeShaderState(tShaderInterface& state, CommandBufferHandle& cb) {
		flushComptuePipeline(cb, state);
		flushDescriptorSet(cb, state);
	}
}