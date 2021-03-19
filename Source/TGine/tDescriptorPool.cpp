#include"tDescriptorPool.h"
#include<unordered_map>
#include <iomanip>
#include <numeric>
#include"PriorityAllocator.h"
#include"tGpuBlock.h"
#include"tLog.h"
#include"tResource.h"
namespace tEngine {

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
	
	bool BufferBindInfo::operator==(const BufferBindInfo& a)const {
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
	}

}