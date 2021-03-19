#pragma once
#include<vulkan/vulkan.h>
#include<memory>
namespace tEngine {
	class Device;
	class tRenderPass;
	using RenderPassHandle = std::shared_ptr<tRenderPass>;
	class tShaderInterface;
	class CommandBuffer;
	using CommandBufferHandle = std::shared_ptr <CommandBuffer>;
	class tSwapChain;
	using SwapChainHandle = std::shared_ptr<tSwapChain>;

	RenderPassHandle getSingleRenderpass(Device* device,SwapChainHandle& swapChain);

}