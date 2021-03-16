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
	void tDescriptorSetAllocator::createPool() {
		uint32_t maxSets=0;
		std::vector<vk::DescriptorPoolSize> poolSize;
		for (auto& bi : layout->getCreateInfo().bindings) {
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
			lay = layout->getVkHandle();
		}
		vk::DescriptorSetAllocateInfo descSetInfo;
		descSetInfo.setDescriptorPool(pool);
		descSetInfo.setSetLayouts(descLayouts);
		auto sets=device->allocateDescriptorSets(descSetInfo);
		allocatedDescSets = sets;
		resSetBinding.resize(sets.size());
	}

	vk::DescriptorSet tDescriptorSetAllocator::requestDescriptorSet(const ResSetBinding& rb) {
		if (rb.getBuffers().size() == 0 && rb.getImages().size() == 0)return vk::DescriptorSet();
		auto re=descriptorSetPool.request(rb);
		if (re != nullptr) {
			return *re;
		}
		vk::DescriptorSetAllocateInfo descSetInfo;
		descSetInfo.setDescriptorPool(pool);
		descSetInfo.setSetLayouts(layout->getVkHandle());
		auto set = device->allocateDescriptorSets(descSetInfo);
		re = descriptorSetPool.allocate(rb,set.begin());
		std::vector<vk::WriteDescriptorSet> write;
		auto& vkDescSet = *re;
		std::vector< vk::DescriptorBufferInfo> bufferInfo;
		std::vector<vk::DescriptorImageInfo> imageInfo;
	//	auto m_binding = descriptorSetPool.getFirstAttribute();
		for (auto& b : rb.getBuffers()) {
			auto& binding = b.binding;
			{
			//	m_binding.getBuffers()[binding] = b;
				auto& buffer = b.buffer;
				bufferInfo.emplace_back(buffer->getVkHandle(), 0, buffer->getSize());
				write.emplace_back(vk::WriteDescriptorSet(vkDescSet, binding, 0, layout->getCreateInfo().bindingAt(binding).descriptorType, nullptr, bufferInfo.back(), {}));
			}
		}

		
		for (auto& b : rb.getImages()) {
			auto& binding = b.binding;
			 {
			//	m_binding.getImages()[binding] = b;
				auto type = layout->getCreateInfo().bindingAt(binding).descriptorType;
				
				switch (type) {
				case vk::DescriptorType::eCombinedImageSampler:
					imageInfo.emplace_back(vk::DescriptorImageInfo(device->getSampler(b.getSampler())->getVkHandle(), b.getImageView(), vk::ImageLayout::eShaderReadOnlyOptimal));
					break;
				case vk::DescriptorType::eSampledImage:
					imageInfo.emplace_back(vk::DescriptorImageInfo({}, b.getImageView(), vk::ImageLayout::eShaderReadOnlyOptimal));
					break;
				case vk::DescriptorType::eSampler:
					imageInfo.emplace_back(vk::DescriptorImageInfo(device->getSampler(b.getSampler())->getVkHandle(), {}, vk::ImageLayout::eUndefined));
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
		return *re;
	}
}