#pragma once
#include"vulkan/vulkan.h"
#include<vulkan/vulkan.hpp>
#include<memory>
#include<assert.h>
#include<iostream>
#include<mutex>
#include"vma/src/vk_mem_alloc.h"
#define VULKAN_DEBUG DEBUG
namespace tEngine {
#define VK_ASSERT(T) assert(T)
#define DECLARE_SHARED(TYPE) using SharedPtr=std::shared_ptr<TYPE>;
#define DECLARE_NO_COPY_SEMANTICS(TYPE) \
	TYPE(const TYPE&) = delete; \
	const TYPE& operator=(const TYPE&) = delete;
	/// <summary>Aligns a given number based on the given alignment</summary>
/// <param name="numberToAlign">A number ot align based alignment</param>
/// <param name="alignment">The value to which the numberToAlign will be aligned</param>
/// <returns>An aligned value</returns>
	template<typename t1, typename t2>
	inline t1 align(t1 numberToAlign, t2 alignment)
	{
		if (alignment)
		{
			t1 align1 = numberToAlign % (t1)alignment;
			if (!align1) { align1 += (t1)alignment; }
			numberToAlign += t1(alignment) - align1;
		}
		return numberToAlign;
	}
	
	struct GpuBlockMember {
		
		std::string name = "";
		uint32_t offset = static_cast<uint32_t>(-1);
		uint32_t size = static_cast<uint32_t>(-1);
		bool operator==(const GpuBlockMember& b2) {
			return name == b2.name && offset == b2.offset && size == b2.size;
		}
		bool operator!=(const GpuBlockMember& b) {
			return !(*this == b);
		}
	};
	struct GpuBlockBuffer {
		GpuBlockMember& operator[](int id)noexcept {
			assert(id < data.size() && "id exceed blocks");
			return data[id];
		}
		uint32_t ByteSize()const {
			if (data.empty())return 0;
			return data.back().size + data.back().offset;
		}
		/*char* RequestMemory() {
			if (ByteSize() == 0)return nullptr;
			return new char[ByteSize()];
		}*/
		size_t size() const {
			return data.size();
		}
		void push_back(GpuBlockMember _data) {
			data.push_back(_data);
		}
		void AddElement(std::string name, uint32_t size) {
			GpuBlockMember _data;
			_data.size = size;
			_data.offset = align(ByteSize(), _data.size);
			_data.name = name;
			data.push_back(_data);
		}
		GpuBlockBuffer operator+(const GpuBlockBuffer& block) {
			if (data.size() == 0)return block;
			else if (block.size() == 0)return *this;
			//将this的末尾对齐新block的开头
			int offset = align(ByteSize(), block.begin()->size);
			GpuBlockBuffer _block = *this;
			for (auto& _data : block) {
				_block.push_back(_data);
				_block.back().offset += offset;
			}
			return _block;
		}
		GpuBlockBuffer operator+=(const GpuBlockBuffer& block) {
			*this = *this + block;
			return *this;
		}
		using iterator = std::vector<GpuBlockMember>::iterator;
		using const_iterator = std::vector<GpuBlockMember>::const_iterator;
		const_iterator begin()const { return data.cbegin(); }
		iterator begin() { return data.begin(); }
		iterator end() { return data.end(); }
		const_iterator end()const { return data.cend(); }
		GpuBlockMember& back() { return data.back(); }

		std::string name;
	//	vk::DescriptorType descType;
	private:
		
		std::vector<GpuBlockMember> data;
	};

	struct tPhysicalDevice {
		tPhysicalDevice() {};
		
		void SetPhysicalDevice(vk::PhysicalDevice physicalDevice) {
			this->physicalDevice = physicalDevice;
			memoryProperties = physicalDevice.getMemoryProperties();
			deviceProperties = physicalDevice.getProperties();
			
		}
		const vk::PhysicalDeviceMemoryProperties& getMemoryProperties()const {

			return memoryProperties;
		}
		const vk::PhysicalDeviceProperties& getProperties()const {
			return deviceProperties;
		}
		const vk::FormatProperties& getFormatProperties(vk::Format format)const {
			return physicalDevice.getFormatProperties(format);
		}
		const vk::PhysicalDevice& getPhysicalDevice() const{
			return physicalDevice;
		}
		vk::PhysicalDevice& operator()() {
			return physicalDevice;
		}
		bool bUniqueQueueFamily()const {
			assert(graphicsQueuefamilyId == presentQueuefamilyId&&"our system need graphic queue equals to prsent queue");
			return graphicsQueuefamilyId == presentQueuefamilyId &&
				presentQueuefamilyId == computeQueuefamilyId &&
				computeQueuefamilyId == transferQueuefamilyId;
		}


