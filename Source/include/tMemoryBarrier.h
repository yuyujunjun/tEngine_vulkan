#pragma once
#include"Core.h"
#include"tResource.h"
namespace tEngine {
	
	struct tImageMemoryBarrier
	{
		using AccessFlags=vk::AccessFlags;
		using ImageLayout = vk::ImageLayout;
	private:
		vk::AccessFlags srcAccessMask; //!< Bitmask of vk::AccessFlagBits specifying a source access mask.
		vk::AccessFlags dstAccessMask; //!< Bitmask of vk::AccessFlagBits specifying a destination access mask.
		vk::ImageLayout oldLayout; //!< Old layout in an image layout transition.
		vk::ImageLayout newLayout; //!< New layout in an image layout transition.
		uint32_t srcQueueFamilyIndex; //!< Source queue family for a queue family ownership transfer.
		uint32_t dstQueueFamilyIndex; //!< Destination queue family for a queue family ownership transfer
		tImage::SharedPtr image; //!< Handle to the image affected by this barrier
		vk::ImageSubresourceRange subresourceRange; //!< Describes the image subresource range within image that is affected by this barrier
		//vk::ImageMemoryBarrier imageMemoryBarrier;
	public:
		/// <summary>Constructor. All flags are zero initialized, and family indexes set to -1.</summary>
		tImageMemoryBarrier()
			: srcAccessMask(vk::AccessFlags(0)), dstAccessMask(vk::AccessFlags(0)), oldLayout(vk::ImageLayout::eUndefined), newLayout(vk::ImageLayout::eUndefined),
			srcQueueFamilyIndex(static_cast<uint32_t>(-1)), dstQueueFamilyIndex(static_cast<uint32_t>(-1))
		{}

		/// <summary>Constructor. Set all individual elements.</summary>
		/// <param name="srcMask">Bitmask of vk::AccessFlagBits specifying a source access mask.</param>
		/// <param name="dstMask">Bitmask of vk::AccessFlagBits specifying a destination access mask.</param>
		/// <param name="image">Handle to the image affected by this barrier</param>
		/// <param name="subresourceRange">Describes the image subresource range within image that is affected by this barrier</param>
		/// <param name="oldLayout">Old layout in an image layout transition.</param>
		/// <param name="newLayout">New layout in an image layout transition.</param>
		/// <param name="srcQueueFamilyIndex">Source queue family for a queue family ownership transfer.</param>
		/// <param name="dstQueueFamilyIndex">Destination queue family for a queue family ownership transfer</param>
		tImageMemoryBarrier(vk::AccessFlags srcMask, vk::AccessFlags dstMask, const tImage::SharedPtr& image, const vk::ImageSubresourceRange& subresourceRange, vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout, uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex)
			: srcAccessMask(srcMask), dstAccessMask(dstMask), oldLayout(oldLayout), newLayout(newLayout), srcQueueFamilyIndex(srcQueueFamilyIndex),
			dstQueueFamilyIndex(dstQueueFamilyIndex), image(image), subresourceRange(subresourceRange)
		{}

		/// <summary>Get srcAccessMask</summary>
		/// <returns>AcceAn AccessFlags structure specifying the source memory barrier access flagsssFlags</returns>
		inline const vk::AccessFlags& getSrcAccessMask() const { return srcAccessMask; }

		/// <summary>Set srcAccessMask</summary>
		/// <param name="inSrcAccessMask">An AccessFlags structure specifying the source memory barrier access flags</param>
		inline void setSrcAccessMask(const AccessFlags& inSrcAccessMask) { this->srcAccessMask = inSrcAccessMask; }

		/// <summary>Get dstAccessMask</summary>
		/// <returns>An AccessFlags structure specifying the destination memory barrier access flags</returns>
		inline const AccessFlags& getDstAccessMask() const { return dstAccessMask; }

		/// <summary>Set dstAccessMask</summary>
		/// <param name="inDstAccessMask">An AccessFlags structure specifying the destination memory barrier access flags</param>
		inline void setDstAccessMask(const AccessFlags& inDstAccessMask) { this->dstAccessMask = inDstAccessMask; }

		/// <summary>Get the old image layout of the image associated with the memory barrier</summary>
		/// <returns>The old image layout of the image associated with the memory barrier</returns>
		inline const ImageLayout& getOldLayout() const { return oldLayout; }
		/// <summary>Set old image layout</summary>
		/// <param name="inOldLayout">The old image layout of the image associated memory barrier</param>
		inline void setOldLayout(const ImageLayout& inOldLayout) { this->oldLayout = inOldLayout; }

		/// <summary>Get the new image layout of the image associated with the memory barrier</summary>
		/// <returns>The new image layout of the image associated with the memory barrier</returns>
		inline const ImageLayout& getNewLayout() const { return newLayout; }

		/// <summary>Set new image layout</summary>
		/// <param name="inNewLayout">The new image layout of the image associated memory barrier</param>
		inline void setNewLayout(const ImageLayout& inNewLayout) { this->newLayout = inNewLayout; }

