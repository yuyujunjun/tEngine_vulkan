#include"EngineContext.h"
#include"Camera.h"
#include"ShaderVariable.h"
#include"GameObject.h"
#include"renderer.h"
#include"Material.h"
#include"World.h"
#include"pWorld.h"
#include"tParticles.h"
#include"pLinks.h"
#include"pWorld.h"
#include"platform.h"
using namespace tEngine;
using namespace tPhysics;






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
	ParticleWorld pWorld(100);

	auto& context = tEngine::tEngineContext::context;
	auto& device = context.device;
	tWorld world(device.get());
	GameObject obj = GameObject_::Create();
	GameObject plane = GameObject_::Create();
	plane->AddComponent<MeshBuffer>()->setMeshUpload(Mesh::UnitSquare(),device.get());
	
	obj->AddComponent<MeshBuffer>();
	obj->AddComponent<Platform>(&pWorld);
	obj->getComponent<Platform>()->initializeMesh(device.get());
	auto shader = tShader::Create(device.get());
	shader->SetShaderModule({ "draw.vsh","draw.fsh" }, { vk::ShaderStageFlagBits::eVertex,vk::ShaderStageFlagBits::eFragment });
	obj->AddComponent<MeshRenderer>(std::make_shared<Material>(shader.get()));
	plane->AddComponent<MeshRenderer>(obj->getComponent<MeshRenderer>()->material);
	plane->transform.rotation = Vector3(-90,0,0);
	plane->transform.scale = Vector3(10,10,10);
	world.RegistryMeshRenderer(plane);
	world.RegistryMeshRenderer(obj);
	
	CameraTransform cam;
	cam.m_windowSize = glm::uvec2(context.swapChain->getExtent().width,context.swapChain->getExtent().height);
	cam.update();
	CameraSystem cam_sys;
	cam_sys.setCamera(&cam);
	world.AddSystem(&cam_sys);
	

	float pos[2] = {0,0};
	bool rope = false;
	context.Update([&](double timeDelta) {
		auto& io = ImGui::GetIO();
		ImGui::Begin("Window");
		if (ImGui::Button("rope")) {
			if (rope) {
				obj->getComponent<Platform>()->cutRope(&pWorld);
				rope = false;
			}
			else {
				obj->getComponent<Platform>()->Rope(&pWorld);
				rope = true;
			}
		}
		ImGui::SliderFloat2("pos", pos, 0, 1);
		ImGui::End();
		
		
		world.update(timeDelta);
		obj->getComponent<Platform>()->updateMesh();
		
	});
	context.FixedUpdate([&](double timeDelta) {
		obj->getComponent<Platform>()->updateAdditionalMass(pos[0], pos[1]);
		pWorld.startFrame();
		pWorld.runPhysics(timeDelta);
	
		
		},1.0/120.0);
	defaultRender(&world,cam);
	obj.reset();
	
	return 0;
}