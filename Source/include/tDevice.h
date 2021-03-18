#pragma once
#include"Tgine.h"
#include"../PriorityAllocator.h"
#include<unordered_map>
#include"utils.hpp"
#define VULKAN_DEBUG DEBUG
namespace tEngine {
	

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

	struct Device :public vk::Device{
	public:
		friend class tEngineContext;
		Device(VkDevice device,  vk::Instance instance,vk::Extent2D extent);

		void Device::initStockSamplers();
		void freeAllocation(VmaAllocation allocation)const;
		SamplerHandle Device::createSampler(const SamplerCreateInfo& sampler_info);
		SamplerHandle Device::getSampler(const StockSampler& sampler)const;
		BufferHandle createBuffer(const BufferCreateInfo& createInfo, const void* initial = nullptr, CommandBufferHandle cb = nullptr)const;
		ImageHandle createImage(const ImageCreateInfo& info, std::shared_ptr<ImageAsset> initial = nullptr, CommandBufferHandle cb = nullptr)const;
		ImageHandle create_image_from_staging_buffer(const ImageCreateInfo& info, const InitialImageBuffer* buffer = nullptr, CommandBufferHandle cb = nullptr)const;
		InitialImageBuffer create_image_staging_buffer(const ImageCreateInfo& info, const ImageAsset* initial)const;
		InitialImageBuffer create_image_staging_buffer(const TextureFormatLayout& layout)const;
		PipelineLayoutHandle createPipelineLayout(std::vector<DescriptorSetLayoutHandle>& descLayouts, GpuBlockBuffer& pushConstant, vk::ShaderStageFlags shaderStage);;
		const tPhysicalDevice& getPhysicalDevice()const { return physicalDevice; }
		tPhysicalDevice& getPhysicalDevice() { return physicalDevice; }
		const VmaAllocator& getAllocator()const { return allocator; ; }
		const vk::PipelineCache& getPipelineCache()const { return pipelineCache; }
		CommandBufferHandle requestTransientCommandBuffer() const ;
		
		void clearDeviceObject();
		vk::Queue requestQueue(uint32_t familyIndx)const {
			return queues.at(familyIndx);
		}
		vk::Queue requestQueue(vk::QueueFlagBits flag)const {
			return queues.at(getQueueId(flag));
		}
		GLFWwindow* getWindow() {
			return gWindow;
		}
		SwapChainHandle swapChain;
		std::unordered_map<uint32_t, vk::Queue> queues;
		uint32_t getQueueId(vk::QueueFlagBits queueType)const;
		const std::unique_ptr<FenceManager>& getFenceManager()const { return fenceManager; }
		const std::unique_ptr<SemaphoreManager>& getSemaphoreManager()const { return semaphoreManager; }
		//vk::Device* operator->()const { return const_cast<vk::Device*>( &vkDevice); }
		//operator vk::Device() { return vkDevice; }
	private:
		vk::DebugUtilsMessengerEXT debugUtilsMessenger;
		vk::Instance instance;
		GLFWwindow* gWindow;
		vk::SurfaceKHR surface;
		std::unique_ptr<FenceManager> fenceManager;
		std::unique_ptr<SemaphoreManager> semaphoreManager;

		vk::PipelineCache pipelineCache;
		tPhysicalDevice physicalDevice;
		VmaAllocator allocator;
		CommandPoolHandle transientPool;
		std::array<SamplerHandle, static_cast<unsigned>(StockSampler::Count)> samplers;
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
		template<typename Attribute>
		static void Log(const Attribute& info,LogLevel level) {
			std::stringstream temp;
			temp<< info;
			
			LogStyle(temp.str(), level);
			
		}
	private:
		static void LogStyle(const std::string& s,LogLevel level);
		static std::stringstream stream;
	};
	


	template<typename Attribute>
	void LOGI(Attribute args) {
		Logger::Log(args,LogLevel::Information);
	}
	template<typename Attribute, typename ...Args>
	void LOGI(Attribute value, Args... args) {
		LOGI(value);
		LOGI(args...);
	}
	template<typename Attribute>
	void Log(Attribute info, LogLevel level=LogLevel::Information) {
		Logger::Log(info, level);
	}

}