#include"EngineContext.h"
#include"Camera.h"
#include"ShaderVariable.h"
#include"GameObject.h"
#include"renderer.h"
#include"Material.h"
#include"World.h"
#include"pWorld.h"
#include"tParticles.h"
#include"FlightSims.h"
using namespace tEngine;
using namespace tPhysics;
void TransformGeo(const Transform& transform, Geo* geo) {
	auto mtx = transform.getMtx();
	for (auto& v : geo->vertices) {
		v.Position = mtx * glm::vec4(v.Position, 1);
	}
}





void defaultRender(tWorld* world) {
	auto& context = tEngineContext::context;
	if (!context.hasInitialized()) {
		ContextInit();
	}
	auto device = tEngineContext::context.device.get();
	
	
	context.Record([&](double timeDelta, CommandBufferHandle& cb) {
		world->getRenderWorld().Render(cb, context.swapChain, context.getImageIdx());
		});
	context.Loop(context.AddThreadContext());
}
int main() {
	ContextInit();
	//ParticleWorld pWorld(100);
	
	//PhysicsWorld phy_world;
	auto& context = tEngine::tEngineContext::context;
	auto& device = context.device;
	tWorld world(device.get());
	
	GameObject aircraft = GameObject_::Create();
	Transform fuselage;// = GameObject_::Create();
	Transform rearFuselage;// = GameObject_::Create();
	Transform wings;// = GameObject_::Create();
	Transform rudder;// = GameObject_::Create();
	Transform tailPlane;// = GameObject_::Create();
	GameObject plane = GameObject_::Create();
	plane->AddComponent<MeshBuffer>()->setMeshUpload(Mesh::UnitSquare(), device.get());
	
	auto box = Mesh::UnitBox();
	auto g_fuselage = Mesh::UnitBox();
	auto g_rearFuselage = Mesh::UnitBox();
	auto g_wings = Mesh::UnitBox();
	auto g_rudder = Mesh::UnitBox();
	auto g_tailPlane = Mesh::UnitBox();
	//fuselage.setParent(&aircraft->transform);
	fuselage.setPosition(-0.5, 0, 0);
	fuselage.setScale(2, 0.8, 1);
	fuselage.updateMtx();
	rearFuselage.setPosition(1,0.15,0);
	rearFuselage.setScale(2.75,0.5,0.5);
	rearFuselage.updateMtx();
	wings.setPosition(0, 0.3, 0);
	wings.setScale(0.8,0.1,6);
	wings.updateMtx();
	rudder.setPosition(2.0,0.775,0);
	rudder.setScale(0.75,1.15,0.1);
	rudder.updateMtx();
	tailPlane.setPosition(1.9,0,0);
	tailPlane.setScale(0.85,0.1,2);
	tailPlane.updateMtx();
	TransformGeo(fuselage, &g_fuselage);
	TransformGeo(rearFuselage, &g_rearFuselage);
	TransformGeo(wings, &g_wings);
	TransformGeo(rudder, &g_rudder);
	TransformGeo(tailPlane, &g_tailPlane);
	g_fuselage.AddGeo(&g_rearFuselage).AddGeo(&g_wings).AddGeo(&g_rudder).AddGeo(&g_tailPlane);
	aircraft->AddComponent<MeshBuffer>()->setMeshUpload(g_fuselage,device.get());
	auto shader = tShader::Create(device.get());
	shader->SetShaderModule({ "draw.vsh","draw.fsh" }, { vk::ShaderStageFlagBits::eVertex,vk::ShaderStageFlagBits::eFragment });
	aircraft->AddComponent<MeshRenderer>(std::make_shared<Material>(shader.get()));
	aircraft->AddComponent<RigidBody>();
	plane->AddComponent<MeshRenderer>(aircraft->getComponent<MeshRenderer>()->material);
	plane->transform.setOrientation(Vector3(-90, 0, 0));
	plane->transform.setScale(Vector3(10, 10, 10));
	plane->transform.setPosition(0, -10, 0);
	plane->transform.setParent(&aircraft->transform);
	



	auto body = aircraft->getComponent<RigidBody>();
	FlightSims sims(body);
	
	auto& phy_world = world.getPhysicsWorld();
	phy_world.registerForce(&sims.left_wing, body);
	phy_world.registerForce(&sims.right_wing, body);
	phy_world.registerForce(&sims.rudder, body);
	phy_world.registerForce(&sims.tail, body);
	GameObject camera = Camera::Create();
	
	camera->getComponent<Camera>()->transform.m_windowSize = glm::uvec2(context.swapChain->getExtent().width,context.swapChain->getExtent().height);
	camera->getComponent<Camera>()->transform.update();
	CameraSystem cam_sys;
	cam_sys.setCamera(&camera->getComponent<Camera>()->transform);
	world.AddSystem(&cam_sys);
	world.AddGameObject(aircraft);
	world.AddGameObject(plane);
	world.AddGameObject(camera);

	float pos[2] = {0,0};
	bool rope = false;
	context.Update([&](double timeDelta) {
		auto& io = ImGui::GetIO();
		world.update(timeDelta);
		ImGui::Begin("control");
		ImGui::SliderFloat("left wing", &sims.left_wing_control,-1,1);
		ImGui::SliderFloat("right wing", &sims.right_wing_control,-1,1);
		ImGui::SliderFloat("rudder_control", &sims.rudder_control,-1,1);
		ImGui::SliderFloat3("wind", &sims.windSpeed[0], -10, 10);
		sims.left_wing.setControl(sims.left_wing_control);
		sims.right_wing.setControl(sims.right_wing_control);
		sims.rudder.setControl(sims.rudder_control);
		ImGui::End();

		if (glfwGetKey(context.gWindow, 'R')) {
			aircraft->getComponent<RigidBody>()->setVelocity({ 0, 0, 0 });
			aircraft->getComponent<RigidBody>()->setAngularVelocity({ 0,0,0 });
			aircraft->transform.setPosition({ 0, 0, 0 });
			aircraft->transform.setOrientation({ 0,0,0 });

		}
		
	
	});
	context.FixedUpdate([&](double timeDelta) {
		phy_world.startFrame();
		phy_world.runPhysics(timeDelta);
		//LOG(LogLevel::Information,aircraft->transform.getEulerAngle().x, aircraft->transform.getEulerAngle().y, aircraft->transform.getEulerAngle().z);
		},1.0/120.0);
	defaultRender(&world);
	aircraft.reset();
	
	return 0;
}