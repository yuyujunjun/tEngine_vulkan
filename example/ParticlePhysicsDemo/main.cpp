#include"EngineContext.h"
#include"Camera.h"
#include"ShaderVariable.h"
#include"GameObject.h"
#include"renderer.h"
#include"Material.h"
#include"World.h"
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
	tWorld world(device.get());
	GameObject obj = GameObject_::Create();
	obj->AddComponent<MeshRenderer>(std::make_shared<Material>(tShaderInterface::requestTexturedShader(device.get())));
	//obj->getComponent<MeshRenderer>();
	obj->AddComponent<MeshBuffer>()->setMeshUpload(Mesh::UnitBox(),device.get());
	world.RegistryMeshRenderer(obj);
	
	CameraTransform cam;
	cam.m_windowSize = glm::uvec2(context.swapChain->getExtent().width,context.swapChain->getExtent().height);
	cam.update();
	CameraSystem cam_sys;
	cam_sys.setCamera(&cam);
	world.AddSystem(&cam_sys);
	context.Update([&](double timeDelta) {
		auto& io = ImGui::GetIO();
		
		world.update(timeDelta);
	});
	defaultRender(&world,cam);
	obj.reset();
	
	return 0;
}