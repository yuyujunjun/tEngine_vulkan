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
//	class ResSetBinding;
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
	class tPipeline;
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
	class BufferRangeManager;
	class MemoryBarrierSet;
	class BindingResourceInfo;
	using ResSetBinding = std::vector<BindingResourceInfo>;
	class GpuBlockBuffer;
//	void SetRange(const VmaAllocation allocation, void* data, size_t size, size_t offset = 0);
	bool imageFormat_is_supported(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatFeatureFlags required, VkImageTiling tiling);
	VkFormat default_depth_format(VkPhysicalDevice physicalDevice);
	VkFormat default_depth_stencil_format(VkPhysicalDevice physicalDevice);
	uint32_t minBufferAlignment(const tPhysicalDevice& physicalDevice);
	uint32_t alignUniformBufferAlignment(size_t size,const tPhysicalDevice& physicalDevice);
	uint32_t alignStorageBufferAlignment(size_t size,const tPhysicalDevice& physicalDevice);
	void fill_buffer_sharing_indices(const tPhysicalDevice& physicalDevice, VkBufferCreateInfo& info, uint32_t* sharing_indices);
	bool memory_type_is_host_visible(const tPhysicalDevice& physicalDevice, uint32_t type);
	void FreeMemory(VmaAllocator allocator, VmaAllocation allocation);
	void FlushMemory(const Device* device, VmaAllocation allocation, size_t offset, size_t size);
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
	vk::BufferUsageFlags descriptorTypeToBufferUsage(vk::DescriptorType type);
	BufferRangeManager createBufferFromBlock(Device* device, const GpuBlockBuffer& block, uint32_t rangeCount);
	SamplerHandle createSampler(const Device* device, const SamplerCreateInfo& sampler_info);
	BufferHandle createBuffer(const Device* device, BufferCreateInfo& createInfo, const void* initial = nullptr, CommandBufferHandle cb = nullptr);
	ImageHandle createImage(const Device* device, const ImageCreateInfo& info, std::shared_ptr<ImageAsset> initial = nullptr, CommandBufferHandle cb = nullptr);
	ImageHandle create_image_from_staging_buffer(const Device* device, const ImageCreateInfo& info, const InitialImageBuffer* buffer = nullptr, CommandBufferHandle cb = nullptr);
	InitialImageBuffer create_image_staging_buffer(const Device* device, const ImageCreateInfo& info, const ImageAsset* initial);
	InitialImageBuffer create_image_staging_buffer(const Device* device, const TextureFormatLayout& layout);
	PipelineLayoutHandle createPipelineLayout(const Device* device, std::vector<DescriptorSetLayoutHandle>& descLayouts, GpuBlockBuffer& pushConstant, vk::ShaderStageFlags shaderStage);;

}