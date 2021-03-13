#include"tDescriptorPool.h"
#include<unordered_map>
#include <iomanip>
#include <numeric>
#include"tShader.h"
#include"tMaterial.h"
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
	void tDescriptorSetAllocator::AllocatePool() {
		uint32_t maxSets=0;
		std::vector<vk::DescriptorPoolSize> poolSize;
		for (auto& bi : layout->bindings.bindings) {
			poolSize.emplace_back(bi.descriptorType,bi.descriptorCount* RingSize);
			maxSets += bi.descriptorCount * RingSize;
		}
		assert(0 < maxSets);
		vk::DescriptorPoolCreateInfo info;
		info.setPoolSizes(poolSize);
		
		info.setMaxSets(maxSets);
		pool= device->createDescriptorPool(info);
		std::vector<vk::DescriptorSetLayout > descLayouts(minRingSize);
		size_t idx = 0;
		for (auto& lay : descLayouts) {
			lay = layout->vkLayout;
		}
		vk::DescriptorSetAllocateInfo descSetInfo;
		descSetInfo.setDescriptorPool(pool);
		descSetInfo.setSetLayouts(descLayouts);
		auto sets=device->allocateDescriptorSets(descSetInfo);
		allocatedDescSets = sets;
		resSetBinding.resize(sets.size());
	}
	void tDescriptorSetAllocator::allocate_DescriptorSet() {
		if (allocatedDescSets.size() < RingSize) {
			vk::DescriptorSetAllocateInfo descSetInfo;
			descSetInfo.setDescriptorPool(pool);
			descSetInfo.setSetLayouts(layout->vkLayout);
			auto sets = device->allocateDescriptorSets(descSetInfo);
			allocatedDescSets.emplace_back(sets);
			resSetBinding.push_back(ResSetBinding());
		}
	}
	uint32_t tDescriptorSetAllocator::request_DescriptorSet(const ResSetBinding& rb) {
		auto iter = std::find(resSetBinding.begin(), resSetBinding.end(), rb);
		if (iter != resSetBinding.end()) {
			return iter - resSetBinding.begin();
		}

		uint32_t idx = nextIdx();

		std::vector<vk::WriteDescriptorSet> write;
		auto& vkDescSet = allocatedDescSets[idx];
		std::vector< vk::DescriptorBufferInfo> bufferInfo;
		std::vector<vk::DescriptorImageInfo> imageInfo;
		auto& m_binding = resSetBinding[idx];
		for (auto& b : rb.getBuffers()) {
			auto& binding = b.first;
			if (!m_binding.hasBuffer(binding) || resSetBinding[idx].getBuffers().at(binding) != b.second) {
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
			if (!m_binding.hasImage(binding) || resSetBinding[idx].getImages().at(binding) != b.second) {
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
		if (write.size() != 0) {
			device->updateDescriptorSets(write, {});
		}

	}
}