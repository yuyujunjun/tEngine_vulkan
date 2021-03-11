#include"tDescriptorPool.h"
#include<unordered_map>
#include <iomanip>
#include <numeric>
namespace tEngine {
	//Each element belongs to a set
	void MergeSet(std::vector<tDescSetsData>& setLayoutBinding) {
		using BindMap = std::pair<std::vector<vk::DescriptorSetLayoutBinding>,std::unordered_map<uint32_t,GpuBlockBuffer>>;
		std::unordered_map<uint32_t, BindMap> mappings;
		for (auto& setData : setLayoutBinding) {
			auto& v = mappings[setData.set_number];
			v.first.insert(v.first.end(),setData.bindings.begin(),setData.bindings.end());
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
			setLayoutBinding[idx].bindings = map.second.first;
			setLayoutBinding[idx].set_number = map.first;
			setLayoutBinding[idx].blockBuffers = map.second.second;
			++idx;
		}
		std::sort(setLayoutBinding.begin(), setLayoutBinding.end(),
			[](const tEngine::tDescSetsData& d1,const tEngine::tDescSetsData& d2) {
				assert(d1.set_number != d2.set_number);
				return d1.set_number < d2.set_number;
			});
	}
	//return ordered descriptorSets, nullptr for null set
	std::vector<tDescriptorSets::SharedPtr> tDescriptorPool::AllocateDescriptorSets( std::vector<tDescriptorSetLayout::SharedPtr>& setLayoutBinding) {
		if (setLayoutBinding.size() == 0)return std::vector<tDescriptorSets::SharedPtr>();
		auto& d = device.lock();
		if (!descPool && poolInfo.size() > 0) {
			uint32_t maxSets =
				std::accumulate(poolInfo.begin(), poolInfo.end(), 0, [](uint32_t sum, vk::DescriptorPoolSize const& dps) {
				return sum + dps.descriptorCount;
					});
			assert(0 < maxSets);

			vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo(
				vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, maxSets, poolInfo);
			descPool= d->createDescriptorPool(descriptorPoolCreateInfo);
			
		}
		//First merge all set
		//MergeSet(setLayoutBinding);
		uint32_t maxSetSize=0;
		for (int i = 0; i < setLayoutBinding.size(); ++i) {
			maxSetSize = maxSetSize < setLayoutBinding[i]->bindings.set_number ? setLayoutBinding[i]->bindings.set_number : maxSetSize;
		}
		maxSetSize += 1;
		std::vector<tDescriptorSets::SharedPtr> results(maxSetSize);
		std::vector<vk::DescriptorSet> vkSets(setLayoutBinding.size());
	
		if (descPool) {
			vk::DescriptorSetAllocateInfo descSetInfo;
			descSetInfo.setDescriptorPool(descPool);
			std::vector<vk::DescriptorSetLayout > descLayouts(setLayoutBinding.size());
			for (int i = 0; i < setLayoutBinding.size(); ++i) {
				descLayouts[i] = setLayoutBinding[i]->vkLayout;
			}
			descSetInfo.setSetLayouts(descLayouts);
			//Allocate sets
			vkSets = d->allocateDescriptorSets(descSetInfo);
			for (int i = 0; i < vkSets.size(); ++i) {
				auto& set = vkSets[i];
				results[setLayoutBinding[i]->bindings.set_number]=(tDescriptorSets::Create(d,set, setLayoutBinding[i]));
				allocatedDescSets.emplace_back(results[setLayoutBinding[i]->bindings.set_number]);
			}
		}
		assert(results.size() != 0);
		return results;
	}
	void tDescriptorPool::clearAllSets(const sharedDevice& d) {
		//const auto& d = device.lock();
		std::vector<vk::DescriptorSet> sets(allocatedDescSets.size());
		int idx = 0;
		for (auto& set : allocatedDescSets) {
			sets[idx++] = set->vkDescSet;
		}
		d->freeDescriptorSets(descPool,sets);
		allocatedDescSets.clear();
	}
}