#pragma once
//#include<mutex>
#include"vulkan/vulkan.hpp"
#include"tDevice.h"
#include"utils.hpp"
#include<unordered_map>
#include"tResource.h"

#include<mutex>
namespace tEngine {
	//Not ordered by binding
	class DescriptorLayoutCreateInfo {
	public:
		const vk::DescriptorSetLayoutBinding& bindingAt(uint32_t idx)const {
			auto& result= std::find_if(bindings.begin(), bindings.end(),
				[&idx](const vk::DescriptorSetLayoutBinding & b) {
					return  idx == b.binding;
				 });
			return bindings[result-bindings.begin()];
		}
		vk::DescriptorSetLayoutBinding& bindingAt(uint32_t idx) {
			auto& result = std::find_if(bindings.begin(), bindings.end(),
				[&idx](const vk::DescriptorSetLayoutBinding& b) {
					return  idx == b.binding;
				});
			return bindings[result - bindings.begin()];
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
		bool hasUsed = false;
		BufferBindInfo():hasUsed(false), binding(-1) {}
		BufferBindInfo(uint32_t binding, BufferHandle& buffer) :binding(binding), buffer(buffer),hasUsed(true) {}
		uint32_t binding;
		BufferHandle buffer;
		bool operator==(const BufferBindInfo& a)const {
			if (!a.hasUsed || !hasUsed)return true;
			return a.binding == binding && a.buffer->getVkHandle() == buffer->getVkHandle();
		}
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
		VkImageView getImageView()const {
			return defaultView == VK_NULL_HANDLE ? image->get_view()->getDefaultView() : defaultView;
		}
		StockSampler getSampler()const {
			return sampler;
		}
		bool operator==(const ImageBindInfo& a) const {
			if (!hasUsed || !a.hasUsed)return true;
			auto& left = defaultView == VK_NULL_HANDLE ? image->get_view()->getDefaultView() : defaultView;
			auto& right = a.defaultView == VK_NULL_HANDLE ? a.image->get_view()->getDefaultView() : a.defaultView;
			return left == right && a.sampler==sampler && binding==a.binding;
		}
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
		ImageviewHandle getImageViewHandle()const {
			return image->get_view();
		}
	private:
		StockSampler  sampler=StockSampler::LinearClamp;
		ImageHandle image;
		VkImageView defaultView = VK_NULL_HANDLE;
	};
	  
	

	class ResSetBinding {
	public:
		friend class tDescriptorPool;
		ResSetBinding() = default;
		ResSetBinding(uint32_t bindingNumber)  {
			bindBuffer.resize(bindingNumber);
			bindImage.resize(bindingNumber);
		}
		void SetBindingCount(uint32_t bindingCount) {
			bindBuffer.resize(bindingCount);
			bindImage.resize(bindingCount);
		}
		bool isEmpty()const {
			for (auto& b : bindBuffer) {
				if (b.hasUsed)return false;
			}
			for (auto& b : bindImage) {
				if (b.hasUsed)return false;
			}
			return true;
		}
		void SetBuffer(uint32_t binding,BufferHandle buffer) {
			if (bindBuffer[binding].hasUsed) {
				if (bindBuffer[binding].buffer->getVkHandle() == buffer->getVkHandle())return;
			}
			bindBuffer[binding] = BufferBindInfo(binding, buffer);
		}
	
		void SetImage(uint32_t binding, ImageHandle& handle, VkImageView view= VK_NULL_HANDLE, StockSampler sampler = StockSampler::LinearClamp) {
			auto requestView = view == VK_NULL_HANDLE ? handle->get_view()->getDefaultView() : view;
			if (bindImage[binding].hasUsed) {
				if (bindImage[binding].getImageView() == requestView && bindImage[binding].getSampler() == sampler)return;
			}
			
			bindImage[binding] = ImageBindInfo(binding,handle,requestView,sampler);
		}
	
		bool operator==(const ResSetBinding& b)const {
			assert(b.bindBuffer.size() == bindBuffer.size() && bindBuffer.size() == b.bindImage.size() && "same layout must have same binding number");
			return b.bindBuffer == bindBuffer&&bindImage==b.bindImage;
		}
		bool operator!=(const ResSetBinding& b)const {
			return !(*this == b);
		}
		bool hasBuffer(uint32_t binding)const {
			return bindBuffer[binding].hasUsed;
			/*auto iter = std::find_if(bindBuffer.begin(), bindBuffer.end(), [&binding](const BufferBindInfo& bind) {return bind.binding == binding; });
			return iter != bindBuffer.end();*/
		}
		bool hasImage(uint32_t binding)const {
			return bindImage[binding].hasUsed;
			/*auto iter=std::find_if(bindImage.begin(), bindImage.end(), [&binding](const ImageBindInfo& bind) {return bind.binding == binding; });
			return iter != bindImage.end();*/
		}
		 BufferBindInfo getBuffer(uint32_t binding)const {
			 assert(hasBuffer(binding));
			 return bindBuffer[binding];
			/* auto iter = std::find_if(bindBuffer.begin(), bindBuffer.end(), [&binding](const BufferBindInfo& bind) {return bind.binding == binding; });
			 return *iter;*/
		}
		 ImageBindInfo getImage(uint32_t binding)const {
			 assert(hasImage(binding));
			 return bindImage[binding];
			/* auto iter = std::find_if(bindImage.begin(), bindImage.end(), [&binding](const ImageBindInfo& bind) {return bind.binding == binding; });
			 return *iter;*/
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
	//	std::vector< BindResource> bindResource;
		std::vector<BufferBindInfo> bindBuffer;
		std::vector<ImageBindInfo> bindImage;


	};
	
	class tShader;
	class tDescriptorPool {
	public:
		tDescriptorPool(weakDevice device, vk::DescriptorPool pool) :device(device), pool(pool) {}
		~tDescriptorPool() {
			if (pool) {
				device->destroyDescriptorPool(pool);
			}
		}
		vk::DescriptorPool getVkHandle() {
			return pool;
		}
	private:

		weakDevice device;
		vk::DescriptorPool pool;
	};
	class tDescriptorSet {
	public:
		tDescriptorSet(weakDevice device,DescriptorPoolHandle pool,vk::DescriptorSet set,const ResSetBinding& binding):device(device),set(set),pool(pool),binding(binding){}
		vk::DescriptorSet getVkHandle() {
			return set;
		}
		void SetResource(const ResSetBinding& res) {
			binding = res;
		}
		const ResSetBinding& getResource()const { return binding; }
		~tDescriptorSet() {
			device->freeDescriptorSets(pool->getVkHandle(),set);
		}
	private:
		weakDevice device;
		vk::DescriptorSet set;
		//just store, in case delete pool before this set
		DescriptorPoolHandle pool;
		//When binding this descriptorSet, commandBuffer must store resSetBinding
		ResSetBinding binding;
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
		RingPool<tDescriptorSet, ResSetBinding> descriptorSetPool;
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