		/// <summary>Get the source queue family index for the image associated with the memory barrier</summary>
		/// <returns>The source queue family index of the image associated with the memory barrier</returns>
		inline uint32_t getSrcQueueFamilyIndex() const { return srcQueueFamilyIndex; }

		/// <summary>Set the source queue family index</summary>
		/// <param name="inSrcQueueFamilyIndex">The source queue family index of the image associated with the memory barrier</param>
		inline void setSrcQueueFamilyIndex(uint32_t inSrcQueueFamilyIndex) { this->srcQueueFamilyIndex = inSrcQueueFamilyIndex; }

		/// <summary>Get the destination queue family index for the image associated with the memory barrier</summary>
		/// <returns>The destination queue family index of the image associated with the memory barrier</returns>
		inline uint32_t getDstQueueFamilyIndex() const { return dstQueueFamilyIndex; }

		/// <summary>Set the destination queue family index</summary>
		/// <param name="inDstQueueFamilyIndex">The destination queue family index of the image associated with the memory barrier</param>
		inline void setDstQueueFamilyIndex(uint32_t inDstQueueFamilyIndex) { this->dstQueueFamilyIndex = inDstQueueFamilyIndex; }

		/// <summary>Get Image</summary>
		/// <returns>The vk::Image associated with the memory barrier</returns>
		inline tImage* getImage() const { return image.get(); }

		/// <summary>Set Image</summary>
		/// <param name="inImage">The vk::Image associated with the memory barrier</param>
		inline void setImage(const tImage::SharedPtr& inImage) { this->image = inImage; }

		/// <summary>Get the subresource range of the image associated with the memory barrier</summary>
		/// <returns>The subresource range of the image associated with the memory barrier</returns>
		inline const vk::ImageSubresourceRange& getSubresourceRange() const { return subresourceRange; }

		/// <summary>Set the subresource range of the image associated with the memory barrier</summary>
		/// <param name="inSubresourceRange">The subresource range of the image associated with the memory barrier</param>
		inline void setSubresourceRange(const vk::ImageSubresourceRange& inSubresourceRange) { this->subresourceRange = inSubresourceRange; }
	};

	/// <summary>A Buffer memory barrier used only for memory accesses involving a specific range of the specified
/// buffer object. It is also used to transfer ownership of an buffer range from one queue family to another.</summary>
	struct tBufferMemoryBarrier
	{
		using AccessFlags = vk::AccessFlags;

	private:
		vk::AccessFlags srcAccessMask; //!< Bitmask of vk::AccessFlagBits specifying a source access mask.
		vk::AccessFlags dstAccessMask; //!< Bitmask of vk::AccessFlagBits specifying a destination access mask.
		tBuffer::SharedPtr buffer; //!< Handle to the buffer whose backing memory is affected by the barrier.
		uint32_t offset; //!< Offset in bytes into the backing memory for buffer. This is relative to the base offset as bound to the buffer
		uint32_t size; //!< Size in bytes of the affected area of backing memory for buffer, or VK_WHOLE_SIZE to use the range from offset to the end of the buffer.

	public:
		/// <summary>Constructor, zero initialization</summary>
		tBufferMemoryBarrier() : srcAccessMask(vk::AccessFlags(0)), dstAccessMask(vk::AccessFlags(0)),offset(0),size(0) {}

		/// <summary>Constructor, individual elements</summary>
		/// <param name="srcAccessMask">Bitmask of vk::AccessFlagBits specifying a source access mask.</param>
		/// <param name="dstAccessMask">Bitmask of vk::AccessFlagBits specifying a destination access mask.</param>
		/// <param name="buffer">Handle to the buffer whose backing memory is affected by the barrier.</param>
		/// <param name="offset">Offset in bytes into the backing memory for buffer. This is relative to the base offset as bound to the buffer</param>
		/// <param name="size">Size in bytes of the affected area of backing memory for buffer, or VK_WHOLE_SIZE to use the range from offset to the end of the buffer.</param>
		tBufferMemoryBarrier(vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, tBuffer::SharedPtr buffer, uint32_t offset, uint32_t size)
			: srcAccessMask(srcAccessMask), dstAccessMask(dstAccessMask), buffer(buffer), offset(offset), size(size)
		{}

		/// <summary>Get srcAccessMask</summary>
		/// <returns>An AccessFlags structure specifying the source memory barrier access flags</returns>
		inline const AccessFlags& getSrcAccessMask() const { return srcAccessMask; }

		/// <summary>Set srcAccessMask</summary>
		/// <param name="inSrcAccessMask">An AccessFlags structure specifying the source memory barrier access flags</param>
		inline void setSrcAccessMask(const AccessFlags& inSrcAccessMask) { this->srcAccessMask = inSrcAccessMask; }

		/// <summary>Get dstAccessMask</summary>
		/// <returns>An AccessFlags structure specifying the destination memory barrier access flags</returns>
		inline const AccessFlags& getDstAccessMask() const { return dstAccessMask; }

		/// <summary>Set dstAccessMask</summary>
		/// <param name="inDstAccessMask">An AccessFlags structure specifying the destination memory barrier access flags</param>
		inline void setDstAccessMask(const AccessFlags& inDstAccessMask) { this->dstAccessMask = inDstAccessMask; }

