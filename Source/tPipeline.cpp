#include"tPipeline.h"
#include"tFrameBuffer.h"
#include"tShader.h"
namespace tEngine {
	GraphicsPipelineCreateInfo getDefaultPipelineCreateInfo(tShaderInterface* shader, const tRenderPass* renderPass, uint32_t subpass, const tFrameBuffer* frameBuffer) {
		// Shader
		GraphicsPipelineCreateInfo createInfo;
		for (int i = 0; i < shader->getShader()->shaderModule.size(); ++i) {
			const auto& sha = shader->getShader();
			createInfo.addShader(sha->shaderModule[i], sha->shaderStage[i]);
		}
		//Blend

		vk::PipelineColorBlendAttachmentState blendState = {};
		blendState.blendEnable = false;
		blendState.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
		for (uint32_t i = 0; i < renderPass->getSubpass(subpass)->getColorAttachmentCount(); ++i) {
			createInfo.coloBlendState.Attachments.emplace_back(blendState);
		}
		//DepthStencil

		createInfo.depthStencilState.depthTestEnable = true;
		createInfo.depthStencilState.depthWriteEnable = true;
		//createInfo.depthStencilState.depthCompareOp = vk::CompareOp::eAlways;
		//dynamicState
		createInfo.dynamicState.dynamicStates.emplace_back(vk::DynamicState::eViewport);

		//Layout
		createInfo.layout = shader->getShader()->pipelinelayout->getVkHandle();

		///multiSampleState
		createInfo.multisampleState.sampleShadingEnable = false;
		//createInfo.multisampleState.SampleMask = 0xFFFFFFFF;
		//rasterizationState
		//createInfo.rasterizationState.cullMode = vk::CullModeFlagBits::eBack;
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
	vk::Pipeline GraphicsPipelineCreateInfo:: createPipeline(const Device* device)const {
		vk::GraphicsPipelineCreateInfo info = {};
		info.setLayout(layout);
		info.setPColorBlendState(&coloBlendState.getCreateInfo());
		info.setPDepthStencilState(&depthStencilState.getCreateInfo());
		info.setPDynamicState(&dynamicState.getCreateInfo());
		vk::PipelineVertexInputStateCreateInfo VertexInputState;
		VertexInputState.setVertexAttributeDescriptions(vertexAttribute);
		VertexInputState.setVertexBindingDescriptions(vertexInput);
		info.setPVertexInputState(&VertexInputState);
		info.setPInputAssemblyState(&topology.getCreateInfo());
		info.setPMultisampleState(&multisampleState.getCreateInfo());
		info.setPRasterizationState(&rasterizationState.getCreateInfo());
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
}