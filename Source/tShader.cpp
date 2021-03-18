#include"tShader.h"
#include"Reflector.h"
#include"tAssetLoadManager.h"
#include"CommandBufferBase.h"
namespace tEngine {
	void tShader::SetShaderModule(const vk::ArrayProxy<const std::string>& fileName, const vk::ArrayProxy<const vk::ShaderStageFlagBits>& stageFlag) {
		for (uint32_t i = 0; i < fileName.size(); ++i) {
			AddShaderModule(reinterpret_cast<const std::string*>(fileName.data())[i], reinterpret_cast<const vk::ShaderStageFlagBits*>(stageFlag.data())[i]);

		}
		MergeSet(setInfos);
		CreateShaderLayout();
		CreateUniformBuffers();
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
					bcif.size = set.blockBuffers[bindType.binding].ByteSize()*10;
					uniformBuffers[set_number].emplace(bindType.binding, std::make_shared<BufferRangeManager>());

				//	uniformBuffers[set_number].at(bindType.binding).SetBufferOffset(device->createBuffer(bcif), 0);
					uniformBuffers[set_number].at(bindType.binding)->SetBufferOffset( device->createBuffer(bcif),0);
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
	void tShader::AddShaderModule(std::string fileName, vk::ShaderStageFlagBits stageFlag) {
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
		shaderStage.emplace_back(stageFlag);
	}
	std::shared_ptr<tShaderInterface> tShader::getInterface() {

		return std::make_shared<tShaderInterface>(this);

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
			uint32_t maxBinding=0;
			for (auto& b : set.data.bindings) {
				maxBinding = b.binding > maxBinding ? b.binding : maxBinding;
			}
			maxBinding++;
			resSetBinding.SetBindingCount(maxBinding);
			for (auto& bindType : set.data.bindings) {
				switch (bindType.descriptorType) {
				case vk::DescriptorType::eUniformBufferDynamic:
					BindingInfo[set_number].offsets[bindType.binding] = 0;
					resSetBinding.SetBuffer(bindType.binding, shader->uniformBuffers[set_number].at(bindType.binding)->buffer()) ;
					break;
				case vk::DescriptorType::eSampledImage:
					break;
				}
				
			}
		}
	};


	size_t BufferRangeManager::SetRangeIncremental(void* data, size_t size) {
		std::lock_guard<std::mutex> m(mtx);
		return SetRangeIncrementalNoLock(data, size);
	}
	//return current offset
	size_t BufferRangeManager::SetRangeIncrementalNoLock(void* data, size_t size) {

		size_t off = offset;
		if (off + size >= handle->getSize()) {
			off = initialOff;
		}
		assert(off + size < handle->getSize());
		handle->setRange(data, off, size);
		//	memcpy(static_cast<uint8_t*>(handle->getAllocation()->GetMappedData())+off, data, size);
		offset = off + size;
		return off;
	}
	void BufferRangeManager::SetBufferOffset(BufferHandle& handl, const size_t off) {
		initialOff = offset = off;
		handle = handl;
	}

	void tShaderInterface:: uploadUniformBuffer() {
		for (int set_number = 0; set_number < setCount(); ++set_number) {
			auto& buffers = const_cast<tShader*>(base_shader)->uniformBuffers[set_number];
			auto& buffer_data = BindingInfo[set_number].buffer_data;
			auto& offsets = BindingInfo[set_number].offsets;
			for (const auto& buf : buffers) {
				auto& data = buffer_data[buf.first];
				if (buf.second->buffer()) {
					offsets[buf.first] = buf.second->SetRangeIncremental(data.data(), data.size());
					buf.second->buffer()->Flush();
				}

			}
		}
	}




















	void collectDescriptorSets(std::vector<DescriptorSetHandle>& bindedSets, std::vector<uint32_t>& offsets,
		const tShaderInterface::PipelineBinding& setBindings, const DescSetAllocHandle& setAllocator)
	{
		if (setBindings.resSetBinding.isEmpty()) {
			return;
		}
		bindedSets.emplace_back(setAllocator->requestDescriptorSet(setBindings.resSetBinding));
		for (const auto& bindBufer : setBindings.resSetBinding.getBuffers()) {
			if (bindBufer.hasUsed) {
				offsets.emplace_back(setBindings.offsets.at(bindBufer.binding));
			}
		}
	}
	void flushDescriptorSet(const CommandBufferHandle& cb, tShaderInterface& state) {
		state.uploadUniformBuffer();
		std::vector<DescriptorSetHandle> bindedSets;
		std::vector<uint32_t> offsets;
		
				/// <summary>
				/// BindDescriptorSet
				/// </summary>
				/// <param name="cb"></param>
				/// <param name="state"></param>
				auto bindNow = [&bindedSets, &cb, &offsets, &state](uint32_t firstSet) {
					if (bindedSets.size() != 0) {
						cb->bindDescriptorSet(static_cast<vk::PipelineBindPoint>(state.getShader()->getPipelineBindPoint()),
							state.getShader()->pipelinelayout, firstSet, bindedSets, offsets);
						bindedSets.clear();
						offsets.clear();
					}
				};
		for (uint32_t i = 0; i < state.setCount(); ++i) {
			if (state.set_isEmpty(i)) {
				
				bindNow(i-bindedSets.size());

			}
			collectDescriptorSets(bindedSets, offsets, state.getBindings()[i], state.getDescSetAllocator(i));
		}
		bindNow(state.setCount()-bindedSets.size());
	}
	void flushGraphicsPipeline(const CommandBufferHandle& cb, tShaderInterface& state, tRenderPass* renderPass, uint32_t subpass) {
		auto createInfo = getDefaultPipelineCreateInfo(&state, renderPass, subpass, renderPass->requestFrameBuffer().get());
		

		cb->bindPipeline(state.getShader()->pipelinelayout->requestGraphicsPipeline(createInfo));
		if (state.getPushConstantBlock().size()) {
			cb->pushConstants(state.getShader()->pipelinelayout->getVkHandle(), (VkShaderStageFlags)state.getShader()->allstageFlags, state.getPushConstantBlock());
		}
	}
	void flushComptuePipeline(const CommandBufferHandle& cb, tShaderInterface& state) {
		ComputePipelineCreateInfo info;
		info.setShader(state.getShader()->shaderModule[0], state.getShader()->shaderStage[0]);
		info.setLayout(state.getShader()->pipelinelayout->getVkHandle());
		cb->bindPipeline(state.getShader()->pipelinelayout->requestComputePipeline(info));
		if (state.getPushConstantBlock().size()) {
			cb->pushConstants(state.getShader()->pipelinelayout->getVkHandle(), (VkShaderStageFlags)state.getShader()->allstageFlags, state.getPushConstantBlock());
		}
	}
	void flushGraphicsShaderState(tShaderInterface& state, CommandBufferHandle& cb, tRenderPass* renderPass, uint32_t subpass) {
		flushGraphicsPipeline(cb, state, renderPass, subpass);
		flushDescriptorSet(cb, state);
	}
	void flushComputeShaderState(tShaderInterface& state, CommandBufferHandle& cb) {
		flushComptuePipeline(cb, state);
		flushDescriptorSet(cb, state);
	}

}