		/// <summary>Get Buffer associated with the memory barrier</summary>
		/// <returns>The vk::Buffer associated with the memory barrier</returns>
		inline const tBuffer::SharedPtr& getBuffer() const { return buffer; }

		/// <summary>Set buffer associated with the memory barrier</summary>
		/// <param name="inBuffer">The vk::Buffer associated with the memory barrier</param>
		inline void setBuffer(const tBuffer::SharedPtr& inBuffer) { this->buffer = inBuffer; }

		/// <summary>Get size corresponding to the slice of the Buffer associated with the memory barrier</summary>
		/// <returns>The size of the range of the vk::Buffer associated with the memory barrier</returns>
		inline uint32_t getSize() const { return size; }

		/// <summary>Set the size of the slice of the buffer associated with the memory barrier</summary>
		/// <param name="inSize">The size of the slice of the vk::Buffer associated with the memory barrier</param>
		inline void setSize(uint32_t inSize) { this->size = inSize; }

		/// <summary>Get the offset into the Buffer associated with the memory barrier</summary>
		/// <returns>The offset into vk::Buffer associated with the memory barrier</returns>
		inline uint32_t getOffset() const { return offset; }

		/// <summary>Set the offset into the buffer associated with the memory barrier</summary>
		/// <param name="inOffset">The offset into the vk::Buffer associated with the memory barrier</param>
		inline void setOffset(uint32_t inOffset) { this->offset = inOffset; }
	};
	struct MemoryBarrierSet
	{
	private:
		MemoryBarrierSet(const MemoryBarrierSet&) = delete; // deleted
		MemoryBarrierSet& operator=(const MemoryBarrierSet&) = delete; // deleted

		std::vector<vk::MemoryBarrier> memBarriers;
		std::vector<tImageMemoryBarrier> imageBarriers;
		std::vector<tBufferMemoryBarrier> bufferBarriers;

	public:
		/// <summary>Constructor. Empty barrier</summary>
		MemoryBarrierSet() = default;

		/// <summary>Clear this object of all barriers</summary>
		/// <returns>MemoryBarrierSet&</returns>
		MemoryBarrierSet& clearAllBarriers()
		{
			memBarriers.clear();
			imageBarriers.clear();
			bufferBarriers.clear();
			return *this;
		}

		/// <summary>Clear this object of all Memory barriers</summary>
		/// <returns>MemoryBarrierSet&</returns>
		MemoryBarrierSet& clearAllMemoryBarriers()
		{
			memBarriers.clear();
			return *this;
		}

		/// <summary>Clear this object of all Buffer barriers</summary>
		/// <returns>MemoryBarrierSet&</returns>
		MemoryBarrierSet& clearAllBufferRangeBarriers()
		{
			bufferBarriers.clear();
			return *this;
		}

		/// <summary>Clear this object of all Image barriers</summary>
		/// <returns>MemoryBarrierSet&</returns>
		MemoryBarrierSet& clearAllImageAreaBarriers()
		{
			imageBarriers.clear();
			return *this;
		}

		/// <summary>Add a generic Memory barrier.</summary>
		/// <param name="barrier">The barrier to add</param>
		/// <returns>This object (allow chained calls)</returns>
		MemoryBarrierSet& addBarrier(vk::MemoryBarrier barrier)
		{
			memBarriers.emplace_back(barrier);
			return *this;
		}

		/// <summary>Add a Buffer Range barrier, signifying that operations on a part of a buffer
		/// must complete before other operations on that part of the buffer execute.</summary>
		/// <param name="barrier">The barrier to add</param>
		/// <returns>This object (allow chained calls)</returns>
		MemoryBarrierSet& addBarrier(const tBufferMemoryBarrier& barrier)
		{
			bufferBarriers.emplace_back(barrier);
			return *this;
		}

		/// <summary>Add a Buffer Range barrier, signifying that operations on a part of an Image
		/// must complete before other operations on that part of the Image execute.</summary>
		/// <param name="barrier">The barrier to add</param>
		/// <returns>This object (allow chained calls)</returns>
		MemoryBarrierSet& addBarrier(const tImageMemoryBarrier& barrier)
		{
			imageBarriers.emplace_back(barrier);
			return *this;
		}

		/// <summary>Get an array of the tMemoryBarrier object of this set.</summary>
		/// <returns>All tMemoryBarrier objects that this object contains</returns>
		const std::vector<vk::MemoryBarrier>& getMemoryBarriers() const { return this->memBarriers; }

		/// <summary>Get an array of the Image Barriers of this set.</summary>
		/// <returns>All tMemoryBarrier objects that this object contains</returns>
		const std::vector<tImageMemoryBarrier>& getImageBarriers() const { return this->imageBarriers; }

		/// <summary>Get an array of the Buffer Barriers of this set.</summary>
		/// <returns>All tMemoryBarrier objects that this object contains</returns>
		const std::vector<tBufferMemoryBarrier>& getBufferBarriers() const { return this->bufferBarriers; }
	};
}