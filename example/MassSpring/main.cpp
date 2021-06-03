#include"MassSpring.h"
int main() {
	using namespace tEngine;
	tEngine::ContextInit();
	auto& context = tEngineContext::context;
	auto& device = tEngineContext::context.device;
	
	auto shader = tShader::Create(device.get());// ("draw.vsh", "draw.fsh");
	shader->SetShaderModule({ "draw.vsh","draw.fsh" }, { vk::ShaderStageFlagBits::eVertex,vk::ShaderStageFlagBits::eFragment });
	auto clothMaterial = shader->getInterface();
	auto sphereMaterial = shader->getInterface();
	//clothMaterial->SetImage("_MainTex", tImage::requestDummyImage(device.getComponent()));
//	sphereMaterial->SetImage("_MainTex", tImage::requestDummyImage(device.getComponent()));

	auto renderPass = getSingleRenderpass(device.get(), context.swapChain->getFormat(),(vk::Format)context.swapChain->getDepth()->getFormat());

	auto cameraBuffer = shader->requestBufferRange("CameraMatrix", 2);
	auto transformBuffer = shader->requestBufferRange("ModelMatrix", 4);
	Transform sphereTransform;
	sphereTransform.position = { 0,0.4,-1.4 };
	sphereTransform.scale = { 0.2,0.2,0.2 };
	SpringSolver::MSpring spring;
	spring.Start(device.get());
	context.cameraManipulator.setLookat({ 0,0,1 }, { 0,0,0 }, { 0,1,0 });
	context.Update([&](double deltaTime) {
		spring.Update(deltaTime);
		spring.mesh.uploadVertexBuffer(context.device.get(), nullptr);
		//request new buffer range
		cameraBuffer.NextRangenoLock();
		clothMaterial->SetBuffer("CameraMatrix", cameraBuffer.buffer(), cameraBuffer.getOffset());
		sphereMaterial->SetBuffer("CameraMatrix", cameraBuffer.buffer(), cameraBuffer.getOffset());
		//request new buffer range
		transformBuffer.NextRangenoLock();
		clothMaterial->SetBuffer("ModelMatrix", transformBuffer.buffer(), transformBuffer.getOffset());
		transformBuffer.NextRangenoLock();
		sphereMaterial->SetBuffer("ModelMatrix", transformBuffer.buffer(), transformBuffer.getOffset());
		//Update
		uploadCameraMatrix(context.cameraManipulator.getMatrix(), Perspective(context.swapChain->getExtent()), clothMaterial.get());
		uploadCameraMatrix(context.cameraManipulator.getMatrix(), Perspective(context.swapChain->getExtent()), sphereMaterial.get());
		clothMaterial->SetValueOnBuffer(ShaderString(SV::_MATRIX_M), glm::mat4(1));
		sphereMaterial->SetValueOnBuffer(ShaderString(SV::_MATRIX_M), sphereTransform.Matrix());

		//FrameBuffer
		setupFrameBufferBySwapchain(renderPass);
		});
	context.Record([&](double, CommandBufferHandle& cb) {
		cb->beginRenderPass(renderPass, renderPass->requestFrameBuffer(), true);
		cb->setViewport(renderPass->requestFrameBuffer()->getViewPort());
		cb->setScissor(0, renderPass->requestFrameBuffer()->getRenderArea());
		flushGraphicsShaderState(clothMaterial.get(), cb, renderPass.get(), 0);
		DrawMesh(&spring.mesh, cb, 1);
		flushGraphicsShaderState(sphereMaterial.get(), cb, renderPass.get(), 0);
		DrawMesh(&spring.collide, cb, 1);
		cb->endRenderPass();
		});
	ThreadContext* threadContext = new ThreadContext(&context);
	context.Loop(threadContext);
	delete threadContext;

}