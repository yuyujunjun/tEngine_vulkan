#pragma once
#include"tGine.h"
#include"tDescriptorPool.h"
#include<unordered_map>
#include"tGpuBlock.h"
#include"tResource.h"
namespace tEngine {

	struct ShaderAsset;
	class tShaderInterface;
	class tShader {
	public:
		/// <summary>
	/// Name map to set, binding
	/// </summary>
		struct sValueMap {
			uint32_t set_number;
			uint32_t bind_number;
			uint32_t offset;
		};
		friend class tShaderInterface;
		using SharedPtr = std::shared_ptr<tShader>;
		static SharedPtr Create(Device* device) {
			return std::make_shared<tShader>(device);
		}
		tShader(Device* device) :device(device) {}
		void SetShaderModule(const vk::ArrayProxy<const std::string>& fileName, const vk::ArrayProxy<const vk::ShaderStageFlagBits>& stageFlag);
		const vk::ShaderModule& getShaderModule(int i) const {
			return shaderModule[i];
		}
		const uint32_t getShaderCount()const { return shaderModule.size(); }
		const vk::ShaderStageFlagBits& getShaderStage(int i) const {
			return shaderStage[i];
		}
		~tShader();
		VkPipelineBindPoint getPipelineBindPoint()const {
			if (static_cast<uint32_t>(allstageFlags & vk::ShaderStageFlagBits::eAllGraphics) != 0) {
				return VK_PIPELINE_BIND_POINT_GRAPHICS;
			}
			else {
				return VK_PIPELINE_BIND_POINT_COMPUTE;
			}
		}
		std::shared_ptr<tShaderInterface> getInterface();
		sValueMap getVarSetBinding(std::string name)const {
			return valueToSetBinding.at(name);
		}
		std::pair<uint8_t, uint16_t> getBlockSetBinding(std::string name)const
		{
			return blockToSetBinding.at(name);
		}
		//including empty set
		uint32_t setCount()const {
			uint32_t maxSet = 0;
			for (auto& set : setInfos) {
				maxSet = maxSet < set.set_number ? set.set_number : maxSet;
			}
			maxSet++;
			return maxSet;
		}
		const PipelineLayoutHandle& getPipelineLayout()const {
			return pipelinelayout;
		}
		const vk::ShaderStageFlags  getAllStagesFlag()const {
			return allstageFlags;
		}
		const GpuBlockBuffer& getBlock(std::string name) {
			auto s_b = blockToSetBinding.at(name);
			return setInfos[s_b.first].blockBuffers.at(s_b.second);
		}
		BufferRangeManager requestBufferRange(std::string name, uint32_t rangeCount)const {
			auto s_b = blockToSetBinding.at(name);
			auto block = setInfos[s_b.first].blockBuffers.at(s_b.second);
			return createBufferFromBlock(device, block, rangeCount);

		}
		const BufferHandle& requestBuffer(std::string name, uint32_t rangeCount = 1)const {
			return requestBufferRange(name, 1).buffer();
		}
		std::vector<tDescSetsDataWithSetNumber> setInfos;
	private:

		std::unordered_map<std::string, sValueMap> valueToSetBinding;//Value,set, binding
		std::unordered_map<std::string, std::pair<uint8_t, uint16_t>> blockToSetBinding;//Resource Name,set,bind. Tell which set and binding each image or buffer belong to

		void BuildNameBindingInfo();
		/// <summary>
		/// Shader layout information
		/// </summary>
		GpuBlockBuffer pushConstant;
		;//setInfos coresspond to setAllocator
		std::vector<DescSetAllocHandle> setAllocator;
		PipelineLayoutHandle pipelinelayout;
		void CreateShaderLayout();

		//	void CreateUniformBuffers();
		void AddShaderModule(std::string fileName, vk::ShaderStageFlagBits stageFlag);
		bool isCreate = false;
		std::vector<std::shared_ptr<ShaderAsset>> shaderAsset;


		std::vector<vk::ShaderModule> shaderModule;
		std::vector<vk::ShaderStageFlagBits> shaderStage;
		vk::ShaderStageFlags allstageFlags;
		Device* device;
	};
	std::vector<ResSetBinding> getResourceBindingInfo(tShader* shader);
	//给shader里的buffer赋值
	template <typename Attribute>
	void SetValue(std::string valueName, const Attribute& value, const tShader* shader, BufferHandle& buffer, size_t rangeOffset = 0) {
		auto info = shader->getVarSetBinding(valueName);
		buffer->setRange(&value, info.offset + rangeOffset, sizeof(value));

	}
	class tShaderInterface {
	public:

		friend class CommandBuffer;
		tShaderInterface(const tShader* shader);



		//只是缓存，仅有拷贝操作
		void SetBuffer(std::string name, BufferHandle buffer, uint32_t offset = 0) {
			auto s_b = base_shader->getBlockSetBinding(name);

			auto set_number = s_b.first;
			auto binding = s_b.second;
			bindResources[s_b.first][s_b.second].buffer = buffer;
			bindResources[s_b.first][s_b.second].offset = offset;

		}
		//fake viewInfo
		void SetImage(std::string name, ImageHandle image, vk::ImageView vkView = {}, StockSampler sampler = StockSampler::LinearClamp) {
			//	assert(bindResource_to_idx.count(name) != 0);
			auto s_b = base_shader->getBlockSetBinding(name);

			auto set_number = s_b.first;
			auto binding = s_b.second;
			bindResources[s_b.first][s_b.second].image = image;
			vkView = vkView ? vkView : image->get_view()->getDefaultView();
			bindResources[s_b.first][s_b.second].view = vkView;
			bindResources[s_b.first][s_b.second].sampler = sampler;
		}
		template <typename Attribute>
		//using  Attribute=float;
		void SetValue(std::string valueName, const Attribute& value, BufferHandle buffer = nullptr) {
			auto info = base_shader->getVarSetBinding(valueName);
			if (buffer == nullptr) {
				assert(bindResources[info.set_number][info.bind_number].buffer != nullptr);
				buffer = bindResources[info.set_number][info.bind_number].buffer;
			}
			auto rangeOffset = bindResources[info.set_number][info.bind_number].offset;
			buffer->setRange(&value, info.offset + rangeOffset, sizeof(value));

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


		size_t setCount()const {
			return bindResources.size();
		}
		const tShader* getShader() {
			return base_shader;
		}
		const DescSetAllocHandle& getDescSetAllocator(uint32_t set_number)const {
			return base_shader->setAllocator[set_number];
		}

		const std::vector<uint8_t>& getPushConstantBlock()const {
			return pushConstantBlock;
		}
		const std::vector<ResSetBinding>& getResSetBinding()const {
			return bindResources;
		}
		bool isSetEmpty(int i) {

			for (auto& b : bindResources[i]) {
				if (!b.emptyResource()) {
					return false;
				}
			}
			return true;
		}

	protected:

		std::vector<ResSetBinding> bindResources;
		//std::vector<PipelineBinding> bindResources;
		std::vector<uint8_t> pushConstantBlock;
	private:

		const tShader* base_shader;


	};




}