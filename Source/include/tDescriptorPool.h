#pragma once
//#include<mutex>
#include"vulkan/vulkan.hpp"
#include"Core.h"
#include"utils.hpp"
#include<unordered_map>
#include"tResource.h"

#include<mutex>
namespace tEngine {
	//Not ordered by binding
	struct tDescLayoutData {
		const vk::DescriptorSetLayoutBinding& bindingAt(uint32_t idx)const {
			auto& result= std::find_if(bindings.begin(), bindings.end(),
				[&idx](const vk::DescriptorSetLayoutBinding & b) {
					return  idx == b.binding;
				 });
			return *result;
		}
		vk::DescriptorSetLayoutBinding& bindingAt(uint32_t idx) {
			auto& result = std::find_if(bindings.begin(), bindings.end(),
				[&idx](const vk::DescriptorSetLayoutBinding& b) {
					return  idx == b.binding;
				});
			return *result;
		}
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
	private:
		
		
	};
	struct tDescSetsDataWithSetNumber {
		uint32_t set_number;
		tDescLayoutData data;
		std::unordered_map<uint32_t, GpuBlockBuffer> blockBuffers;
	};

	void MergeSet(std::vector<tDescSetsDataWithSetNumber>& setLayoutBinding);
	class ResSetBinding;

	class tDescriptorSetLayout {
	public:
		DECLARE_SHARED(tDescriptorSetLayout);
		tDescLayoutData bindings;
		vk::DescriptorSetLayout vkLayout;
		~tDescriptorSetLayout() {
			if (vkLayout) {
				device->destroyDescriptorSetLayout(vkLayout);
				vkLayout = vk::DescriptorSetLayout();
			}
		}
		tDescriptorSetLayout(Device* device, const tDescLayoutData& bindings) :device(device), bindings(bindings) {
			vk::DescriptorSetLayoutCreateInfo info;
			info.setBindings(bindings.bindings);
			vkLayout = device->createDescriptorSetLayout(info);
		}
	private:
		friend class tDescriptorSetLayoutManager;
		static SharedPtr Create(Device*& device,const tDescLayoutData& bindings) {
			return std::make_shared<tDescriptorSetLayout>(device, bindings);
		}
		
	
		
	private:
		
		weakDevice device;
	};
	//To ensure that the same bindings of a set produce the same layout
	class tDescriptorSetLayoutManager {
	public:
		static tDescriptorSetLayoutManager manager;
		tDescriptorSetLayoutManager() {}
	
		tDescriptorSetLayout::SharedPtr createSetLayout(Device*& device,const tDescLayoutData& bindings) {
			//find first
			for (size_t i = 0; i < bindingInfoList.size(); ++i) {
				if (bindings.bindings == bindingInfoList[i].bindings ) {
					if (!layoutList[i].expired()) {
						return layoutList[i].lock();
					}
				}
			}
			std::lock_guard<std::mutex> lock(mtx);

			for (size_t i = 0; i < bindingInfoList.size(); ++i) {
				if (bindings.bindings == bindingInfoList[i].bindings) {
					if (!layoutList[i].expired()) {
						return layoutList[i].lock();
					}
					else {

						auto descSetLayout = tDescriptorSetLayout::Create(device,bindings);
						layoutList[i] = descSetLayout;
						return descSetLayout;
					}
				}
			}
			auto descSetLayout = tDescriptorSetLayout::Create(device, bindings);
			layoutList.push_back(descSetLayout);
			bindingInfoList.push_back(bindings);
			return descSetLayout;
		}
		
	private:
		std::mutex mtx;
		//using BindingInfo = std::vector<vk::DescriptorSetLayoutBinding>;

		std::vector<tDescLayoutData> bindingInfoList;
		std::vector< std::weak_ptr<tDescriptorSetLayout>> layoutList;
	};
	struct BufferBindInfo {
		
		uint32_t binding;
		BufferHandle buffer;
		bool operator==(const BufferBindInfo& a) {
			return a.binding == binding && a.buffer->getVkHandle() == buffer->getVkHandle();
		}
		bool operator!=(const BufferBindInfo& a) {
			return !(*this == a);
		}
	};
	struct DynamicBindInfo :public BufferBindInfo {
		uint32_t offset;
		operator BufferBindInfo() {
			return BufferBindInfo({ binding,buffer });
		}
		bool operator==(const DynamicBindInfo& a) {
			return a.binding == binding && a.buffer->getVkHandle() == buffer->getVkHandle() &&a.offset==offset;
		}
		bool operator!=(const DynamicBindInfo& a) {
			return !(*this == a);
		}
	};
	
	struct ImageBindInfo {
		uint32_t binding;
		ImageviewHandle image;
		bool operator==(const ImageBindInfo& a) {
			return a.binding == binding && a.image->getVkHandle() == image->getVkHandle();
		}
		bool operator!=(const ImageBindInfo& a) {
			return !(*this == a);
		}
	};
	class ResSetBinding {
	public:
		friend class tDescriptorPool;
	
