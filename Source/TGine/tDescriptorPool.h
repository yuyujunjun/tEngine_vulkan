#pragma once
//#include<mutex>
#include"vulkan/vulkan.hpp"
#include"tGine.h"
#include"tDescriptorShared.h"
#include<unordered_map>
#include"PriorityAllocator.h"
#include<mutex>
namespace tEngine {
	//Not ordered by binding
	
	

	class tDescriptorSetLayout {
	public:
		DECLARE_SHARED(tDescriptorSetLayout);
	
		
		tDescriptorSetLayout(Device* device,vk::DescriptorSetLayout layout, const DescriptorLayoutCreateInfo& bindings) :device(device), info(bindings),vkLayout(layout) {
		}
		const DescriptorLayoutCreateInfo& getCreateInfo()const {
			return info;
		}
		const vk::DescriptorSetLayout& getVkHandle()const {
			return vkLayout;
		}
		tDescriptorSetLayout::~tDescriptorSetLayout();
	private:
		friend class tDescriptorSetAllocatorManager;
		
	private:
		DescriptorLayoutCreateInfo info;
		vk::DescriptorSetLayout vkLayout;
		weakDevice device;
	};
	
	
	struct BufferBindInfo {
		bool hasUsed = false;
		BufferBindInfo():hasUsed(false), binding(-1),range(-1) {}
		BufferBindInfo(uint32_t binding, BufferHandle& buffer,uint32_t range) :range(range),binding(binding), buffer(buffer),hasUsed(true) {}
		uint32_t binding;
		uint32_t range;
		BufferHandle buffer;
		bool operator==(const BufferBindInfo& a)const;
		bool operator!=(const BufferBindInfo& a)const {
			return !(*this == a);
		}
	};
	//If don't set view explicit, we choose imageviewHandle->getDefaultView()
	struct ImageBindInfo {
		bool hasUsed = false;
		ImageBindInfo():hasUsed(false),binding(-1){}
		ImageBindInfo(uint32_t binding, ImageHandle image, VkImageView defaultView, StockSampler  sampler) :binding(binding), image(image),
			defaultView(defaultView), sampler(sampler),hasUsed(true) {}
		uint32_t binding;
	public:
		VkImageView getImageView()const;
		StockSampler getSampler()const;
		bool operator==(const ImageBindInfo& a) const;
		bool operator!=(const ImageBindInfo& a)const {
			return !(*this == a);
		}
		void SetImage(ImageHandle& handle) {
			image = handle;
		}
	
		void SetView(VkImageView view) {
			defaultView = view;
		}
		void SetSampler(StockSampler sam) {
			sampler = sam;
		}
		ImageHandle getImageResource()const {
			return image;
		}
		ImageviewHandle getImageViewHandle()const;
	private:
		StockSampler  sampler=StockSampler::LinearClamp;
		ImageHandle image;
		VkImageView defaultView = VK_NULL_HANDLE;
	};

	class tDescriptorPool {
	public:
		tDescriptorPool(weakDevice device, vk::DescriptorPool pool) :device(device), pool(pool) {}
		
		vk::DescriptorPool getVkHandle() {
			return pool;
		}
		~tDescriptorPool();
	private:

		weakDevice device;
		vk::DescriptorPool pool;
	};
	class tDescriptorSet {
	public:
		tDescriptorSet(weakDevice device,DescriptorPoolHandle pool,vk::DescriptorSet set,const ResSetBinding& binding):device(device),set(set),pool(pool), setInfo(binding){}
		vk::DescriptorSet getVkHandle() {
			return set;
		}
	/*	void SetResource(const ResSetBinding& res) {
			setInfo = res;
		}*/
		const ResSetBinding& getResource()const { return setInfo; }
		~tDescriptorSet();
	private:
		weakDevice device;
		vk::DescriptorSet set;
		//just store, in case delete pool before this set
		DescriptorPoolHandle pool;
		//When binding this descriptorSet, commandBuffer must store resSetBinding
		ResSetBinding setInfo;
	};
	class descRingPool :public RingPool<tDescriptorSet, ResSetBinding> {
	public:
		std::shared_ptr<tDescriptorSet> request(const ResSetBinding& value)override;
	};
	class tDescriptorSetAllocator {
	public:
		tDescriptorSetAllocator(Device* device, DescriptorSetLayoutHandle layout):device(device), layout(layout){
			createPool();
		}
		void createPool();
		std::shared_ptr<tDescriptorSet> requestDescriptorSet(const ResSetBinding& rb);
		const DescriptorSetLayoutHandle getLayout()const {
			return layout;
		}
	private:
		const uint32_t RingSize = 8;
		Device* device;
		DescriptorPoolHandle pool;
		descRingPool descriptorSetPool;
		const DescriptorSetLayoutHandle layout;
	};

	//To ensure that the same info of a set produce the same layout
	class tDescriptorSetAllocatorManager {
	public:
		static tDescriptorSetAllocatorManager manager;
		tDescriptorSetAllocatorManager() {}

		DescSetAllocHandle requestSetAllocator(Device* device, const DescriptorLayoutCreateInfo& bindings);

	private:
		std::mutex mtx;
		std::vector<DescriptorLayoutCreateInfo> bindingInfoList;
		std::vector< std::weak_ptr<tDescriptorSetAllocator>> allocList;
	};
}