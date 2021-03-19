#include"DescriptorPool.h"
#include<unordered_map>
#include <iomanip>
#include <numeric>
#include"GpuBlock.h"
#include"Log.h"
#include"Buffer.h"
#include"Sampler.h"
#include"Image.h"
#include"Device.h"
#include"GpuBlock.h"
namespace tEngine {
	tDescriptorSetAllocatorManager tDescriptorSetAllocatorManager::manager;
	//Each element belongs to a set
	void MergeSet(std::vector<tDescSetsDataWithSetNumber>& setLayoutBinding) {
		using BindMap = std::pair<std::vector<vk::DescriptorSetLayoutBinding>,std::unordered_map<uint32_t,GpuBlockBuffer>>;
		std::unordered_map<uint32_t, BindMap> mappings;
		for (auto& setData : setLayoutBinding) {
			auto& v = mappings[setData.set_number];
			v.first.insert(v.first.end(),setData.data.bindings.begin(),setData.data.bindings.end());
			for (auto& buffer : setData.blockBuffers) {
				v.second.insert(v.second.end(),buffer);
			}
			
		}

		for (auto& map : mappings) {
			uint32_t set_number = map.first;
			auto& bindings = map.second;
			std::unordered_map<uint32_t, std::tuple<vk::DescriptorType, uint32_t,vk::ShaderStageFlags>> bindMap;//binding,Type,count
			for (const auto& b : bindings.first) {
				if (bindMap.count(b.binding) == 0) {
					bindMap[b.binding] = { b.descriptorType,b.descriptorCount,b.stageFlags };
				}
				else {
					auto& m = bindMap[b.binding];
					assert(std::get<0>(m) == b.descriptorType && std::get<1>(m) == b.descriptorCount&&"Binding Error");
					std::get<2>(m) |= b.stageFlags;
				}
			}
			bindings.first.resize(bindMap.size());
			int idx = 0;
			for (auto& b : bindMap) {
				bindings.first[idx] = vk::DescriptorSetLayoutBinding();
				bindings.first[idx].binding = b.first;
				bindings.first[idx].descriptorType = std::get<0>(b.second);
				bindings.first[idx].descriptorCount =std::get<1>( b.second);
				bindings.first[idx].stageFlags =std::get<2>( b.second);
				++idx;
			}
		}
		setLayoutBinding.resize(mappings.size());
		int idx = 0;
		for (auto& map : mappings) {
			setLayoutBinding[idx].data.bindings = map.second.first;
			setLayoutBinding[idx].set_number = map.first;
			setLayoutBinding[idx].blockBuffers = map.second.second;
			++idx;
		}
		std::sort(setLayoutBinding.begin(), setLayoutBinding.end(),
			[](const tEngine::tDescSetsDataWithSetNumber& d1,const tEngine::tDescSetsDataWithSetNumber& d2) {
				assert(d1.set_number != d2.set_number);
				return d1.set_number < d2.set_number;
			});
	}

