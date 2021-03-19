#pragma once
#include"vulkan/vulkan.h"
#include<vulkan/vulkan.hpp>
#include<memory>
#include<assert.h>
#include<iostream>
#include<mutex>


namespace tEngine {



	class Device;
	class tBuffer;
	using BufferHandle = std::shared_ptr<tBuffer>;
	class tImage;
	using ImageHandle = std::shared_ptr<tImage>;// ::SharedPtr;
	class tSampler;
	using SamplerHandle = std::shared_ptr<tSampler>;
	class tImageView;
	using ImageviewHandle = std::shared_ptr<tImageView>;
	class CommandBuffer;
	using CommandBufferHandle = std::shared_ptr <CommandBuffer>;
	class tSwapChain;
	using SwapChainHandle = std::shared_ptr<tSwapChain>;
	class tFrameBuffer;
	using FrameBufferHandle = std::shared_ptr<tFrameBuffer>;
	class tPipelineLayout;
	using PipelineLayoutHandle = std::shared_ptr<tPipelineLayout>;
	class tShaderInterface;
	class tPipeline;
	using PipelineHandle = std::shared_ptr<tPipeline>;
	class tCommandPool;
	using CommandPoolHandle = std::shared_ptr<tCommandPool>;
	class tFence;
	using FenceHandle = std::shared_ptr<tFence>;
	class tPhysicalDevice;
	
}