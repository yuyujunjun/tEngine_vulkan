#pragma once
#include<vulkan/vulkan.hpp>
#include<memory>
#include"RingPool.h"
namespace tEngine {
	class Device;
	class tRenderPass;
	using RenderPassHandle = std::shared_ptr<tRenderPass>;
	class tShaderInterface;
	class CommandBuffer;
	using CommandBufferHandle = std::shared_ptr <CommandBuffer>;
	class tSwapChain;
	using SwapChainHandle = std::shared_ptr<tSwapChain>;


	class ForwardRenderPass {
	public:
		ForwardRenderPass()  {}
		struct uniqueAttribute {
			vk::Format format;
			vk::ImageLayout finalLayout;
			bool operator==(const uniqueAttribute& att) {
				return format == att.format&&finalLayout==att.finalLayout;
			}
		};
		RingPool<tRenderPass, uniqueAttribute,16> forwardRenderPassPool;
		RenderPassHandle requestRenderPass(const Device* device,vk::Format format, vk::ImageLayout finalLayout);
	private:

	};
	RenderPassHandle getSingleRenderpass(Device* device, vk::Format format,const vk::ImageLayout& layout);
	void getForwardRenderPass(const ForwardRenderPass::uniqueAttribute& att, const Device* device, RenderPassHandle& renderPass);
	RenderPassHandle getUIRenderpass(Device* device, vk::Format format);
	RenderPassHandle getShadowMapPass(Device* device, vk::Format format, vk::Format depthFormat);
	RenderPassHandle getCollectShadowPass(Device* device, vk::Format format);
}