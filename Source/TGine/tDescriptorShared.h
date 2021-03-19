#pragma once
#include"vulkan/vulkan.h"
#include<vulkan/vulkan.hpp>
#include<unordered_map>
#include"tSampler.h"
#include"tGpuBlock.h"
namespace tEngine {
	class tBuffer;
	using BufferHandle = std::shared_ptr<tBuffer>;
	class tImage;
	using ImageHandle = std::shared_ptr<tImage>;// ::SharedPtr;
	class tSampler;
	using SamplerHandle = std::shared_ptr<tSampler>;
	class tImageView;
	using ImageviewHandle = std::shared_ptr<tImageView>;

	class DescriptorLayoutCreateInfo {
	public:

		const vk::DescriptorSetLayoutBinding& bindingAt(uint32_t idx)const {
			auto& result = std::find_if(bindings.begin(), bindings.end(),
				[&idx](const vk::DescriptorSetLayoutBinding& b) {
					return  idx == b.binding;
				});
			return bindings[result - bindings.begin()];
		}
		vk::DescriptorSetLayoutBinding& bindingAt(uint32_t idx) {
			auto& result = std::find_if(bindings.begin(), bindings.end(),
				[&idx](const vk::DescriptorSetLayoutBinding& b) {
					return  idx == b.binding;
				});
			return bindings[result - bindings.begin()];
		}
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
	private:


	};
	struct tDescSetsDataWithSetNumber {
		uint32_t set_number;
		//include empty binding point
		const uint32_t getBindingCount()const {
			uint32_t binding = 0;
			for (auto& b : data.bindings) {
				binding = b.binding > binding ? b.binding : binding;
			}
			binding++;
			return binding;
		}
		DescriptorLayoutCreateInfo data;
		std::unordered_map<uint32_t, GpuBlockBuffer> blockBuffers;
	};
	struct BindingResourceInfo {
		BindingResourceInfo() :dstBinding(-1) {}
		BindingResourceInfo(uint32_t dstBinding, vk::DescriptorType type, ImageHandle image,
			vk::ImageView view, StockSampler sampler, BufferHandle buffer, uint32_t offset, uint32_t range) :dstBinding(dstBinding), type(type)
			, image(image), view(view), sampler(sampler), buffer(buffer), offset(offset), range(range) {}
		void updateBufferHandle(BufferHandle handle, uint32_t offset, uint32_t range) { buffer = handle; this->offset = offset; this->range = range; }
		void updateImageView(ImageHandle image, vk::ImageView view, StockSampler sampler) { this->image = image; this->view = view; this->sampler = sampler; }
		bool emptyResource() const { return buffer == nullptr && image == nullptr; }
		uint32_t dstBinding = 0;
		uint32_t dstArrayElement = 0;
		vk::DescriptorType type;
		ImageHandle image = nullptr;
		vk::ImageView view;
		StockSampler sampler = StockSampler::LinearClamp;
		BufferHandle buffer = nullptr;
		uint32_t offset = 0;
		uint32_t range = 0;
		//Don't compare offset
		bool operator==(const BindingResourceInfo& info)const;
		bool operator!=(const BindingResourceInfo& info)const {
			return !(*this == info);
		}
	};

	using ResSetBinding = std::vector<BindingResourceInfo>;
	void MergeSet(std::vector<tDescSetsDataWithSetNumber>& setLayoutBinding);
}