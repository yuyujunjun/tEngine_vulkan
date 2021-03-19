#pragma once
#include"vulkan/vulkan.h"
#include<vulkan/vulkan.hpp>
#include<unordered_map>
#include"tDescriptorShared.h"
#include"tGpuBlock.h"

namespace tEngine {
	class Device;
	class BindingResourceInfo;
	using ResSetBinding = std::vector<BindingResourceInfo>;
	struct ShaderAsset;
	class tShaderInterface;
	class BufferRangeManager;
	class tPipelineLayout;
	using PipelineLayoutHandle = std::shared_ptr<tPipelineLayout>;
	class tDescriptorSetAllocator;
	using DescSetAllocHandle = std::shared_ptr<tDescriptorSetAllocator>;
	class tBuffer;
	using BufferHandle = std::shared_ptr<tBuffer>;
	class tImage;
	using ImageHandle = std::shared_ptr<tImage>;// ::SharedPtr;
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
		tShader( Device* device) :device(device) {}
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
		uint32_t setCount()const;
		const PipelineLayoutHandle& getPipelineLayout()const {
			return pipelinelayout;
		}
		const vk::ShaderStageFlags  getAllStagesFlag()const {
			return allstageFlags;
		}
		const GpuBlockBuffer& getBlock(std::string name);
		BufferRangeManager requestBufferRange(std::string name, uint32_t rangeCount)const;
		const BufferHandle& requestBuffer(std::string name, uint32_t rangeCount = 1)const;
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
	

}