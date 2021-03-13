#include"tEngineContext.h"
#include"tRenderGraph.h"
int main(){
	using namespace tEngine;
	auto context = tEngineContext::Context();

	tEngine::RenderGraph  graph;
	auto& gbuffer =graph.add_pass("gbuffer", tEngine::RenderGraphQueueFlagBits::GraphicsBit);
	tEngine::AttachmentInfo emissive, albedo, normal, pbr, depth; // Default is swapchain sized.
	emissive.format = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
	albedo.format = VK_FORMAT_R8G8B8A8_SRGB;
	normal.format = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
	pbr.format = VK_FORMAT_R8G8_UNORM;
	depth.format = VK_FORMAT_D32_SFLOAT;
	gbuffer.add_color_output("emissive", emissive);
	gbuffer.add_color_output("albedo", albedo);
	gbuffer.add_color_output("normal", normal);
	gbuffer.add_color_output("pbr", pbr);
	
	gbuffer.set_depth_stencil_output("depth", depth);

	auto& lighting = graph.add_pass("lighting", tEngine::RenderGraphQueueFlagBits::GraphicsBit);
	lighting.add_color_output("HDR", emissive, "emissive");
	lighting.add_attachment_input("albedo");
	lighting.add_attachment_input("normal");
	lighting.add_attachment_input("pbr");
	lighting.add_attachment_input("depth");
	lighting.set_depth_stencil_input("depth");

	lighting.add_texture_input("shadow-main"); // Some external dependencies
	lighting.add_texture_input("shadow-near");

	auto& adapt_pass = graph.add_pass("adapt-luminance", tEngine::RenderGraphQueueFlagBits::ComputeBit);
	tEngine::BufferInfo buffer_info;
	adapt_pass.add_storage_output("average-luminance-updated", buffer_info, "average-luminance");
	adapt_pass.add_texture_input("bloom-downsample-3");


	//Do what renderManger do, collect all visible renderers and render
	gbuffer.set_build_render_pass([this, type](CommandBuffer::SharedPtr& cmd) {
		render_main_pass(cmd, cam.get_projection(), cam.get_view());
		});

	gbuffer.set_get_clear_depth_stencil([](VkClearDepthStencilValue* value) -> bool {
		if (value)
		{
			value->depth = 1.0f;
			value->stencil = 0;
		}
		return true; // CLEAR or DONT_CARE?
		});

	gbuffer.set_get_clear_color([](unsigned render_target_index, VkClearColorValue* value) -> bool {
		if (value)
		{
			value->float32[0] = 0.0f;
			value->float32[1] = 0.0f;
			value->float32[2] = 0.0f;
			value->float32[3] = 0.0f;
		}
		return true; // CLEAR or DONT_CARE?
		});
	const char* backbuffer_source = getenv("GRANITE_SURFACE");
	graph.set_backbuffer_source(backbuffer_source ? backbuffer_source : "tonemapped");


	graph.bake();

	






	ThreadContext* threadContext=new ThreadContext(context);
	auto meshAsset=tEngine::LoadMesh("pig.obj");
	auto imageAsset = tEngine::LoadImage("174.png");
	
	auto shader=tShader::Create (context->device);
	std::vector<std::string> shaders = { "draw.vsh","draw.fsh" };
	shader->SetShaderModule(shaders, vk::ShaderStageFlagBits::eVertex| vk::ShaderStageFlagBits::eFragment);
//	auto material = tEngine::tShaderInterface::Create(context->device, threadContext->descriptorPool);
//	material->SetShader(shader);
	//auto renderPass = tRenderPass::();


	threadContext->cmdBuffers[0]->begin(vk::CommandBufferUsageFlags());
	auto image=CreateImageViewWithImage(context->device, imageAsset,threadContext->cmdBuffers[0] );
	shader->getInterface()->SetImage("_MainTex", image);
	threadContext->cmdBuffers[0]->bindDescriptorSets(threadContext->descriptorPool, vk::PipelineBindPoint::eGraphics, shader->getInterface().get());
	threadContext->cmdBuffers[0]->end();

	vk::SubmitInfo info;
	info.commandBufferCount = 1;
	info.pCommandBuffers =& threadContext->cmdBuffers[0]->getVkHandle();// data();
	info.setSignalSemaphoreCount(0);
	info.setWaitSemaphoreCount(0);
	context->graphicsQueue.submit(info);
	context->device->waitIdle();
	delete threadContext;
	shader.reset();
	//material.reset();
	//CreateImageViewWithImage();
	

}