#pragma once
//#include<mutex>
#include"vulkan/vulkan.hpp"
#include"Core.h"
#include"utils.hpp"
#include<unordered_map>

namespace tEngine {
	struct tDescSetsData {
		uint32_t set_number;
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
		std::unordered_map<uint32_t, GpuBlockBuffer> blockBuffers;
	};
	void MergeSet(std::vector<tDescSetsData>& setLayoutBinding);
	class tDescriptorSets;
	class tDescriptorPool {
	public:
		using SharedPtr = std::shared_ptr<tDescriptorPool>;
		tDescriptorPool(sharedDevice device,std::vector<vk::DescriptorPoolSize> poolInfo):device(device),poolInfo(poolInfo){
			
		}
		tDescriptorPool(sharedDevice device) :device(device) {

		}
		//can add DescriptorInfo only when don't allocate
		tDescriptorPool& addDescriptorInfo(vk::DescriptorType type, uint32_t count) {
			if (descPool) { throw std::exception("cannot add DescriptorInfo after create descriptorPool"); }

			for (auto& poolSize : poolInfo) {
				if (poolSize.type == type) {
					poolSize.descriptorCount += count;
				}
			}

		}
		std::vector<std::shared_ptr<tDescriptorSets>> AllocateDescriptorSets( std::vector<tDescSetsData>& setLayoutBinding);
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
	class tDescriptorSets {
	public:
		friend class tDescriptorPool;
		using SharedPtr = std::shared_ptr<tDescriptorSets>;
		static SharedPtr Create(vk::DescriptorSet vkDescSet,
			vk::DescriptorSetLayout vkSetlayout,
			const std::vector<vk::DescriptorSetLayoutBinding>& bindings) {
			return std::make_shared<tDescriptorSets>(vkDescSet, vkSetlayout, bindings);
		}
		tDescriptorSets(vk::DescriptorSet vkDescSet,
				vk::DescriptorSetLayout vkSetlayout,
			const std::vector<vk::DescriptorSetLayoutBinding>& bindings) :vkDescSet(vkDescSet), vkSetlayout(vkSetlayout),bindings(bindings) {
			
		
		}
	
		
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
		vk::DescriptorSetLayout vkSetlayout;
		vk::DescriptorSet vkDescSet;
	private:
		
		
	};

}