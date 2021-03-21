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
	shader->SetShaderModule({ "draw.vsh","blingPhong.fsh" }, { vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment });
	auto material = shader->getInterface();
	auto renderPass = getSingleRenderpass(device.get(),context->swapChain->getFormat(),(vk::Format)context->swapChain->getDepth()->getFormat());

	//Load Mesh
	auto mesh = std::make_shared<MeshBuffer>();
	auto meshAsset = tEngine::LoadMesh("Obj/Marry.obj");
	mesh->setMeshUpload(meshAsset->mesh, device.get());
	//Load Textue
	auto imageAsset = tEngine::LoadImage("Obj/MC003_Kozakura_Mari.png");
	auto image =createImage(device.get(),
		ImageCreateInfo::immutable_2d_image(imageAsset->width, imageAsset->height, VK_FORMAT_R8G8B8A8_UNORM), imageAsset, nullptr);
	material->SetImage("_MainTex", image);
	
	auto CameraBuffer = material->getShader()->requestBufferRange("CameraMatrix", 10);
	auto modelMatrix = material->getShader()->requestBufferRange("ModelMatrix", 3);
	auto materialInfo = material->getShader()->requestBufferRange("MaterialInfo",3);
	material->SetBuffer("MaterialInfo", materialInfo.buffer());
	material->SetBuffer("CameraMatrix", CameraBuffer.buffer());
	material->SetBuffer("ModelMatrix", modelMatrix.buffer());

	glm::vec3 lightPos(0.2, 0.2, 0.2);
	float lightIntensity = 3;
	glm::vec3 uKd = glm::vec3(0.2, 0.2, 0.2);
	glm::vec3 uKs = glm::vec3(0.2, 0.2, 0.2);

	context->Update([&](double timeDelta) {
		renderPass->SetImageView("back", context->swapChain->getImage(context->imageIdx));
		renderPass->SetImageView("depth", context->swapChain->getDepth());
		renderPass->setClearValue("back", { 0,0,0,1 });
		renderPass->setDepthStencilValue("depth", 1);
		CameraBuffer.NextRangenoLock();
		modelMatrix.NextRangenoLock();
		materialInfo.NextRangenoLock();
		material->SetBuffer("CameraMatrix", CameraBuffer.buffer(), CameraBuffer.getOffset());
		material->SetBuffer("ModelMatrix", modelMatrix.buffer(), modelMatrix.getOffset());
		material->SetBuffer("MaterialInfo", materialInfo.buffer(), materialInfo.getOffset());
		material->SetValue("lightPos", lightPos);
		material->SetValue<float>("lightIntensity", lightIntensity);
		material->SetValue("uKd", uKd);
		material->SetValue("uKs", uKs);
		material->SetValue("cameraPos", tEngineContext::context.cameraManipulator.getCameraPosition());
		uploadCameraMatrix(tEngineContext::context.cameraManipulator.getMatrix(), Perspective(context->swapChain->getExtent()), material.get());
		material->SetValue(ShaderString(SV::_MATRIX_M), glm::mat4(1));
		ImGui::ShowDemoWindow();
		ImGui::Begin("test");
	
		ImGui::Text("story begin");
		ImGui::DragFloat3("lightPos",&lightPos[0],.01,-10,10);
		ImGui::DragFloat3("uKd",&uKd[0],.01,0,3);
		ImGui::DragFloat3("uKs",&uKs[0],.01,0,5);
		ImGui::DragFloat("lightIntensity",&lightIntensity,.01,0,10);
		ImGui::End();

		});
	context->Record([&](double timeDelta, CommandBufferHandle& cb) {


		auto& frameBuffer = renderPass->requestFrameBuffer();
		cb->beginRenderPass(renderPass, frameBuffer, true);
		cb->setViewport(frameBuffer->getViewPort());
		cb->setScissor(0, frameBuffer->getRenderArea());
		flushGraphicsShaderState(material.get(), cb, renderPass.get(), 0);
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