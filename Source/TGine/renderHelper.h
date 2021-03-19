#pragma once
#include"Tgine.h"
namespace tEngine {
	RenderPassHandle getSingleRenderpass(Device* device);
	void flushDescriptorSet(const CommandBufferHandle& cb, tShaderInterface& state);
	void flushGraphicsPipeline(const CommandBufferHandle& cb, tShaderInterface& state, tRenderPass* renderPass, uint32_t subpass);
	void flushComptuePipeline(const CommandBufferHandle& cb, tShaderInterface& state);
	void flushGraphicsShaderState(tShaderInterface& state, CommandBufferHandle& cb, tRenderPass* renderPass, uint32_t subpass);
	void flushComputeShaderState(tShaderInterface& state, CommandBufferHandle& cb);
	void collectDescriptorSets(std::vector<DescriptorSetHandle>& bindedSets, std::vector<uint32_t>& offsets,
		const ResSetBinding& setBindings, const DescSetAllocHandle& setAllocator);
}