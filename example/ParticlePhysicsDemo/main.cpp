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
using namespace tEngine;






void defaultRender(RenderWorld* world, const CameraTransform& cam) {
	

}
int main() {
	ContextInit();
	auto& context = tEngine::tEngineContext::context;
	auto& device = context.device;
	tWorld world(device.get());
	ParticleWorld pWorld(100);
	auto cameraObj=Camera::Create();
	auto cam = cameraObj->getComponent<Camera>();
	cam->transform.m_windowSize = glm::uvec2(context.swapChain->getExtent().width, context.swapChain->getExtent().height);
	GameObject obj = GameObject_::Create();
	GameObject plane = GameObject_::Create();
	plane->AddComponent<MeshFilter>()->setMeshUpload(Mesh::UnitSquare(),device.get());
	
	obj->AddComponent<MeshFilter>();
	obj->AddComponent<Platform>(&pWorld);
	obj->getComponent<Platform>()->initializeMesh(device.get());
	auto shader = tShader::Create(device.get());
	shader->SetShaderModule({ "draw.vsh","draw.fsh" }, { vk::ShaderStageFlagBits::eVertex,vk::ShaderStageFlagBits::eFragment });
	obj->AddComponent<MeshRenderer>(std::make_shared<Material>(shader.get()));
	plane->AddComponent<MeshRenderer>(obj->getComponent<MeshRenderer>()->material);
	plane->transform.setOrientation ( Vector3(-90,0,0));
	plane->transform.setScale( Vector3(10,10,10));
	world.AddGameObject(plane);
	world.AddGameObject(obj);
	world.AddGameObject(cameraObj);
	
	CameraSystem cam_sys;
	cam_sys.setCamera(&cam->transform);
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
	context.Record([&](double timeDelta, CommandBufferHandle& cb) {
		world.getRenderWorld().Render(cb, context.swapChain, context.getImageIdx());
		});
	context.Loop(context.AddThreadContext());

	obj.reset();
	
	return 0;
}