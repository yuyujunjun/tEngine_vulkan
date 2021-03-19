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

	RenderPassHandle getSingleRenderpass(Device* device);

}