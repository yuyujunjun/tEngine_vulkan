#pragma once
#include<vector>
namespace tEngine {
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
		const GpuBlockMember& operator[](int id)const {
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
}