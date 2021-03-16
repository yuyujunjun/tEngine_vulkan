#pragma once
//#include<mutex>
#include"vulkan/vulkan.hpp"
#include"Core.h"
#include"utils.hpp"
#include<unordered_map>
#include"tResource.h"

#include<mutex>
namespace tEngine {
	//Not ordered by binding
	struct DescriptorLayoutCreateInfo {
		const vk::DescriptorSetLayoutBinding& bindingAt(uint32_t idx)const {
			auto& result= std::find_if(bindings.begin(), bindings.end(),
				[&idx](const vk::DescriptorSetLayoutBinding & b) {
					return  idx == b.binding;
				 });
			return *result;
		}
		vk::DescriptorSetLayoutBinding& bindingAt(uint32_t idx) {
			auto& result = std::find_if(bindings.begin(), bindings.end(),
				[&idx](const vk::DescriptorSetLayoutBinding& b) {
					return  idx == b.binding;
				});
			return *result;
		}
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
	private:
		
		
	};
	struct tDescSetsDataWithSetNumber {
		uint32_t set_number;
		DescriptorLayoutCreateInfo data;
		std::unordered_map<uint32_t, GpuBlockBuffer> blockBuffers;
	};

	void MergeSet(std::vector<tDescSetsDataWithSetNumber>& setLayoutBinding);
	class ResSetBinding;

	class tDescriptorSetLayout {
	public:
		DECLARE_SHARED(tDescriptorSetLayout);
	
		~tDescriptorSetLayout() {
			if (vkLayout) {
				device->destroyDescriptorSetLayout(vkLayout);
				vkLayout = vk::DescriptorSetLayout();
			}
		}
		tDescriptorSetLayout(Device* device,vk::DescriptorSetLayout layout, const DescriptorLayoutCreateInfo& bindings) :device(device), info(bindings),vkLayout(layout) {
		}
		const DescriptorLayoutCreateInfo& getCreateInfo()const {
			return info;
		}
		const vk::DescriptorSetLayout& getVkHandle()const {
			return vkLayout;
		}
	private:
		friend class tDescriptorSetAllocatorManager;
		
	private:
		DescriptorLayoutCreateInfo info;
		vk::DescriptorSetLayout vkLayout;
		weakDevice device;
	};

	
	struct BufferBindInfo {
		
		uint32_t binding;
		BufferHandle buffer;
		bool operator==(const BufferBindInfo& a) {
			return a.binding == binding && a.buffer->getVkHandle() == buffer->getVkHandle();
		}
		bool operator!=(const BufferBindInfo& a) {
			return !(*this == a);
		}
	};
	//If don't set view explicit, we choose imageviewHandle->getDefaultView()
	struct ImageBindInfo {
		uint32_t binding;
	public:
		VkImageView getImageView()const {
			return defaultView == VK_NULL_HANDLE ? image->getDefaultView() : defaultView;
		}
		StockSampler getSampler()const {
			return sampler;
		}
		bool operator==(const ImageBindInfo& a) const {
			auto& left = defaultView == VK_NULL_HANDLE ? image->getDefaultView() : defaultView;
			auto& right = a.defaultView == VK_NULL_HANDLE ? a.image->getDefaultView() : a.defaultView;
			return left == right && a.sampler==sampler && binding==a.binding;
		}
		bool operator!=(const ImageBindInfo& a)const {
			return !(*this == a);
		}
		void SetImageView(ImageviewHandle& handle) {
			image = handle;
		}
	
		void SetView(VkImageView view) {
			defaultView = view;
		}
		void SetSampler(StockSampler sam) {
			sampler = sam;
		}
	private:
		StockSampler  sampler=StockSampler::LinearClamp;
		ImageviewHandle image;
		VkImageView defaultView = VK_NULL_HANDLE;
	};
	class ResSetBinding {
	public:
		friend class tDescriptorPool;
	
		ResSetBinding()  {
		}
		void SetBuffer(uint32_t binding,BufferHandle buffer) {
			auto& bindBuffer = getBuffer(binding).buffer;
			if (bindBuffer->getVkHandle() == buffer->getVkHandle())return;
			bindBuffer = buffer;
		
		}
	
