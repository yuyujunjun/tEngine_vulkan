
#include"Device.h"
#include"CommandBufferBase.h"
#include"FenceSemaphore.h"
#include"SwapChain.h"
#include"Buffer.h"
#include"Queue.h"
#include"vma/src/vk_mem_alloc.h"
#include"utils.hpp"
//#include"TextureFormatLayout.h"
namespace tEngine {
	using namespace std;
	void Device::clearDeviceObject(){
		for (auto& sampler : samplers) {
			sampler.reset();
		}
		//transientCb.reset();
		swapChain.reset();
		transientPool.reset();
		destroyPipelineCache(pipelineCache);
		vmaDestroyAllocator(allocator);
	}
	

	
	

	uint32_t Device::getQueueId(vk::QueueFlagBits queueType)const {
		auto queueFamilyProperties = physicalDevice.getPhysicalDevice().getQueueFamilyProperties();
		return findQueueFamilyIndex(queueFamilyProperties, queueType);
	}
	Device::Device(VkDevice device, vk::Instance instance, vk::Extent2D extent):vk::Device(device),instance(instance) {

		//physical Device
		auto phyDevice = instance.enumeratePhysicalDevices().front();
		physicalDevice.SetPhysicalDevice(phyDevice);
		
		//Allocator
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
		allocatorInfo.physicalDevice = phyDevice;
		allocatorInfo.device = device;
		allocatorInfo.instance = instance;
		vmaCreateAllocator(&allocatorInfo, &allocator);
		//window
		glfwInit();
		glfwSetErrorCallback([](int error, const char* msg) {
			std::cerr << "glfw: "
				<< "(" << error << ") " << msg << std::endl;
			});


		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		gWindow = glfwCreateWindow(extent.width, extent.height, "Tgine", nullptr, nullptr);

		//surface
		VkSurfaceKHR _surface;
		VkResult err = glfwCreateWindowSurface(static_cast<VkInstance>(instance), gWindow, nullptr, &_surface);
		if (err != VK_SUCCESS)
			throw std::runtime_error("Failed to create window!");
		surface = vk::SurfaceKHR(_surface);
		
		
		
		
		
		
		vk::PipelineCacheCreateInfo info;
		pipelineCache=createPipelineCache(info);

		this->initStockSamplers();
		auto pair = vk::su::findGraphicsAndPresentQueueFamilyIndex(physicalDevice.getPhysicalDevice(), surface);
		this->getPhysicalDevice().graphicsQueuefamilyId = pair.first;
		this->getPhysicalDevice().presentQueuefamilyId = pair.second;
		auto queueFamilyProperties = physicalDevice.getPhysicalDevice().getQueueFamilyProperties();
		this->getPhysicalDevice().computeQueuefamilyId = findQueueFamilyIndex(queueFamilyProperties, vk::QueueFlagBits::eCompute);
		this->getPhysicalDevice().transferQueuefamilyId = findQueueFamilyIndex(queueFamilyProperties, vk::QueueFlagBits::eTransfer);

		this->queues[this->getPhysicalDevice().graphicsQueuefamilyId] = this->getQueue(this->getPhysicalDevice().graphicsQueuefamilyId, 0);
		this->queues[this->getPhysicalDevice().computeQueuefamilyId] = this->getQueue(this->getPhysicalDevice().computeQueuefamilyId, 0);
		this->queues[this->getPhysicalDevice().presentQueuefamilyId] = this->getQueue(this->getPhysicalDevice().presentQueuefamilyId, 0);
		this->swapChain = createSwapChain(this, surface,extent, vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst, vk::SwapchainKHR(), this->getPhysicalDevice().graphicsQueuefamilyId, this->getPhysicalDevice().presentQueuefamilyId);




		vk::CommandPoolCreateInfo poolInfo = {};
		poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eTransient| vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
		poolInfo.setQueueFamilyIndex(physicalDevice.graphicsQueuefamilyId);
		auto vkPool = createCommandPool(poolInfo);
		transientPool = std::make_shared<tCommandPool>(this,vkPool,physicalDevice.graphicsQueuefamilyId);
		fenceManager = std::make_unique<FenceManager>(this);
		semaphoreManager = std::make_unique<SemaphoreManager>(this);
	}

	
	CommandBufferHandle Device::requestTransientCommandBuffer() const { return allocateCommandBuffer(this, transientPool); };


	void Device::initStockSamplers() {
		SamplerCreateInfo info = {};
		info.max_lod = VK_LOD_CLAMP_NONE;
		info.max_anisotropy = 1.0f;

		for (unsigned i = 0; i < static_cast<unsigned>(StockSampler::Count); i++)
		{
			auto mode = static_cast<StockSampler>(i);

			switch (mode)
			{
			case StockSampler::NearestShadow:
			case StockSampler::LinearShadow:
				info.compare_enable = true;
				info.compare_op = VK_COMPARE_OP_LESS_OR_EQUAL;
				break;

			default:
				info.compare_enable = false;
				break;
			}

			switch (mode)
			{
			case StockSampler::TrilinearClamp:
			case StockSampler::TrilinearWrap:
				info.mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				break;

			default:
				info.mipmap_mode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
				break;
			}

			switch (mode)
			{
			case StockSampler::LinearClamp:
			case StockSampler::LinearWrap:
			case StockSampler::TrilinearClamp:
			case StockSampler::TrilinearWrap:
			case StockSampler::LinearShadow:
			case StockSampler::LinearYUV420P:
			case StockSampler::LinearYUV422P:
			case StockSampler::LinearYUV444P:
				info.mag_filter = VK_FILTER_LINEAR;
				info.min_filter = VK_FILTER_LINEAR;
				break;

			default:
				info.mag_filter = VK_FILTER_NEAREST;
				info.min_filter = VK_FILTER_NEAREST;
				break;
			}

			switch (mode)
			{
			default:
			case StockSampler::LinearWrap:
			case StockSampler::NearestWrap:
			case StockSampler::TrilinearWrap:
				info.address_mode_u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				info.address_mode_v = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				info.address_mode_w = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				break;

			case StockSampler::LinearClamp:
			case StockSampler::NearestClamp:
			case StockSampler::TrilinearClamp:
			case StockSampler::NearestShadow:
			case StockSampler::LinearShadow:
			case StockSampler::LinearYUV420P:
			case StockSampler::LinearYUV422P:
			case StockSampler::LinearYUV444P:
				info.address_mode_u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				info.address_mode_v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				info.address_mode_w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				break;
			}

			samplers[i] = tEngine::createSampler(this,info);
		}
	}
	SamplerHandle Device::getSampler(const StockSampler& id)const {
		return samplers[static_cast<int>(id)];
	}

	
	
	
	
	
	
	

}