		ResSetBinding()  {
		}
		void SetBuffer(uint32_t binding,BufferHandle buffer) {
			auto& bindBuffer = getBuffer(binding);
			if (bindBuffer->getVkHandle() == buffer->getVkHandle())return;
			bindBuffer = buffer;
		
		}
		void SetImage(uint32_t binding, ImageviewHandle image) {

			auto& bindImage = getImage(binding);
			if (bindImage->getVkHandle() == image->getVkHandle())return;
			bindImage = image;

		}
		bool operator==(const ResSetBinding& b) {
			return b.bindBuffer == bindBuffer&&bindImage==b.bindImage;
		}
		bool operator!=(const ResSetBinding& b) {
			return !(*this == b);
		}
		bool hasBuffer(uint32_t binding) {
			return bindBuffer.count(binding) != 0;
		}
		bool hasImage(uint32_t binding) {
			return bindImage.count(binding) != 0;
		}
		 BufferHandle& getBuffer(uint32_t binding) {
			return bindBuffer.at(binding).buffer;
			
		}
		 ImageviewHandle& getImage(uint32_t binding) {
			return bindImage.at(binding).image;
		}
	
		const std::unordered_map<uint32_t, BufferBindInfo>& getBuffers()const {
			return bindBuffer;

		}
		const std::unordered_map<uint32_t, ImageBindInfo>& getImages()const {
			return bindImage;
		}
		 std::unordered_map<uint32_t, BufferBindInfo>& getBuffers() {
			return bindBuffer;

		}
		 std::unordered_map<uint32_t, ImageBindInfo>& getImages() {
			return bindImage;
		}
	private:
		std::unordered_map<uint32_t,BufferBindInfo> bindBuffer;
		std::unordered_map<uint32_t,ImageBindInfo> bindImage;

	};
	
	class tShader;
	
	
	class tDescriptorSetAllocator {
		tDescriptorSetAllocator(Device* device, tDescriptorSetLayout* layout):device(device), layout(layout){
			AllocatePool();
		}
		void AllocatePool();
		uint32_t request_DescriptorSet(const ResSetBinding& rb) {
			auto iter=std::find(resSetBinding.begin(), resSetBinding.end(), rb);
			if (iter != resSetBinding.end()) {
				return iter - resSetBinding.begin();
			}
			uint32_t idx= nextIdx();

			std::vector<vk::WriteDescriptorSet> write;
			auto& vkDescSet = allocatedDescSets[idx];
			std::vector< vk::DescriptorBufferInfo> bufferInfo;
			std::vector<vk::DescriptorImageInfo> imageInfo;
			auto& m_binding = resSetBinding[idx];
			for (auto& b : rb.getBuffers()) {
				auto& binding = b.first;
				if (!m_binding.hasBuffer(binding)|| resSetBinding[idx].getBuffers().at(binding) != b.second) {
					m_binding.getBuffers()[binding] = b.second;
					auto& buffer = b.second.buffer;
					bufferInfo.emplace_back(buffer->getVkHandle(), 0, buffer->getSize());
					write.emplace_back(vk::WriteDescriptorSet(vkDescSet, binding, 0, layout->bindings.bindingAt(binding).descriptorType, nullptr, bufferInfo.back(), {}));
				}
			}

			StockSampler sampler;
			sampler = StockSampler::LinearClamp;
			for (auto& b : rb.getImages()) {
				auto& binding = b.first;
				if (!m_binding.hasImage(binding)|| resSetBinding[idx].getImages().at(binding) != b.second) {
					m_binding.getImages()[binding] = b.second;
					auto type = layout->bindings.bindingAt(binding).descriptorType;
					switch (type) {
					case vk::DescriptorType::eCombinedImageSampler:
						imageInfo.emplace_back(vk::DescriptorImageInfo(device->getSampler(sampler)->getVkHandle(), b.second.image->getVkHandle(), vk::ImageLayout::eShaderReadOnlyOptimal));
						break;
					case vk::DescriptorType::eSampledImage:
						imageInfo.emplace_back(vk::DescriptorImageInfo({}, b.second.image->getVkHandle(), vk::ImageLayout::eShaderReadOnlyOptimal));
						break;
					case vk::DescriptorType::eSampler:
						imageInfo.emplace_back(vk::DescriptorImageInfo(device->getSampler(sampler)->getVkHandle(), {}, vk::ImageLayout::eUndefined));
						break;
					default:
						throw("Wrong image descriptor Type");
					}
					write.emplace_back(vk::WriteDescriptorSet(vkDescSet, binding, 0, type, imageInfo.back(), {}, {}));
				}
			}
			
		}
		~tDescriptorSetAllocator() {
			if (pool) {
				device->destroyDescriptorPool(pool);
			}
		}
		uint32_t nextIdx() {
			if (currentUse < allocatedDescSets.size())return currentUse++;
			allocate_DescriptorSet();
			if (currentUse < allocatedDescSets.size())return currentUse++;
			return 0;
		}
		
	private:
		void allocate_DescriptorSet();
		uint32_t currentUse = 0;
		const uint32_t minRingSize = 4;
		const uint32_t RingSize = 8;
		Device* device;
		VkDescriptorPool pool;//We assume that pool is big enought 
		std::vector<vk::DescriptorSet> allocatedDescSets;
		std::vector<ResSetBinding> resSetBinding;
		const tDescriptorSetLayout* layout;
	};
	
}