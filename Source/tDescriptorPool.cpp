#include"tDescriptorPool.h"
#include<unordered_map>
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
	}
	std::vector<tDescriptorSets::SharedPtr> tDescriptorPool::AllocateDescriptorSets( std::vector<tDescSetsData>& setLayoutBinding) {
		
		auto d = device.lock();
		if (!descPool && poolInfo.size() > 0) {

			descPool = vk::su::createDescriptorPool(*d.get(), poolInfo);
		}
		//First merge all set
		MergeSet(setLayoutBinding);
		std::vector<tDescriptorSets::SharedPtr> results(setLayoutBinding.size());
		std::vector<vk::DescriptorSetLayout> vkLayouts(setLayoutBinding.size());
		std::vector<vk::DescriptorSet> vkSets(setLayoutBinding.size());
		//There might be the same set
		{
			int idx = 0;
			for (auto& eachSet : setLayoutBinding) {
				vk::DescriptorSetLayoutCreateInfo layoutsCreateInfo;
				layoutsCreateInfo.setBindings(eachSet.bindings);
				vkLayouts[idx++] = d->createDescriptorSetLayout(layoutsCreateInfo);
			}
		}
		if (descPool) {
			vk::DescriptorSetAllocateInfo descSetInfo;
			descSetInfo.setDescriptorPool(descPool);
			descSetInfo.setSetLayouts(vkLayouts);
			//Allocate sets
			vkSets = d->allocateDescriptorSets(descSetInfo);
			for (int i = 0; i < vkSets.size(); ++i) {
				auto& set = vkSets[i];
				results.emplace_back(tDescriptorSets::Create(set,vkLayouts[i],setLayoutBinding[i].bindings));
				allocatedDescSets.emplace_back(results.back());
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
			d->destroyDescriptorSetLayout(set->vkSetlayout);
			sets[idx++] = set->vkDescSet;
		}
		d->freeDescriptorSets(descPool,sets);
		allocatedDescSets.clear();
	}
}