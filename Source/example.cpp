#include"tEngineContext.h"

int main(){
	using namespace tEngine;
	auto context = tEngineContext::Context();

	ThreadContext* threadContext=new ThreadContext(context);
	auto meshAsset=tEngine::LoadMesh("pig.obj");
	auto imageAsset = tEngine::LoadImage("174.png");
	
	auto shader=tShader::Create (context->device);
	std::vector<std::string> shaders = { "draw.vsh","draw.fsh" };
	shader->SetShaderModule(shaders, vk::ShaderStageFlagBits::eVertex| vk::ShaderStageFlagBits::eFragment);
	auto material = shader->getInterface();
	auto renderPass=getSingleRenderpass(context->device.get());
	//Load Textue
	ImageCreateInfo imageCreate=ImageCreateInfo::immutable_2d_image(imageAsset->width,imageAsset->height,VK_FORMAT_R8G8B8A8_UNORM);
	auto image = context->device->createImage(imageCreate,imageAsset);

	//Load Mesh
	

	renderPass->SetRenderFunctor(0,[&](CommandBufferHandle& cb,tRenderPass* rp,uint32_t subpass) {
		std::vector<vk::ClearValue> values = {vk::ClearValue()};
		values[0].color.setFloat32({1.f,0.f,1.f,1.f});
		values[1].depthStencil.depth = 1;

		cb->beginRenderPass(rp, rp->requestFrameBuffer(), true, values.data(), values.size());
		material.SetImage("_MainTex",image->get_view());
		flushGraphicsShaderState(material, cb, rp, subpass);

		
		});

	threadContext->cmdBuffers[0]->begin(vk::CommandBufferUsageFlags());

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