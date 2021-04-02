#include"Pipeline.h"
#include"FrameBuffer.h"
#include"Shader.h"
#include"ShaderInterface.h"
#include"SimpleGeometry.h"
#include"Device.h"
#include"Log.h"
#include"DescriptorPool.h"
#include"GraphicsState.h"
namespace tEngine {
	GraphicsPipelineCreateInfo getDefaultPipelineCreateInfo(tShaderInterface* shader, const GraphicsState& state, const tRenderPass* renderPass, uint32_t subpass, const tFrameBuffer* frameBuffer) {
		// Shader
		GraphicsPipelineCreateInfo createInfo;
		for (uint32_t i = 0; i < shader->getShader()->getShaderCount(); ++i) {
			const auto& sha = shader->getShader();
			createInfo.addShader(sha->getShaderModule(i), sha->getShaderStage(i));
		}
		//Blend

		vk::PipelineColorBlendAttachmentState blendState = {};
		blendState.blendEnable = false;
		blendState.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
		for (uint32_t i = 0; i < renderPass->getSubpass(subpass)->getColorAttachmentCount(); ++i) {
			createInfo.coloBlendState.Attachments.emplace_back(blendState);
		}
		//DepthStencil

		createInfo.depthStencilState.depthTestEnable = state.depthTestEnable;
		createInfo.depthStencilState.depthWriteEnable = state.depthWriteEnable;
		
		//createInfo.depthStencilState.depthCompareOp = vk::CompareOp::eAlways;
		//dynamicState
		createInfo.dynamicState.dynamicStates.emplace_back(vk::DynamicState::eViewport);

		//Layout
		createInfo.layout = shader->getShader()->getPipelineLayout()->getVkHandle();

		///multiSampleState
		createInfo.multisampleState.sampleShadingEnable = false;
		//createInfo.multisampleState.SampleMask = 0xFFFFFFFF;
		//rasterizationState
		//createInfo.rasterizationState.cullMode = vk::CullModeFlagBits::eNone;
		createInfo.rasterizationState.depthBiasEnable = state.depthBias.depthBiasEnable;
		createInfo.rasterizationState.depthClampEnable = state.depthClampEnable;
		createInfo.rasterizationState.depthBiasClamp = state.depthBias.depthBiasClamp;
		createInfo.rasterizationState.depthBiasSlopeFactor = state.depthBias.depthBiasSlopeFactor;
		
		createInfo.rasterizationState.depthBiasConstantFactor = state.depthBias.depthBiasConstantFactor;
		
		//createInfo.rasterizationState.de
		//createInfo.rasterizationState.polygonMode = vk::PolygonMode::eFill;

		createInfo.renderPass = renderPass->getVkHandle();
		createInfo.subpass = subpass;

		//tessellstion
		//createInfo.tessellation
		//Topology
		//createInfo.topology.primitiveRestartEnable = false;
		createInfo.topology.topolygy = vk::PrimitiveTopology::eTriangleList;


		//VertexAttribute
		createInfo.vertexAttribute = {
			{ 0, 0, vk::Format::eR32G32B32Sfloat, 0 } ,
			{ 1, 0, vk::Format::eR32G32B32Sfloat, 12 } ,
			{ 2,0,vk::Format::eR32G32Sfloat,24 } ,
			{ 3,0,vk::Format::eR32G32B32A32Sfloat,32 } };

		createInfo.vertexInput = { {0,sizeof(Vertex),vk::VertexInputRate::eVertex} };


		createInfo.viewport.viewPorts = { frameBuffer->getViewPort() };
		createInfo.viewport.scissors = { frameBuffer->getRenderArea() };

		return createInfo;

	}
	vk::Pipeline GraphicsPipelineCreateInfo::createPipeline(const Device* device)const {
		vk::GraphicsPipelineCreateInfo info = {};

		info.setLayout(layout);
		auto cbs = coloBlendState.getCreateInfo();
		info.setPColorBlendState(&cbs);
		auto dss = depthStencilState.getCreateInfo();
		info.setPDepthStencilState(&dss);
		auto ds = dynamicState.getCreateInfo();
		info.setPDynamicState(&ds);
		vk::PipelineVertexInputStateCreateInfo VertexInputState;
		VertexInputState.setVertexAttributeDescriptions(vertexAttribute);
		VertexInputState.setVertexBindingDescriptions(vertexInput);
		info.setPVertexInputState(&VertexInputState);
		auto t = topology.getCreateInfo();
		info.setPInputAssemblyState(&t);
		auto multisample = multisampleState.getCreateInfo();
		info.setPMultisampleState(&multisample);
		auto rasterization = rasterizationState.getCreateInfo();
		info.setPRasterizationState(&rasterization);
		info.setStages(Stages);
		vk::PipelineTessellationStateCreateInfo tessellationState;
		tessellationState.setPatchControlPoints(tessellation.patchControlPoints);
		info.setPTessellationState(&tessellationState);
		vk::PipelineViewportStateCreateInfo view;
		view.setScissors(viewport.scissors);
		view.setViewports(viewport.viewPorts);
		info.setPViewportState(&view);
		info.setRenderPass(renderPass);
		info.setSubpass(subpass);
		info.setBasePipelineIndex(-1);
		auto result = device->createGraphicsPipeline(device->getPipelineCache(), info);
		assert(result.result == vk::Result::eSuccess);
		return result.value;
	}
	vk::Pipeline ComputePipelineCreateInfo::createPipeline(const Device* device)const {
		vk::ComputePipelineCreateInfo info = {};
		info.setLayout(layout);
		info.setStage(Stage);
		auto result = device->createComputePipeline(device->getPipelineCache(), info);
		assert(result.result == vk::Result::eSuccess);
		return result.value;

	}
	tPipeline::~tPipeline() {
		if (vkpipeline) {
			//	assert(false);
			device->destroyPipeline(vkpipeline);

		}
	}
	tPipelineLayout::~tPipelineLayout() {
		if (vkLayout) {
			device->destroyPipelineLayout(vkLayout);
			//		assert(false);
			vkLayout = vk::PipelineLayout();
		}
	}
	PipelineHandle tPipelineLayout::requestGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) {
		auto pipeline = graphic_pipelinePool.request(createInfo);
		if (pipeline == nullptr) {
			LOGD(LogLevel::Performance, "createPipeline");
			auto vkPipeline = createInfo.createPipeline(device);
			pipeline = graphic_pipelinePool.allocate(createInfo, device, vk::PipelineBindPoint::eGraphics, vkPipeline, this);

		}
		return pipeline;
	}
	PipelineHandle tPipelineLayout::requestComputePipeline(const ComputePipelineCreateInfo& createInfo) {
		auto pipeline = compute_pipelinePool.request(createInfo);
		if (pipeline == nullptr) {
			auto vkPipeline = createInfo.createPipeline(device);
			pipeline = compute_pipelinePool.allocate(createInfo, device, vk::PipelineBindPoint::eCompute, vkPipeline, this);

		}
		return pipeline;
	}
	PipelineLayoutHandle createPipelineLayout(const Device* device, std::vector<DescriptorSetLayoutHandle>& descLayouts, GpuBlockBuffer& pushConstant, vk::ShaderStageFlags shaderStage) {
		vk::PipelineLayoutCreateInfo  info = {};

		std::vector<vk::PushConstantRange> ranges(pushConstant.size());

		for (int idx = 0; idx < ranges.size(); ++idx) {
			auto& pus = pushConstant[idx];
			ranges[idx].setOffset(pus.offset);
			ranges[idx].setSize(pus.size);
			ranges[idx].setStageFlags(shaderStage);
		}
		info.setPushConstantRanges(ranges);
		std::vector<vk::DescriptorSetLayout> layouts(descLayouts.size());
		for (int i = 0; i < layouts.size(); ++i) {
			layouts[i] = descLayouts[i]->getVkHandle();
		}
		info.setSetLayouts(layouts);
		auto vklayout = device->createPipelineLayout(info);
		return std::make_shared<tPipelineLayout>(device, vklayout);
	}

}