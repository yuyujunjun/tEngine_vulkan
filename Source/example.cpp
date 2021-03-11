#include"tEngineContext.h"
int main(){
	using namespace tEngine;
	auto context=tEngineContext::Context();
	ThreadContext* threadContext=new ThreadContext(context);
	auto meshAsset=tEngine::LoadMesh("pig.obj");
	auto imageAsset = tEngine::LoadImage("174.png");
	
	auto shader=tShader::Create (context->device);
	std::vector<std::string> shaders = { "draw.vsh","draw.fsh" };
	shader->SetShaderModule(shaders, vk::ShaderStageFlagBits::eVertex| vk::ShaderStageFlagBits::eFragment);
	auto material = tEngine::tMaterial::Create(context->device, threadContext->descriptorPool);
	material->SetShader(shader);
	


	threadContext->cmdBuffers[0]->begin(vk::CommandBufferUsageFlags());
	auto image=CreateImageViewWithImage(context->device, imageAsset,threadContext->cmdBuffers[0] );
	material->SetImage("_MainTex", image);
	material->BindDescriptorSets(threadContext->cmdBuffers[0]);
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
	material.reset();
	//CreateImageViewWithImage();
	

}