		void SetImage(uint32_t binding, ImageviewHandle handle, VkImageView view= VK_NULL_HANDLE, StockSampler sampler = StockSampler::LinearClamp) {
			auto& bindImage = getImage(binding);
			auto requestView = view == VK_NULL_HANDLE ? handle->getDefaultView() : view;
			if (bindImage.getImageView() == requestView&&bindImage.getSampler()==sampler)return;
			bindImage.SetImageView(handle);
			bindImage.SetView(view);
			bindImage.SetSampler(sampler);
		}
	
		bool operator==(const ResSetBinding& b) {
			return b.bindBuffer == bindBuffer&&bindImage==b.bindImage;
		}
		bool operator!=(const ResSetBinding& b) {
			return !(*this == b);
		}
		bool hasBuffer(uint32_t binding)const {
			auto iter = std::find_if(bindBuffer.begin(), bindBuffer.end(), [&binding](const BufferBindInfo& bind) {return bind.binding == binding; });
			return iter != bindBuffer.end();
		}
		bool hasImage(uint32_t binding) {
			auto iter=std::find_if(bindImage.begin(), bindImage.end(), [&binding](const ImageBindInfo& bind) {return bind.binding == binding; });
			return iter != bindImage.end();
		}
		 BufferBindInfo& getBuffer(uint32_t binding) {
			 auto iter = std::find_if(bindBuffer.begin(), bindBuffer.end(), [&binding](const BufferBindInfo& bind) {return bind.binding == binding; });
			 return *iter;
		}
		 ImageBindInfo& getImage(uint32_t binding) {
			 auto iter = std::find_if(bindImage.begin(), bindImage.end(), [&binding](const ImageBindInfo& bind) {return bind.binding == binding; });
			 return *iter;
		}
	
		const std::vector<BufferBindInfo>& getBuffers()const {
			return bindBuffer;

		}
		const 	std::vector<ImageBindInfo>& getImages()const {
			return bindImage;
		}
		std::vector<BufferBindInfo>& getBuffers() {
			return bindBuffer;

		}
		 std::vector<ImageBindInfo>& getImages() {
			return bindImage;
		}
	private:
		std::vector<BufferBindInfo> bindBuffer;
		std::vector<ImageBindInfo> bindImage;


	};
	
	class tShader;
	
	
	class tDescriptorSetAllocator {
	public:
		tDescriptorSetAllocator(Device* device, DescriptorSetLayoutHandle layout):device(device), layout(layout){
			createPool();
		}
		void createPool();
		vk::DescriptorSet requestDescriptorSet(const ResSetBinding& rb);
		~tDescriptorSetAllocator() {
			if (pool) {
				device->destroyDescriptorPool(pool);
			}
		}
		/*uint32_t nextIdx() {
			if (currentUse < allocatedDescSets.size())return currentUse++;
			allocateDescriptorSet();
			if (currentUse < allocatedDescSets.size())return currentUse++;
			return 0;
		}*/
		const DescriptorSetLayoutHandle getLayout()const {
			return layout;
		}
	private:
		void allocateDescriptorSet();
		uint32_t currentUse = 0;
		const uint32_t minRingSize = 4;
		const uint32_t RingSize = 8;
		Device* device;
		VkDescriptorPool pool;//We assume that pool is big enought 
	//	std::vector<vk::DescriptorSet> allocatedDescSets;
		RingPool<vk::DescriptorSet, ResSetBinding> descriptorSetPool;
	//	std::vector<ResSetBinding> resSetBinding;
		const DescriptorSetLayoutHandle layout;
	};

	//To ensure that the same info of a set produce the same layout
	class tDescriptorSetAllocatorManager {
	public:
		static tDescriptorSetAllocatorManager manager;
		tDescriptorSetAllocatorManager() {}

		DescSetAllocHandle requestSetAllocator(Device* device, const DescriptorLayoutCreateInfo& bindings) {
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
						auto alloc = std::make_shared<tDescriptorSetAllocator>(device,descSetLayout);
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

	private:
		std::mutex mtx;
		//using BindingInfo = std::vector<vk::DescriptorSetLayoutBinding>;

		std::vector<DescriptorLayoutCreateInfo> bindingInfoList;

		std::vector< std::weak_ptr<tDescriptorSetAllocator>> allocList;
	
	//	std::vector<tDescriptorSetAllocator> allocatorList;
	};
}