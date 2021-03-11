#pragma once
//#include<mutex>
#include"vulkan/vulkan.hpp"
#include"Core.h"
#include"utils.hpp"
#include<unordered_map>
#include"tResource.h"
namespace tEngine {
	struct tDescSetsData {
		uint32_t set_number;
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
		std::unordered_map<uint32_t, GpuBlockBuffer> blockBuffers;
	};
	void MergeSet(std::vector<tDescSetsData>& setLayoutBinding);
	class tDescriptorSets;

	class tDescriptorSetLayout {
	public:
		DECLARE_SHARED(tDescriptorSetLayout);
		static SharedPtr Create(sharedDevice& device,const tDescSetsData& bindings) {
			return std::make_shared<tDescriptorSetLayout>(device, bindings);
		}
		tDescriptorSetLayout(sharedDevice& device, const tDescSetsData& bindings) :device(device),bindings(bindings) {
			vk::DescriptorSetLayoutCreateInfo info;
			info.setBindings(bindings.bindings);
			vkLayout = device->createDescriptorSetLayout(info);
		}
		~tDescriptorSetLayout() {

		}
		tDescSetsData bindings;
		vk::DescriptorSetLayout vkLayout;
	private:
		weakDevice device;
	};
	class tDescriptorSets {
	public:
		friend class tDescriptorPool;
		using SharedPtr = std::shared_ptr<tDescriptorSets>;
		static SharedPtr Create(sharedDevice& device, vk::DescriptorSet vkDescSet,
			tDescriptorSetLayout::SharedPtr Setlayout) {
			return std::make_shared<tDescriptorSets>(device,vkDescSet, Setlayout);
		}
		tDescriptorSets(sharedDevice& device,vk::DescriptorSet vkDescSet,
				tDescriptorSetLayout::SharedPtr Setlayout
			) :vkDescSet(vkDescSet), Setlayout(Setlayout),device(device) {
			
		
		}
		void SetBuffer(uint32_t binding,tBuffer::SharedPtr buffer) {
			
			if (bindBuffer.count(binding) != 0 && bindBuffer.at(binding)->vkbuffer == buffer->vkbuffer)return;
			bindBuffer[binding] = buffer;
			//updatedBuffer.emplace_back(binding);
			bufferInfos.emplace_back(buffer->vkbuffer, 0, buffer->deviceMemory->getSize());
			writeDescriptorSets.emplace_back(vk::WriteDescriptorSet(vkDescSet, binding, 0, Setlayout->bindings.bindings[binding].descriptorType, nullptr, bufferInfos.back(), {}));

		}
		void SetImage(uint32_t binding, tImageView::SharedPtr image) {
			if (bindImage.count(binding) != 0 && bindImage.at(binding)->vkimageView == image->vkimageView)return;
			bindImage[binding] = image;
			auto type = Setlayout->bindings.bindings[binding].descriptorType;
			switch (type) {
			case vk::DescriptorType::eCombinedImageSampler:
				imageInfos.emplace_back(vk::DescriptorImageInfo(image->sampler->vksampler, image->vkimageView, vk::ImageLayout::eShaderReadOnlyOptimal));
				break;
			case vk::DescriptorType::eSampledImage:
				imageInfos.emplace_back(vk::DescriptorImageInfo({}, image->vkimageView, vk::ImageLayout::eShaderReadOnlyOptimal));
				break;
			case vk::DescriptorType::eSampler:
				imageInfos.emplace_back(vk::DescriptorImageInfo(image->sampler->vksampler, {}, vk::ImageLayout::eUndefined));
				break;
			default:
				throw("Wrong image descriptor Type");
			}
			writeDescriptorSets.emplace_back(vk::WriteDescriptorSet(vkDescSet, binding, 0, type, imageInfos.back(), {}, {}));

		}
		void UpdateDescriptorSets() {
			if (writeDescriptorSets.size() != 0) {
				device.lock()->updateDescriptorSets(writeDescriptorSets, nullptr);
			}
			bufferInfos.clear();
			imageInfos.clear();
			writeDescriptorSets.clear();
		}
		tBuffer* getBuffer(uint32_t binding) {
			return bindBuffer.at(binding).get();
		}
		tImageView* getImage(uint32_t binding) {
			return bindImage.at(binding).get();
		}
		
		
		const std::unordered_map<uint32_t, tBuffer::SharedPtr> getBuffers()const{
			return bindBuffer;
		}
		const std::unordered_map<uint32_t, tImageView::SharedPtr> getImages()const {
			return bindImage;
		}
		tDescriptorSetLayout::SharedPtr Setlayout;
		vk::DescriptorSet vkDescSet;
		
	private:
		std::unordered_map<uint32_t, tBuffer::SharedPtr> bindBuffer;
		std::unordered_map<uint32_t, tImageView::SharedPtr> bindImage;
		std::vector<vk::DescriptorBufferInfo> bufferInfos;
		std::vector < vk::DescriptorImageInfo> imageInfos;
		std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
		
		weakDevice device;
		
	};
	class tDescriptorPool {
	public:
		using SharedPtr = std::shared_ptr<tDescriptorPool>;
		tDescriptorPool(sharedDevice device, std::vector<vk::DescriptorPoolSize> poolInfo) :device(device), poolInfo(poolInfo) {

		}
		tDescriptorPool(sharedDevice device) :device(device) {

		}
		//can add DescriptorInfo only before allocate
		tDescriptorPool& addDescriptorInfo(vk::DescriptorType type, uint32_t count) {
			if (descPool) { throw std::exception("cannot add DescriptorInfo after create descriptorPool"); }

			for (auto& poolSize : poolInfo) {
				if (poolSize.type == type) {
					poolSize.descriptorCount += count;
				}
			}
			return *this;

		}
		std::vector<std::shared_ptr<tDescriptorSets>> AllocateDescriptorSets(std::vector<std::shared_ptr<tDescriptorSetLayout>>& setLayoutBinding);
		void clearAllSets(const sharedDevice& d);

		~tDescriptorPool() {
			if (descPool) {
				if (!device.expired()) {
					const auto& d = device.lock();
					clearAllSets(d);
					d->destroyDescriptorPool(descPool);
				}
				else {
					reportDestroyedAfterDevice();
				}

			}
		}

	private:

		weakDevice device;
		std::vector<vk::DescriptorPoolSize> poolInfo;
		vk::DescriptorPool descPool;
		std::vector<std::shared_ptr<tDescriptorSets>> allocatedDescSets;
	};
}