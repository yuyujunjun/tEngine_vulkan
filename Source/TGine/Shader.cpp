#include"Shader.h"
#include"Reflector.h"
#include"CommandBufferBase.h"
#include"Buffer.h"
#include"Image.h"
#include"ShaderInterface.h"
#include"DescriptorPool.h"
#include"Pipeline.h"
#include"FrameBuffer.h"
#include"Device.h"
#include"AssetLoadManager.h"
namespace tEngine {
	uint32_t tShader::setCount()const {
		uint32_t maxSet = 0;
		for (auto& set : setInfos) {
			maxSet = maxSet < set.set_number ? set.set_number : maxSet;
		}
		maxSet++;
		return maxSet;
	}
	std::vector<ResSetBinding> getResourceBindingInfo(const tShader* shader) {
		std::vector<ResSetBinding> result(shader->setCount());
		for (const auto& set : shader->setInfos) {
			result[set.set_number].resize(set.getBindingCount());
			for (uint32_t i = 0; i < set.data.bindings.size(); i++) {
				auto bId = set.data.bindings[i].binding;
				auto type = set.data.bindings[i].descriptorType;
				auto range = set.blockBuffers.at(bId).ByteSize();
				result[set.set_number][bId] = BindingResourceInfo(bId, type, {}, {}, {}, {}, 0, range);
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

	void tShader::AddShaderModule(std::string fileName, vk::ShaderStageFlagBits stageFlag) {
		assert(!isCreate && "Can't change shader after set it to material");
		if (isCreate)throw("Can't change shader after set it to material");
		pipelinelayout = nullptr;
		setAllocator.clear();
		auto& d = device;
		shaderAsset.push_back(LoadShader(fileName));
		Reflector::reflectionShader(shaderAsset.back()->shaderReflection, setInfos, pushConstant, stageFlag);
		allstageFlags |= stageFlag;
		//To check stage Flags
		vk::ShaderModuleCreateInfo info;
		info.setCode(shaderAsset.back()->shaderSource);
		shaderModule.push_back(d->createShaderModule(info));
		shaderStage.emplace_back(stageFlag);
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
			pipelinelayout = createPipelineLayout(d, layouts, pushConstant, allstageFlags);

		}
		isCreate = true;
	}
	//只是缓存，仅有拷贝操作
	void tShaderInterface::SetBuffer(std::string name, BufferHandle buffer, uint32_t offset) {
		auto s_b = base_shader->getBlockSetBinding(name);

		auto set_number = s_b.first;
		auto binding = s_b.second;
		bindResources[s_b.first][s_b.second].buffer = buffer;
		bindResources[s_b.first][s_b.second].offset = offset;

	}
	//fake viewInfo
	void tShaderInterface::SetImage(std::string name, ImageHandle image, vk::ImageView vkView, StockSampler sampler) {
		//	assert(bindResource_to_idx.count(name) != 0);
		auto s_b = base_shader->getBlockSetBinding(name);

		auto set_number = s_b.first;
		auto binding = s_b.second;
		bindResources[s_b.first][s_b.second].image = image;
		vkView = vkView ? vkView : image->get_view()->getDefaultView();
		bindResources[s_b.first][s_b.second].view = vkView;
		bindResources[s_b.first][s_b.second].sampler = sampler;
	}
	void tShaderInterface::SetPushConstant(std::string name, const void* attribute, size_t size) {

		bool HasSet = false;
		auto& pushConstant = base_shader->pushConstant;
		for (int i = 0; i < base_shader->pushConstant.size(); i++) {
			if (base_shader->pushConstant[i].name == name) {
				int size = size;
				int a = sizeof(char);
				assert(size == pushConstant[i].size);
				memcpy(pushConstantBlock.data() + pushConstant[i].offset, attribute, pushConstant[i].size);
				HasSet = true;
				break;
			}

		}
		assert(HasSet && "Push Constant Name may be wrong or Push Constant size may be zero");
	}
	void tShaderInterface::SetValue(std::string valueName, const void* value, size_t size, BufferHandle buffer) {
		auto info = base_shader->getVarSetBinding(valueName);
		if (buffer == nullptr) {
			assert(bindResources[info.set_number][info.bind_number].buffer != nullptr);
			buffer = bindResources[info.set_number][info.bind_number].buffer;
		}
		auto rangeOffset = bindResources[info.set_number][info.bind_number].offset;

		buffer->setRange(value, info.offset + rangeOffset, size);
	}
	std::shared_ptr<tShaderInterface> tShader::getInterface() {

		return std::make_shared<tShaderInterface>(this);

	}
	tShaderInterface::tShaderInterface(const tShader* shader) : base_shader(shader) {
		bindResources = getResourceBindingInfo(shader);
	};


	const GpuBlockBuffer& tShader::getBlock(std::string name) {
		auto s_b = blockToSetBinding.at(name);
		return setInfos[s_b.first].blockBuffers.at(s_b.second);
	}
	bool tShaderInterface::isSetEmpty(int i) {

		for (auto& b : bindResources[i]) {
			if (b.dstBinding!=-1) {
				return false;
			}
		}
		return true;
	}
	const BufferHandle& tShader::requestBuffer(std::string name, uint32_t rangeCount)const {
		return requestBufferRange(name, 1).buffer();
	}
	BufferRangeManager tShader::requestBufferRange(std::string name, uint32_t rangeCount)const {
		auto s_b = blockToSetBinding.at(name);
		auto block = setInfos[s_b.first].blockBuffers.at(s_b.second);
		return createBufferFromBlock(device, block, rangeCount);

	}

	tShader::~tShader() {
		if (shaderModule.size() > 0) {

			for (int i = 0; i < shaderModule.size(); ++i) {
				if (shaderModule[i]) {
					device->destroyShaderModule(shaderModule[i]);
					shaderModule[i] = vk::ShaderModule();
				}
			}
		}

	}

	const DescSetAllocHandle& tShaderInterface::getDescSetAllocator(uint32_t set_number)const {
		return base_shader->setAllocator[set_number];
	}

	const std::vector<uint8_t>& tShaderInterface::getPushConstantBlock()const {
		return pushConstantBlock;
	}
	const std::vector<ResSetBinding>& tShaderInterface::getResSetBinding()const {
		return bindResources;
	}
	const Device* tShaderInterface::getDevice() {
		return base_shader->device;
	}
	std::vector<ResSetBinding>& tShaderInterface::getResSetBinding()
	{
		return bindResources;
	}
	void fillWithDirtyImage(ResSetBinding& rb, const Device* device) {
		for (auto& bindingInfo : rb) {
			if (bindingInfo.emptyResource()) {
				auto& resource = bindingInfo;
				switch (resource.type) {
				case vk::DescriptorType::eCombinedImageSampler:
				case vk::DescriptorType::eStorageImage:
					resource.image = tImage::requestDummyImage(device);
					resource.view = resource.image->get_view()->getDefaultView();
					resource.sampler = tEngine::StockSampler::LinearClamp;
					break;
		
				}
			}
		}
	}

	void collectDescriptorSets(std::vector<DescriptorSetHandle>& bindedSets, std::vector<uint32_t>& offsets,
		const ResSetBinding& setBindings, const DescSetAllocHandle& setAllocator)
	{
		if (setBindings.size() == 0) {
			return;
		}
		bindedSets.emplace_back(setAllocator->requestDescriptorSet(setBindings));
		//Dynamic Offsets
		for (const auto& bindBufer : setBindings) {
			if (bindBufer.type == vk::DescriptorType::eUniformBufferDynamic || bindBufer.type == vk::DescriptorType::eStorageBufferDynamic) {
				if (bindBufer.buffer) {
					offsets.emplace_back(bindBufer.offset);
				}
				else {
					offsets.emplace_back(0);
				}

			}

		}
	}
	void flushDescriptorSet(const CommandBufferHandle& cb, tShaderInterface& state) {
		//state.uploadUniformBuffer();
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
					state.getShader()->getPipelineLayout(), firstSet, bindedSets, offsets);
				bindedSets.clear();
				offsets.clear();
			}
		};
		for (uint32_t i = 0; i < state.setCount(); ++i) {
			
			if (state.isSetEmpty(i)) {

				bindNow(i - bindedSets.size());

			}
			fillWithDirtyImage(state.getResSetBinding()[i], state.getDevice());
			collectDescriptorSets(bindedSets, offsets, state.getResSetBinding()[i], state.getDescSetAllocator(i));
		}
		bindNow(state.setCount() - bindedSets.size());
	}
	void flushGraphicsPipeline(const CommandBufferHandle& cb, tShaderInterface& state, tRenderPass* renderPass, uint32_t subpass) {
		auto createInfo = getDefaultPipelineCreateInfo(&state, renderPass, subpass, renderPass->requestFrameBuffer().get());


		cb->bindPipeline(state.getShader()->getPipelineLayout()->requestGraphicsPipeline(createInfo));
		if (state.getPushConstantBlock().size()) {
			cb->pushConstants(state.getShader()->getPipelineLayout()->getVkHandle(), (VkShaderStageFlags)state.getShader()->getAllStagesFlag(), state.getPushConstantBlock());
		}
	}
	void flushComptuePipeline(const CommandBufferHandle& cb, tShaderInterface& state) {
		ComputePipelineCreateInfo info;
		info.setShader(state.getShader()->getShaderModule(0), state.getShader()->getShaderStage(0));
		info.setLayout(state.getShader()->getPipelineLayout()->getVkHandle());
		cb->bindPipeline(state.getShader()->getPipelineLayout()->requestComputePipeline(info));
		if (state.getPushConstantBlock().size()) {
			cb->pushConstants(state.getShader()->getPipelineLayout()->getVkHandle(), (VkShaderStageFlags)state.getShader()->getAllStagesFlag(), state.getPushConstantBlock());
		}
	}
	void flushGraphicsShaderState(tShaderInterface* state, CommandBufferHandle& cb, tRenderPass* renderPass, uint32_t subpass) {
		flushGraphicsPipeline(cb, *state, renderPass, subpass);
		flushDescriptorSet(cb, *state);
	}
	void flushComputeShaderState(tShaderInterface* state, CommandBufferHandle& cb) {
		flushComptuePipeline(cb, *state);
		flushDescriptorSet(cb, *state);
	}


}