		uint32_t graphicsQueuefamilyId=-1;
		uint32_t presentQueuefamilyId=-1;
		uint32_t computeQueuefamilyId=-1;
		uint32_t transferQueuefamilyId = -1;
		vk::PhysicalDeviceMemoryProperties memoryProperties;
		vk::PhysicalDeviceProperties deviceProperties;
		vk::PhysicalDevice physicalDevice;
	//	vk::FormatProperties formatProps;
	};
	class tBuffer;
	using BufferHandle = std::shared_ptr<tBuffer>;
	class tImage;
	using ImageHandle = std::shared_ptr<tImage>;// ::SharedPtr;
	class tSampler;
	using SamplerHandle = std::shared_ptr<tSampler>;
	class tImageView;
	using ImageviewHandle = std::shared_ptr<tImageView>;
	class CommandBufferBase_;
	class CommandBuffer;
	class SecondaryCommandBuffer;
	using SecondaryCBHandle = std::shared_ptr<SecondaryCommandBuffer>;
	using CommandBufferHandle = std::shared_ptr <CommandBuffer>;
	class BufferCreateInfo;
	class ImageAsset;
	class ImageCreateInfo;
	class TextureFormatLayout;
	class InitialImageBuffer;
	class SamplerCreateInfo;
	class ResSetBinding;

	enum class StockSampler
	{
		NearestClamp,
		LinearClamp,
		TrilinearClamp,
		NearestWrap,
		LinearWrap,
		TrilinearWrap,
		NearestShadow,
		LinearShadow,
		LinearYUV420P,
		LinearYUV422P,
		LinearYUV444P,
		Count
	};
	struct Device:public vk::Device {
	public:
		Device(vk::Device device, VmaAllocator allocator);
		void Device::init_stock_samplers();
		void free_allocation(VmaAllocation allocation)const;
		bool image_format_is_supported(VkFormat format, VkFormatFeatureFlags required, VkImageTiling tiling) const;
		VkFormat Device::get_default_depth_stencil_format() const;
		VkFormat Device::get_default_depth_format() const;
		inline  uint32_t minBufferAlignment()const;
		bool memory_type_is_host_visible(uint32_t type) const;
		BufferHandle create_buffer(const BufferCreateInfo& createInfo,const void* initial=nullptr,CommandBufferHandle cb=nullptr)const;
		ImageHandle create_image(const ImageCreateInfo& info, std::shared_ptr<ImageAsset> initial = nullptr, CommandBufferHandle cb = nullptr)const;
		ImageHandle create_image_from_staging_buffer(const ImageCreateInfo& info, const InitialImageBuffer* buffer=nullptr, CommandBufferHandle cb = nullptr)const;
		
		InitialImageBuffer create_image_staging_buffer(const ImageCreateInfo& info, const ImageAsset* initial)const;
		InitialImageBuffer create_image_staging_buffer(const TextureFormatLayout& layout)const;
		
		SamplerHandle Device::create_sampler(const SamplerCreateInfo& sampler_info);
		SamplerHandle Device::getSampler(const StockSampler& sampler)const;
		tPhysicalDevice physicalDevice;
		VmaAllocator allocator;
	private:
		std::array<SamplerHandle, static_cast<unsigned>(StockSampler::Count)> samplers;
		void fill_buffer_sharing_indices(VkBufferCreateInfo& info, uint32_t* sharing_indices)const;

	};
	using weakDevice = const Device*;
	using uniqueDevice = std::unique_ptr<Device>;
	enum class LogLevel
	{
		Verbose = 0,
		Debug = 1,
		Information = 2,
		Warning = 3,
		Error = 4,
		Critical = 5,
		Performance = 6,
		None = 100,
	};
	struct Logger {
		template<typename T>
		static void Log(const T& info,LogLevel level) {
			std::stringstream temp;
			temp<< info;
			
			LogStyle(temp.str(), level);
			
		}
	private:
		static void LogStyle(const std::string& s,LogLevel level);
		static std::stringstream stream;
	};
	
	template<typename T>
	void LOGI(T args) {
		Logger::Log(args);
	}
	template<typename T, typename ...Args>
	void LOGI(T value, Args... args) {
		LOGI(value);
		LOGI(args...);
	}
	template<typename T>
	void Log(T info, LogLevel level=LogLevel::Information) {
		Logger::Log(info, level);
	}
	inline void SetRange(const VmaAllocation allocation, void* data, size_t size, size_t offset = 0) {
		assert(allocation->GetMappedData() != nullptr);
		memcpy(static_cast<char*>(allocation->GetMappedData()) + offset, data, size);
	}

}