	std::shared_ptr<tDescriptorSet> descRingPool:: request(const ResSetBinding& value) {
		auto iter = std::find_if(order.begin(), order.end(), [this, value](const uint32_t& a) {
			//if bindPoint is empty or 
			for (int i = 0; i < value.size();++i) {
				const auto& v = value[i];
				if (!attribute[a][i].emptyResource()&&!v.emptyResource()&&attribute[a][i] != v)return false;
			}
			return true;
			}

		);
		if (iter != order.end()) {

			uint32_t idx = *iter;
			if (iter != order.begin()) {
				order.erase(iter);
				order.push_front(idx);
			}
			return objectPool[idx];
		}
		else {
			return nullptr;
		}
	}
	tDescriptorSet::~tDescriptorSet() {
		device->freeDescriptorSets(pool->getVkHandle(), set);
	}
	void tDescriptorSetAllocator::createPool() {
		uint32_t maxSets = 0;
		std::vector<vk::DescriptorPoolSize> poolSize;
		for (auto& bi : layout->getCreateInfo().bindings) {
			poolSize.emplace_back(bi.descriptorType, bi.descriptorCount * RingSize);
			maxSets += bi.descriptorCount * RingSize;
		}
		assert(0 < maxSets);
		vk::DescriptorPoolCreateInfo info = {};
		info.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
		info.setPoolSizes(poolSize);

		info.setMaxSets(maxSets);
		auto vkpool = device->createDescriptorPool(info);
		pool = std::make_shared<tDescriptorPool>(device, vkpool);
	}
	DescSetAllocHandle tDescriptorSetAllocatorManager::requestSetAllocator(Device* device, const DescriptorLayoutCreateInfo& bindings) {
		//find first
		for (size_t i = 0; i < bindingInfoList.size(); ++i) {
			if (bindings.bindings == bindingInfoList[i].bindings) {
				if (!allocList[i].expired()) {
					return allocList[i].lock();
				}
			}
		}
		std::lock_guard<std::mutex> lock(mtx);

		for (size_t i = 0; i < bindingInfoList.size(); ++i) {
			if (bindings.bindings == bindingInfoList[i].bindings) {
				if (!allocList[i].expired()) {
					return allocList[i].lock();
				}
				else {
					vk::DescriptorSetLayoutCreateInfo info;
					info.setBindings(bindings.bindings);
					auto vkLayout = device->createDescriptorSetLayout(info);
					auto descSetLayout = std::make_shared<tDescriptorSetLayout>(device, vkLayout, bindings);
					auto alloc = std::make_shared<tDescriptorSetAllocator>(device, descSetLayout);
					allocList[i] = alloc;
					return alloc;
				}
			}
		}
		vk::DescriptorSetLayoutCreateInfo info;
		info.setBindings(bindings.bindings);
		auto vkLayout = device->createDescriptorSetLayout(info);
		auto descSetLayout = std::make_shared<tDescriptorSetLayout>(device, vkLayout, bindings);
		auto alloc = std::make_shared<tDescriptorSetAllocator>(device, descSetLayout);
		allocList.push_back(alloc);
		bindingInfoList.push_back(bindings);
		return alloc;
	}
	tDescriptorSetLayout::~tDescriptorSetLayout() {
		if (vkLayout) {
			device->destroyDescriptorSetLayout(vkLayout);
			vkLayout = vk::DescriptorSetLayout();
		}
	}
	bool BindingResourceInfo::operator==(const BindingResourceInfo& info)const {
		bool equal = dstBinding == info.dstBinding && dstArrayElement == info.dstArrayElement && type == info.type && view == info.view && sampler == info.sampler
			&& range == info.range;
		if (!equal)return false;

		if (buffer && info.buffer) {
			return  buffer->getVkHandle() == info.buffer->getVkHandle();
		}
		else if (image && info.image) {
			return image->getVkHandle() == info.image->getVkHandle();
		}
		else return true;


	}
	tDescriptorPool::~tDescriptorPool() {
		if (pool) {
			device->destroyDescriptorPool(pool);
		}
	}
	std::shared_ptr<tDescriptorSet> tDescriptorSetAllocator::requestDescriptorSet(const ResSetBinding& rb) {
		if (rb.size() == 0)return nullptr;

		auto re = descriptorSetPool.request(rb);
		if (re != nullptr) {
			return re;
		}
		//Only need rebind
		if (descriptorSetPool.isFull()) {
			re = descriptorSetPool.moveLastToFront(rb);
		}
		else {//Create a new descriptorSet
			LOGD(LogLevel::Performance, "Allocate descriptorSet");
			vk::DescriptorSetAllocateInfo descSetInfo;
			descSetInfo.setDescriptorPool(pool->getVkHandle());
			descSetInfo.setSetLayouts(layout->getVkHandle());
			auto set = device->allocateDescriptorSets(descSetInfo);
			re = descriptorSetPool.allocate(rb, device, pool, set[0], rb);
		}



		std::vector<vk::WriteDescriptorSet> write;
		//allocate enough memory for vector, we need use address of them
		std::vector< vk::DescriptorBufferInfo> bufferInfo;
		bufferInfo.reserve(rb.size());
		//bufferInfo.reserve(rb.getBuffers().size());
		std::vector<vk::DescriptorImageInfo> imageInfo;
		imageInfo.reserve(rb.size());
		//imageInfo.reserve(rb.getImages().size());
		for (auto& bindingInfo : rb) {
			if (bindingInfo.emptyResource())continue;
			auto& resource = bindingInfo;
			switch (resource.type) {
			case vk::DescriptorType::eUniformBufferDynamic:
			case vk::DescriptorType::eStorageBufferDynamic:
				//assert(resource.offset == 0);
				bufferInfo.emplace_back(vk::DescriptorBufferInfo(resource.buffer->getVkHandle(), 0, resource.range));
				;
				write.emplace_back(vk::WriteDescriptorSet(re->getVkHandle(), resource.dstBinding, resource.dstArrayElement, 1, resource.type, nullptr, &bufferInfo.back(), nullptr));
				break;
			case vk::DescriptorType::eCombinedImageSampler:
			case vk::DescriptorType::eStorageImage:
				imageInfo.emplace_back(device->getSampler(resource.sampler)->getVkHandle(), resource.view, vk::ImageLayout::eShaderReadOnlyOptimal);
				write.emplace_back(vk::WriteDescriptorSet(re->getVkHandle(), resource.dstBinding, resource.dstArrayElement, 1, resource.type, &imageInfo.back(), nullptr, nullptr));
				break;
			default:assert(false && "can't support other type for now");
			}
		}

		if (write.size() != 0) {
			LOGD(LogLevel::Performance, "Rebind descriptorSet");
			device->updateDescriptorSets(write, {});
		}
		return re;
	}
	/*bool BufferBindInfo::operator==(const BufferBindInfo& a)const {
		if (!a.hasUsed || !hasUsed)return true;
		return a.binding == binding && a.buffer->getVkHandle() == buffer->getVkHandle() && a.range == range;
	}
	VkImageView ImageBindInfo::getImageView()const {
		return defaultView == VK_NULL_HANDLE ? image->get_view()->getDefaultView() : defaultView;
	}
	StockSampler ImageBindInfo::getSampler()const {
		return sampler;
	}
	bool ImageBindInfo::operator==(const ImageBindInfo& a) const {
		if (!hasUsed || !a.hasUsed)return true;
		auto& left = defaultView == VK_NULL_HANDLE ? image->get_view()->getDefaultView() : defaultView;
		auto& right = a.defaultView == VK_NULL_HANDLE ? a.image->get_view()->getDefaultView() : a.defaultView;
		return left == right && a.sampler == sampler && binding == a.binding;
	}
	ImageviewHandle ImageBindInfo::getImageViewHandle()const {
		return image->get_view();
	}*/

}