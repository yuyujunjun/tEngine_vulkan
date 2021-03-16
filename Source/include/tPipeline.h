#pragma once
#include"Core.h"
#include"tDescriptorPool.h"
#include"../PipelineState.h"
namespace tEngine {
	struct GraphicsPipelineCreateInfo {
		void addShader(VkShaderModule mod, vk::ShaderStageFlagBits flags) {
			Stages.emplace_back(vk::PipelineShaderStageCreateInfo({}, flags,mod));
		}
		std::vector<vk::VertexInputBindingDescription> vertexInput;
		std::vector<vk::VertexInputAttributeDescription> vertexAttribute;
		struct TopologyState {
			vk::PrimitiveTopology topolygy;
			bool primitiveRestartEnable = {};
			bool operator==(const TopologyState& state)const { return state.topolygy == this->topolygy && state.primitiveRestartEnable == primitiveRestartEnable; }
			vk::PipelineInputAssemblyStateCreateInfo getCreateInfo()const {
				return vk::PipelineInputAssemblyStateCreateInfo({},topolygy,primitiveRestartEnable);
			}
		}topology;
		struct TessellationState {
			bool operator==(const TessellationState& state) const{return state.patchControlPoints == patchControlPoints; }
			uint32_t patchControlPoints;
		}tessellation;
		struct ViewPortState {
			std::vector<vk::Viewport> viewPorts;
			std::vector<vk::Rect2D> scissors;
			bool operator==(const ViewPortState& state)const { return state.viewPorts == viewPorts && state.scissors == scissors; }
		}viewport;
		struct RasterizationState {
			
			bool depthClampEnable=true;
			bool resterizerDiscardEnable=false;
			bool depthBiasEnable = false;
			float depthBiasConstantFactor = {};
			float depthBiasClamp = {};
			float depthBiasSlopeFactor = {};
			float lineWidth = {};
			vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
			vk::CullModeFlags cullMode = vk::CullModeFlagBits::eBack;
			vk::FrontFace frontFace = vk::FrontFace::eCounterClockwise;
			bool operator==(const  RasterizationState& state) const{
				return depthClampEnable == state.depthClampEnable
					&& resterizerDiscardEnable == state.resterizerDiscardEnable
					&& state.depthBiasEnable == depthBiasEnable
					&& state.depthBiasConstantFactor == depthBiasConstantFactor
					&& state.depthBiasClamp == depthBiasClamp
					&& state.depthBiasSlopeFactor == depthBiasSlopeFactor
					&& state.lineWidth == lineWidth
					&& polygonMode == state.polygonMode
					&& cullMode == state.cullMode
					&& frontFace == state.frontFace;
			}
			vk::PipelineRasterizationStateCreateInfo getCreateInfo()const {
				return vk::PipelineRasterizationStateCreateInfo({}, depthClampEnable, resterizerDiscardEnable
					,polygonMode,cullMode,frontFace,depthBiasEnable,depthBiasConstantFactor,depthBiasClamp,depthBiasSlopeFactor,lineWidth);
			}
		}rasterizationState;
		struct MultisampleState {
			
			vk::SampleCountFlagBits rasterizationSamples = vk::SampleCountFlagBits::e1;
			bool sampleShadingEnable = false;
			float minSampleShading = {};
			vk::SampleMask SampleMask = {};
			vk::Bool32 alphaToCoverageEnable = false;
			vk::Bool32 alphaToOneEnable = false;
			bool operator==(const MultisampleState& state)const {
				return rasterizationSamples == state.rasterizationSamples
					&& sampleShadingEnable == state.sampleShadingEnable
					&& SampleMask == state.SampleMask
					&& alphaToCoverageEnable == state.alphaToCoverageEnable
					&& alphaToOneEnable == state.alphaToOneEnable;
			}

			vk::PipelineMultisampleStateCreateInfo getCreateInfo()const {
				return vk::PipelineMultisampleStateCreateInfo({},rasterizationSamples,sampleShadingEnable,minSampleShading,&SampleMask,alphaToCoverageEnable
				,alphaToOneEnable);
			}
		}multisampleState;
		struct DepthStencilState {
			bool depthTestEnable = {};
			bool depthWriteEnable = {};
			vk::CompareOp depthCompareOp = vk::CompareOp::eLessOrEqual;
			vk::Bool32 depthBoundsTestEnable = {};
			vk::Bool32 stencilTestEnable = {};
			vk::StencilOpState front = {};
			vk::StencilOpState back = {};
			float minDepthBounds = {};
			float maxDepthBounds = {};
			bool operator==(const DepthStencilState& state)const {
				return depthTestEnable == state.depthTestEnable
					&& depthWriteEnable == state.depthWriteEnable
					&& depthCompareOp == state.depthCompareOp
					&& stencilTestEnable == state.stencilTestEnable
					&& front == state.front
					&& back == state.back
					&& minDepthBounds == state.minDepthBounds
					&& maxDepthBounds == state.maxDepthBounds;
			}
			vk::PipelineDepthStencilStateCreateInfo  getCreateInfo()const{
				return vk::PipelineDepthStencilStateCreateInfo({},depthTestEnable,depthWriteEnable,depthCompareOp,depthBoundsTestEnable,
					stencilTestEnable,front,back,minDepthBounds,maxDepthBounds);
			}
		}depthStencilState;
		struct ColorBlendState {
			vk::Bool32 logicOpEnable = {};
			vk::LogicOp logicOp = VULKAN_HPP_NAMESPACE::LogicOp::eClear;
			std::vector<vk::PipelineColorBlendAttachmentState> Attachments = {};
			vk::ArrayWrapper1D<float, 4> blendConstants = {};
			bool operator==(const ColorBlendState& state)const {
				return logicOpEnable == state.logicOpEnable
					&& logicOp == state.logicOp
					&& Attachments == state.Attachments
					&& blendConstants == state.blendConstants;
			}
			vk::PipelineColorBlendStateCreateInfo getCreateInfo() const{
				return vk::PipelineColorBlendStateCreateInfo({}, logicOpEnable,logicOp,Attachments,blendConstants);
			}
		}coloBlendState;
		struct DynamicState {
			std::vector< vk::DynamicState> dynamicStates;
			bool operator==(const DynamicState& state)const {
				return dynamicStates == state.dynamicStates;
			}
			vk::PipelineDynamicStateCreateInfo getCreateInfo()const {
				vk::PipelineDynamicStateCreateInfo info;
				info.setDynamicStates(dynamicStates);
			}

		}dynamicState;

