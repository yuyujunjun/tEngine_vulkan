#pragma once
//#include<mutex>
#include"vulkan/vulkan.hpp"
#include"Core.h"
#include"utils.hpp"
namespace tEngine {
	class tDescriptorSets {
	public:
		tDescriptorSets(const std::vector<vk::DescriptorSet>& descSets):descSets(descSets){}
		void FreeDescriptorSet(vk::Device* device,vk::DescriptorPool pool) {
			device->freeDescriptorSets(pool, descSets);
		}
	private:
		std::vector<vk::DescriptorSet>descSets;
	};
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
		tDescriptorSets* AllocateDescriptorSet(vk::DescriptorSetAllocateInfo descSetInfo) {
			auto d = device.lock();
			if (!descPool&&poolInfo.size()>0) {
				
				descPool = vk::su::createDescriptorPool(*d.get(),poolInfo);
			}
			if (descPool) {
				descSetInfo.setDescriptorPool(descPool);
				allocatedDescSets.emplace_back(std::make_unique<tDescriptorSets>(d->allocateDescriptorSets(descSetInfo)));
				return allocatedDescSets.back().get();
			}
		}

		~tDescriptorPool() {
			if (descPool) {
				if (!device.expired()) {
					device.lock()->destroyDescriptorPool(descPool);
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
		std::vector<std::unique_ptr<tDescriptorSets>> allocatedDescSets;
	//	std::mutex mutex;
	};

}