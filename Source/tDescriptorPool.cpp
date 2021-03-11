#include"tDescriptorPool.h"
#include<unordered_map>
#include <iomanip>
#include <numeric>
#include"tShader.h"
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
	void tDescriptorPool::CreatePool() {
		auto& d = device.lock();
		assert(poolInfo.size() > 0);
		if (!descPool ) {
			uint32_t maxSets =
				std::accumulate(poolInfo.begin(), poolInfo.end(), 0, [](uint32_t sum, vk::DescriptorPoolSize const& dps) {
				return sum + dps.descriptorCount;
					});
			assert(0 < maxSets);

			vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo(
				vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, maxSets, poolInfo);
			descPool = d->createDescriptorPool(descriptorPoolCreateInfo);

		}
	}
	
	bool DescriptorSetResourceFitting(const tDescriptorSets*const descriptorSet, tDescriptorSetLayout*const descLayout, const std::unordered_map<uint32_t, tBuffer::SharedPtr>& buffers
		, const std::unordered_map<uint32_t, tImageView::SharedPtr>& images) {

		return (!descriptorSet->bInUse)
			&&(descriptorSet->Setlayout->vkLayout == descLayout->vkLayout)
			&& (descriptorSet->getBuffers() == buffers) 
			&& (descriptorSet->getImages() == images);
	}
	//return ordered descriptorSets, nullptr for null set
	std::vector<tDescriptorSets::SharedPtr> tDescriptorPool::AllocateDescriptorSets(const tShader::SharedPtr& shader,
		const std::vector<std::unordered_map<uint32_t, tBuffer::SharedPtr>>& buffers
		, const std::vector<std::unordered_map<uint32_t, tImageView::SharedPtr>>& images) {
		const auto& set_num = shader->setsnumberData;
		if (set_num.size() == 0)return std::vector<tDescriptorSets::SharedPtr>();
		uint32_t maxSetSize=0;
		for (int i = 0; i < set_num.size(); ++i) {
			maxSetSize = maxSetSize < set_num[i].set_number ? set_num[i].set_number : maxSetSize;
		}
		maxSetSize += 1;
		std::vector<tDescriptorSets::SharedPtr> results(maxSetSize);
		auto ValidPopulate = [&results,this,&buffers,&images,&set_num,&shader]()->bool {
			bool bValid = true;
			size_t allocatedSize = allocatedDescSets.size();
			for (size_t j = 0; j < set_num.size(); ++j) {
				auto& setData = set_num[j];
				auto& layout = shader->setlayouts[j];
				size_t i = 0;
				for (i = 0; i < allocatedSize; ++i) {
					if (DescriptorSetResourceFitting(allocatedDescSets[i].get(), layout.get(), buffers[j], images[j])) {
						results[setData.set_number] = allocatedDescSets[i];
						break;
					}
				}
				if (i == allocatedSize)
				{
					bValid = false; break;
				}
			}
			return bValid;
		};
		if (ValidPopulate())return results;
		std::lock_guard<std::mutex> lock(mtx);
		if (ValidPopulate())return results;
		//results.clear();
		if (descPool) {
			Log("Allocate DescriptorSets\n", LogLevel::Performance);
			auto& d = device.lock();
			std::vector<vk::DescriptorSet> vkSets;
			std::vector<size_t> newIdx;
			newIdx.reserve(set_num.size());
			vkSets.reserve(set_num.size());
			std::vector<vk::DescriptorSetLayout > descLayouts;
			descLayouts.reserve(set_num.size());
			vk::DescriptorSetAllocateInfo descSetInfo;
			descSetInfo.setDescriptorPool(descPool);
			for (size_t i = 0; i < set_num.size(); ++i) {
				if (results[set_num[i].set_number] == nullptr) {
					descLayouts.emplace_back(shader->setlayouts[i]->vkLayout);
					newIdx.emplace_back(i);
				}
			}
			descSetInfo.setSetLayouts(descLayouts);
			//Allocate sets
			vkSets = d->allocateDescriptorSets(descSetInfo);
			for (int i = 0; i < vkSets.size(); ++i) {
				auto& set = vkSets[i];
				auto& ori_idx = newIdx[i];
				assert(results[set_num[ori_idx].set_number]==nullptr&&"repeat create");
				results[set_num[ori_idx].set_number]=(tDescriptorSets::Create(d,set, shader->setlayouts[i]));
				allocatedDescSets.emplace_back(results[set_num[i].set_number]);
			}
			for (int i = 0; i < results.size(); ++i) {
				if (results[i] != nullptr) {
					for (const auto& buf : buffers[i]) {
						results[i]->SetBuffer(buf.first, buf.second);
					}
					for (const auto& img : images[i]) {
						results[i]->SetImage(img.first, img.second);
					}
				}
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