		bool operator==(const GraphicsPipelineCreateInfo& info) const {
			return info.layout == layout
				&& info.renderPass == renderPass
				&& info.subpass == subpass
				&& vertexInput == info.vertexInput
				&& vertexAttribute == info.vertexAttribute
				&& topology == info.topology
				&& tessellation == info.tessellation
				&& viewport == info.viewport
				&& rasterizationState == info.rasterizationState
				&& multisampleState == info.multisampleState
				&& depthStencilState == info.depthStencilState
				&& coloBlendState == info.coloBlendState
				&& dynamicState == info.dynamicState;

		}
		bool operator!=(const GraphicsPipelineCreateInfo& info)const {
			return !(info == *this);
		}
		vk::Pipeline createPipeline(const Device* device)const {
			vk::GraphicsPipelineCreateInfo info;
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

			return device->createGraphicsPipeline(device->getPipelineCache(),info);
		}
		vk::PipelineLayout layout = {};
		vk::RenderPass renderPass = {};
		uint32_t subpass = {};
		
	private:
		std::vector<vk::PipelineShaderStageCreateInfo> Stages = {};
	};
	struct ComputePipelineCreateInfo {
		void setShader(VkShaderModule mod, vk::ShaderStageFlagBits flags) {
			Stage=(vk::PipelineShaderStageCreateInfo({}, flags, mod));
		}
		void setLayout(vk::PipelineLayout layout) {
			this->layout = layout;
		}
		bool operator==(const ComputePipelineCreateInfo& info) const{
			return info.layout == layout && info.Stage == Stage;

		}
		bool operator!=(const ComputePipelineCreateInfo& info)const {
			return !(info==*this);
		}
	private:
		vk::PipelineLayout layout = {};
		vk::PipelineShaderStageCreateInfo Stage;
	};
	class tPipeline {
	public:
		friend class tPipelineLayout;
		tPipeline(vk::PipelineBindPoint bindPoint,vk::Pipeline pipeline,tPipelineLayout& layout) :bindPoint(bindPoint), pipelineLayout(layout),vkpipeline(pipeline){}
		~tPipeline() {
			if (vkpipeline) {

				device->destroyPipeline(vkpipeline);

			}
		}
		const vk::Pipeline& getVkHandle()const {
			return vkpipeline;
		}
		const vk::PipelineBindPoint& getBindPoint() const{
			return bindPoint;
		}
		const tPipelineLayout& getPipelineLayout()const {
			return pipelineLayout;
		}
	protected:
		tPipelineLayout& pipelineLayout;
		vk::PipelineBindPoint bindPoint;
		vk::Pipeline vkpipeline;
	private:
		weakDevice device;

	};
	class tPipelineLayout {
	public:
		DECLARE_SHARED(tPipelineLayout)
		
		tPipelineLayout(Device* device,vk::PipelineLayout layout):device(device), vkLayout(layout){
			
		
		}
		tPipeline* requestGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) {
			auto pipeline=graphic_pipelinePool.request(createInfo);
			if (pipeline == nullptr) {
				auto vkPipeline = createInfo.createPipeline(device);
				pipeline=graphic_pipelinePool.allocate(createInfo, vk::PipelineBindPoint::eGraphics,vkPipeline,*this);

			}
			return pipeline;
		}
		tPipeline* requestComputePipeline(const ComputePipelineCreateInfo& createInfo) {

		}
		~tPipelineLayout() {
			if (vkLayout) {
				device->destroyPipelineLayout(vkLayout);
				vkLayout = vk::PipelineLayout();
			}
		}
		vk::PipelineLayout getVkHandle() {
			return vkLayout;
		}
	private:
		RingPool<tPipeline, GraphicsPipelineCreateInfo, 64> graphic_pipelinePool;
		RingPool<tPipeline, ComputePipelineCreateInfo, 32> compute_pipelinePool;
		vk::PipelineLayout vkLayout;
		weakDevice device;
	};
	

	
}