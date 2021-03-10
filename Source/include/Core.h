#pragma once
#include"vulkan/vulkan.h"
#include<vulkan/vulkan.hpp>
#include<memory>
#include<assert.h>
#include<iostream>

namespace tEngine {
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
	};
	struct GpuBlockBuffer {
		GpuBlockMember& operator[](int id)noexcept {
			assert(id < data.size() && "id exceed blocks");
			return data[id];
		}
		uint32_t ByteSize() {
			if (data.empty())return 0;
			return data.back().size + data.back().offset;
		}
		char* RequestMemory() {
			if (ByteSize() == 0)return nullptr;
			return new char[ByteSize()];
		}
		uint32_t size() const {
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
			properties = physicalDevice.getProperties();
		}
		const vk::PhysicalDeviceMemoryProperties& getMemoryProperties()const {

			return memoryProperties;
		}
		const vk::PhysicalDeviceProperties& getDeviceProperties()const {
			return properties;
		}
		vk::PhysicalDevice& getPhysicalDevice() {
			return physicalDevice;
		}
		vk::PhysicalDevice& operator()() {
			return physicalDevice;
		}
		uint32_t graphicsQueuefamilyId=-1;
		uint32_t presentQueuefamilyId=-1;
		uint32_t computeQueuefamilyId=-1;
		vk::PhysicalDeviceMemoryProperties memoryProperties;
		vk::PhysicalDeviceProperties properties;
		vk::PhysicalDevice physicalDevice;
	};
	struct Device:public vk::Device {
	public:
		Device(vk::Device device):vk::Device(device) {
	
		}

		tPhysicalDevice physicalDevice;
		

	};
	using weakDevice = std::weak_ptr<Device>;
	using sharedDevice = std::shared_ptr<Device>;
	/// <summary>INTERNAL. Disable the Copy Constructor and the Copy Assignment Operator of the type</summary>
#define DECLARE_NO_COPY_SEMANTICS(TYPE) \
	TYPE(const TYPE&) = delete; \
	const TYPE& operator=(const TYPE&) = delete;
	
	inline  void reportDestroyedAfterDevice() { assert(false && "Attempted to destroy object after its corresponding device"); }
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
	void Log(T info, LogLevel level=LogLevel::Information) {
		Logger::Log(info, level);
	}
	
}