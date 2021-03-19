
#define VMA_IMPLEMENTATION
#include"tDevice.h"
#include"vmaAllocation.h"
namespace tEngine {
	void FreeMemory(VmaAllocator allocator, VmaAllocation allocation) {
		vmaFreeMemory(allocator, allocation);
	}
	void Device::freeAllocation(VmaAllocation allocation)const {
		if (allocation != nullptr) {
			if (allocation->GetMappedData()) {
				//vmaUnmapMemory(allocator, allocation);
				//allocation->BlockAllocUnmap();
			}
			vmaFreeMemory(allocator, allocation);
		}
	}
	void FlushMemory(const Device* device, VmaAllocation allocation, size_t offset, size_t size) {
		vmaFlushAllocation(device->getAllocator(), allocation, offset, size);
	}
	void FlushMemory(VmaAllocator allocator, VmaAllocation allocation, size_t offset, size_t size) {
		vmaFlushAllocation(allocator, allocation, offset,size);
	}
	void* getMappedData(VmaAllocation allocation) {
		assert(allocation->GetMappedData());
		
		return allocation->GetMappedData();
	}
	uint32_t getMemoryTypeIdx(VmaAllocation allocation) {
		return allocation->GetMemoryTypeIndex();// GetMemoryTypeIndex();
	}
	size_t getAllocationSize(VmaAllocation allocation) {
		return allocation->GetSize();
	}
}