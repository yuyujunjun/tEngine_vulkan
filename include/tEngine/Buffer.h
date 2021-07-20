#pragma once
#include"vulkan/vulkan.hpp"
#include"vmaAllocation.h"
#include<memory>
#include"GpuBlock.h"
//Only support SharingMode: Exclusive, which means have to transfer resource when using the different queues
namespace tEngine {
	class Device;
//	using const Device* = const Device*;
//	class GpuBlockBuffer;
	class CommandBuffer;
	using CommandBufferHandle = std::shared_ptr <CommandBuffer>;
	class tPhysicalDevice;
	enum class BufferDomain
	{
		Device, // Device local. Probably not visible from CPU.
		LinkedDeviceHost, // On desktop, directly mapped VRAM over PCI.
		LinkedDeviceHostPreferDevice, // Prefer device local of host visible.
		Host, // Host-only, needs to be synced to GPU. Might be device local as well on iGPUs.
		CachedHost,
		CachedCoherentHostPreferCoherent, // Aim for both cached and coherent, but prefer COHERENT
		CachedCoherentHostPreferCached, // Aim for both cached and coherent, but prefer CACHED
	};
	enum BufferMiscFlagBits
	{
		BUFFER_MISC_ZERO_INITIALIZE_BIT = 1 << 0
	};
	using BufferMiscFlags = uint32_t;
	class  BufferCreateInfo
	{
	public:
		BufferDomain domain = BufferDomain::Device;
		VkDeviceSize size = 0;
		VkBufferUsageFlags usage = 0;
		BufferMiscFlags misc = 0;
	};
	class tBuffer {
	public:
		
		tBuffer(const Device* device,vk::Buffer buffer,const VmaAllocation& alloc,const BufferCreateInfo& bufferInfo):device(device), bufferInfo(bufferInfo),alloc(alloc), vkHandle(buffer){
		
		}
		~tBuffer();
		const vk::Buffer& getVkHandle()const {
			return vkHandle;
		}
		const VmaAllocation& getAllocation()const {
			return alloc;
		}
		const BufferCreateInfo& getCreateInfo()const {
		//	bufferInfo.size = alloc->GetSize();
			return bufferInfo;
			
		}
		void setRange(const void* data, size_t offset, size_t size);
		const vk::DeviceSize getSize()const;
		void Flush();
	private:
		vk::Buffer vkHandle;
		const Device* device;
		VmaAllocation alloc;
		BufferCreateInfo bufferInfo;
		vk::MemoryPropertyFlags memoryProperty;
	
	};
	using BufferHandle = std::shared_ptr<tBuffer>;
	//offset update must align the minAlignmentOffset(each update must be a buffer update)
	//Manage buffer range usage
	class BufferRangeManager {
	public:
		BufferRangeManager(const BufferHandle buffer, size_t rangeSize = -1, size_t initialOffset = 0) :handle(buffer), rangeSize(rangeSize), initialOff(initialOffset) {
			if (rangeSize == -1)rangeSize = buffer->getSize();
		}
		size_t SetRangeIncremental(void* data);
		//return current offset
		void SetRangeNoLock(void* data);
		const  BufferHandle& buffer() const {
			return handle;
		}
		BufferHandle& buffer() {
			return handle;
		}
		//request new buffer range
		void NextRangenoLock() {
			offset = offset + rangeSize * 2 <= handle->getSize() ? offset + rangeSize : initialOff;
		}
		size_t getOffset()const {
			return offset;
		}
		void removeBuffer() { handle = nullptr; }
	private:

		size_t rangeSize = 0;
		//std::mutex mtx;
		size_t initialOff = 0;
		size_t offset = 0;
		BufferHandle handle;
	};

	
	inline  bool operator==(const BufferHandle& a,const BufferHandle& b) {
		return a->getVkHandle() == b->getVkHandle();
	}
	inline  bool operator != (const BufferHandle& a, const BufferHandle& b) {
		return !(a->getVkHandle() == b->getVkHandle());
	}
	void updateBufferUsingStageBuffer(const Device* device, BufferHandle& buffer, CommandBufferHandle& cb, const void* data, size_t size, size_t offset = 0);
	void fillBufferUsingStageBuffer(const Device* device, BufferHandle& handle, CommandBufferHandle& cb, uint32_t zero, size_t size, size_t offset = 0);
	void find_memory_type(BufferDomain domain, VmaAllocationCreateInfo& info);
	uint32_t minBufferAlignment(const tPhysicalDevice& physicalDevice);
	uint32_t alignUniformBufferAlignment(size_t size, const tPhysicalDevice& physicalDevice);
	uint32_t alignStorageBufferAlignment(size_t size, const tPhysicalDevice& physicalDevice);
	void fill_buffer_sharing_indices(const tPhysicalDevice& physicalDevice, VkBufferCreateInfo& info, uint32_t* sharing_indices);
	bool memory_type_is_host_visible(const tPhysicalDevice& physicalDevice, uint32_t type);
	void FreeMemory(VmaAllocator allocator, VmaAllocation allocation);
	void FlushMemory(const Device* device, VmaAllocation allocation, size_t offset, size_t size);
	void FlushMemory(VmaAllocator allocator, VmaAllocation allocation, size_t offset, size_t size);
	void* getMappedData(VmaAllocation allocation);
	uint32_t getMemoryTypeIdx(VmaAllocation allocation);
	size_t getAllocationSize(VmaAllocation allocation);
	static inline VkPipelineStageFlags buffer_usage_to_possible_stages(VkBufferUsageFlags usage);
	static inline VkAccessFlags buffer_usage_to_possible_access(VkBufferUsageFlags usage);
	std::shared_ptr<BufferRangeManager> createBufferFromBlock(const Device* device, const GpuBlockBuffer& block, uint32_t rangeCount);
	BufferHandle createBuffer(const Device* device, BufferCreateInfo& createInfo, const void* initial = nullptr, CommandBufferHandle cb = nullptr);
	

}