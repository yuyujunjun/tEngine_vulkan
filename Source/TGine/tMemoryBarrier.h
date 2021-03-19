#pragma once
#include"tGine.h"
namespace tEngine {
	class MemoryBarrierSet
	{
	private:
		MemoryBarrierSet(const MemoryBarrierSet&) = delete; // deleted
		MemoryBarrierSet& operator=(const MemoryBarrierSet&) = delete; // deleted

		std::vector<vk::MemoryBarrier> memBarriers;
		std::vector<vk::ImageMemoryBarrier> imageBarriers;
		std::vector<vk::BufferMemoryBarrier> bufferBarriers;

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
		MemoryBarrierSet& addBarrier(const vk::BufferMemoryBarrier& barrier)
		{
			bufferBarriers.emplace_back(barrier);
			return *this;
		}

		/// <summary>Add a Buffer Range barrier, signifying that operations on a part of an Image
		/// must complete before other operations on that part of the Image execute.</summary>
		/// <param name="barrier">The barrier to add</param>
		/// <returns>This object (allow chained calls)</returns>
		MemoryBarrierSet& addBarrier(const vk::ImageMemoryBarrier& barrier)
		{
			imageBarriers.emplace_back(barrier);
			return *this;
		}

		/// <summary>Get an array of the tMemoryBarrier object of this set.</summary>
		/// <returns>All tMemoryBarrier objects that this object contains</returns>
		const std::vector<vk::MemoryBarrier>& getMemoryBarriers() const { return this->memBarriers; }

		/// <summary>Get an array of the Image Barriers of this set.</summary>
		/// <returns>All tMemoryBarrier objects that this object contains</returns>
		const std::vector<vk::ImageMemoryBarrier>& getImageBarriers() const { return this->imageBarriers; }

		/// <summary>Get an array of the Buffer Barriers of this set.</summary>
		/// <returns>All tMemoryBarrier objects that this object contains</returns>
		const std::vector<vk::BufferMemoryBarrier>& getBufferBarriers() const { return this->bufferBarriers; }
	};
}