#pragma once
#include"vulkan/vulkan.h"
#include<vulkan/vulkan.hpp>
#include<memory>
#include<assert.h>
#include<iostream>
#include<mutex>
#include"vma/src/vk_mem_alloc.h"
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
	class tBuffer;
	using BufferHandle = std::shared_ptr<tBuffer>;
	class tImage;
	using ImageHandle = std::shared_ptr<tImage>;// ::SharedPtr;
	class tSampler;
	using SamplerHandle = std::shared_ptr<tSampler>;
	class tImageView;
	using ImageviewHandle = std::shared_ptr<tImageView>;
	class CommandBuffer;

	//using SecondaryCBHandle = std::shared_ptr<CommandBuffer>;
	using CommandBufferHandle = std::shared_ptr <CommandBuffer>;
	class BufferCreateInfo;
	class ImageAsset;
	class ImageCreateInfo;
	class TextureFormatLayout;
	class InitialImageBuffer;
	class SamplerCreateInfo;
	class ResSetBinding;
	class tDescriptorSetLayout;
	using DescriptorSetLayoutHandle = std::shared_ptr<tDescriptorSetLayout>;
	class DescriptorLayoutCreateInfo;
	class tDescriptorSetAllocatorManager;
	using DescSetAllocManagerHandle = std::shared_ptr<tDescriptorSetAllocatorManager>;
	class tDescriptorSetAllocator;
	using DescSetAllocHandle = std::shared_ptr<tDescriptorSetAllocator>;
	class tDescriptorPool;
	using DescriptorPoolHandle = std::shared_ptr<tDescriptorPool>;
	class tDescriptorSet;
	using DescriptorSetHandle = std::shared_ptr<tDescriptorSet>;
	class tSwapChain;
	using SwapChainHandle = std::shared_ptr<tSwapChain>;
	class tRenderPass;
	using RenderPassHandle = std::shared_ptr<tRenderPass>;
	class tFrameBuffer;
	using FrameBufferHandle = std::shared_ptr<tFrameBuffer>;
	class tPipelineLayout;
	using PipelineLayoutHandle = std::shared_ptr<tPipelineLayout>;
	class tShaderInterface;
	class GraphicsPipelineCreateInfo;
	struct tPipeline;
	using PipelineHandle = std::shared_ptr<tPipeline>;
	class tCommandPool;
	using CommandPoolHandle = std::shared_ptr<tCommandPool>;
	class tFence;
	using FenceHandle = std::shared_ptr<tFence>;
	class FenceManager;
	class SemaphoreManager;
	class Device;
	class tPhysicalDevice;
	using weakDevice = const Device*;
	struct ImageViewCreateInfo;
	void SetRange(const VmaAllocation allocation, void* data, size_t size, size_t offset = 0);
	bool imageFormat_is_supported(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatFeatureFlags required, VkImageTiling tiling);
	VkFormat default_depth_format(VkPhysicalDevice physicalDevice);
	VkFormat default_depth_stencil_format(VkPhysicalDevice physicalDevice);
	inline  uint32_t minBufferAlignment(const tPhysicalDevice& physicalDevice);
	void fill_buffer_sharing_indices(const tPhysicalDevice& physicalDevice, VkBufferCreateInfo& info, uint32_t* sharing_indices);
	bool memory_type_is_host_visible(const tPhysicalDevice& physicalDevice, uint32_t type);
	void FreeMemory(VmaAllocator allocator, VmaAllocation allocation);
	void FlushMemory(VmaAllocator allocator, VmaAllocation allocation, size_t offset, size_t size);
	void* getMappedData(VmaAllocation allocation);
	uint32_t getMemoryTypeIdx(VmaAllocation allocation);
	size_t getAllocationSize(VmaAllocation allocation);
	CommandBufferHandle allocateCommandBuffer(weakDevice device, CommandPoolHandle cmdPool);
	uint32_t findQueueFamilyIndex(std::vector<vk::QueueFamilyProperties>const& queueFamilyProperties, vk::QueueFlagBits bits);
	VkImageViewType get_image_view_type(const ImageCreateInfo& create_info, const ImageViewCreateInfo* view);
	vk::AccessFlags getAccesFlagsFromLayout(vk::ImageLayout layout);
	static inline VkPipelineStageFlags buffer_usage_to_possible_stages(VkBufferUsageFlags usage);
	static inline VkAccessFlags buffer_usage_to_possible_access(VkBufferUsageFlags usage);

}