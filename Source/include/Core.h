#pragma once
#include"vulkan/vulkan.h"
#include<vulkan/vulkan.hpp>
#include<memory>
#include<assert.h>
#include<iostream>
namespace tEngine {
	struct Device:public vk::Device {
	public:
		Device(vk::Device device):vk::Device(device) {
	
		}
		void SetPhysicalDevice(vk::PhysicalDevice physicalDevice) {
			deviceMemoryProperties = physicalDevice.getMemoryProperties();
			deviceProperties = physicalDevice.getProperties();
		}
		const vk::PhysicalDeviceMemoryProperties& getMemoryProperties()const {
			
			return deviceMemoryProperties;
		}
		const vk::PhysicalDeviceProperties& getDeviceProperties()const {
			return deviceProperties;
		}
		vk::PhysicalDevice& getPhysicalDevice() {
			return physicalDevice;
		}
	private:
		vk::PhysicalDeviceMemoryProperties deviceMemoryProperties;
		vk::PhysicalDeviceProperties deviceProperties;
		vk::PhysicalDevice physicalDevice;
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