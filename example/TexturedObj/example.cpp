#include"EngineContext.h"
#include"Camera.h"
#include"ShaderVariable.h"
#include"GameObject.h"
#include"World.h"
#include"renderer.h"
#include"Material.h"
using namespace tEngine;


void defaultRender(tWorld* world, const CameraTransform& cam) {
	auto& context = tEngineContext::context;
	if (!context.hasInitialized()) {
		ContextInit();
	}
	auto device = tEngineContext::context.device.get();
	static auto renderPass = getSingleRenderpass(device, context.swapChain->getFormat());
	renderPass->setClearValue("back", { 0,0,0,1 });
	renderPass->setDepthStencilValue("depth", 1);
	context.Record([&](double timeDelta, CommandBufferHandle& cb) {
		renderPass->SetImageView("back", context.swapChain->getImage(context.getImageIdx()));
		renderPass->setTransientImageView("depth");
		auto frameBuffer = renderPass->requestFrameBuffer();
		cb->beginRenderPass(renderPass, frameBuffer, true);
		cb->setViewport(frameBuffer->getViewPort());
		cb->setScissor(0, frameBuffer->getRenderArea());
		RenderInfo info;
		info.renderPass = renderPass.get();
		info.subpass = 0;
		world->renderWithCamera(cb, info, &cam);
		cb->endRenderPass();
		});
	context.Loop(context.AddThreadContext());
}
int main() {
	ContextInit();

	auto& context = tEngine::tEngineContext::context;
	auto& device = context.device; 
	tWorld world(context.device.get());
	auto meshAsset = tEngine::LoadMesh("Obj/Marry.obj");
	GameObject character = GameObject_::Create();
	auto renderer = character->AddComponent<MeshRenderer>(std::make_shared<Material>(tShaderInterface::requestTexturedShader(device.get())));
//	renderer->setMaterial(std::make_shared<Material>(tShaderInterface::requestTexturedShader(device.get())));
	character->AddComponent<MeshBuffer>()->setMeshUpload(meshAsset->mesh,device.get());
	auto imageAsset = tEngine::LoadImage("Obj/MC003_Kozakura_Mari.png");
	auto image = createImage(device.get(),
		ImageCreateInfo::immutable_2d_image(imageAsset->width, imageAsset->height, VK_FORMAT_R8G8B8A8_UNORM), imageAsset, nullptr);
	renderer->material->SetImage("_MainTex", image);
	renderer->material->SetValue(ShaderString(SV::_MATRIX_M), character->transform.Matrix());
	world.RegistryMeshRenderer(character);

	CameraTransform cam;
	cam.m_windowSize = glm::uvec2(context.swapChain->getExtent().width, context.swapChain->getExtent().height);
	cam.update();
	CameraSystem cam_sys;
	cam_sys.setCamera(&cam);
	context.Update([&](double timeDelta) {
		auto& io = ImGui::GetIO();
		cam_sys.ExecuteAllComponents(timeDelta);
		});
	defaultRender(&world, cam);
	return 0;
}