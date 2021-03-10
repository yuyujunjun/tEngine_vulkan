#include"tEngineContext.h"
int main(){
	using namespace tEngine;
	auto context=tEngineContext::Context();
	ThreadContext* threadContext=new ThreadContext(context);
	auto meshAsset=tEngine::LoadMesh("pig.obj");
	auto imageAsset = tEngine::LoadImage("174.png");
	auto shader=tShader::Create (context->device);
	shader->AddShaderModule("draw.vsh", vk::ShaderStageFlagBits::eVertex);
	shader->AddShaderModule("draw.fsh", vk::ShaderStageFlagBits::eFragment);
	threadContext->cmdBuffers[0]->begin(vk::CommandBufferUsageFlags());
	CreateImageViewWithImage(context->device, imageAsset,threadContext->cmdBuffers[0] );
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
	//CreateImageViewWithImage();
	

}