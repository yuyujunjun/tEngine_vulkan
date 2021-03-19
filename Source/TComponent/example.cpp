#include"EngineContext.h"
#include"Camera.h"
#include"ShaderVariable.h"
#include"GameObject.h"
using namespace tEngine;


int main() {
	ContextInit();
	auto context = &tEngineContext::context;
	auto& device = context->device;

	ThreadContext* threadContext = new ThreadContext(context);
	auto shader = tShader::Create(device.get());
	std::vector<std::string> shaders = { "draw.vsh","draw.fsh" };
	shader->SetShaderModule(shaders, { vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment });
	auto material = shader->getInterface();
	auto renderPass = getSingleRenderpass(device.get(),context->swapChain);

	//Load Mesh
	auto mesh = std::make_shared<MeshBuffer>();
	auto meshAsset = tEngine::LoadMesh("pig.obj");
	auto quad = Mesh::UnitSquare();
	//FlipFace(quad);
	FlipFace(quad);
	mesh->setMeshUpload(meshAsset->mesh, device.get());
	//Load Textue
	auto imageAsset = tEngine::LoadImage("174.png");
	auto image =createImage(device.get(),
		ImageCreateInfo::immutable_2d_image(imageAsset->width, imageAsset->height, VK_FORMAT_R8G8B8A8_UNORM), imageAsset, nullptr);
	
	material->SetImage("_MainTex", image);
	

	CameraManipulator camera;
	auto CameraBuffer = material->getShader()->requestBufferRange("CameraMatrix", 1);
	auto modelMatrix = material->getShader()->requestBufferRange("ModelMatrix", 1);
	material->SetBuffer("CameraMatrix", CameraBuffer.buffer(), 3);
	material->SetBuffer("ModelMatrix", modelMatrix.buffer(), 10);

	context->Update([&](uint32_t imageIdx) {
		renderPass->SetImageView("back", context->swapChain->getImage(imageIdx));
		renderPass->SetImageView("depth", context->swapChain->getDepth());
		renderPass->setClearValue("back", { 0,0,0,1 });
		renderPass->setDepthStencilValue("depth", 1);
		
		
		});
	context->Record([&](uint32_t idx, CommandBufferHandle& cb) {

		CameraBuffer.NextRangenoLock();
		modelMatrix.NextRangenoLock();
		material->SetBuffer("CameraMatrix", CameraBuffer.buffer(), CameraBuffer.getOffset());
		material->SetBuffer("ModelMatrix", modelMatrix.buffer(), modelMatrix.getOffset());


		uploadCameraMatrix(tEngineContext::context.cameraManipulator.getMatrix(), Perspective(context->swapChain->getExtent()), material.get());
		material->SetValue(ShaderString(SV::_MATRIX_M), glm::mat4(1));


		auto& frameBuffer = renderPass->requestFrameBuffer();
		cb->beginRenderPass(renderPass, frameBuffer, true);
		cb->setViewport(frameBuffer->getViewPort());
		cb->setScissor(0, frameBuffer->getRenderArea());
		flushGraphicsShaderState(*material.get(), cb, renderPass.get(), 0);
		DrawMesh(mesh.get(), cb);

		cb->endRenderPass();
		
		});



	context->Loop(threadContext);


	


	
	
	
	delete threadContext;
	shader.reset();
	material.reset();
	mesh.reset();
	image.reset();
	CameraBuffer.removeBuffer();
	modelMatrix.removeBuffer();
	renderPass.reset();

	//delete context;
	//material.reset();
	//CreateImageViewWithImage();


}