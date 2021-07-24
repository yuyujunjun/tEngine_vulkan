#include"EngineContext.h"
#include"Camera.h"
#include"ShaderVariable.h"
#include"GameObject.h"
#include"World.h"
#include"renderer.h"
#include"Material.h"
#include"MassSpring.h"
using namespace tEngine;


int main() {
	ContextInit();
	auto& context = tEngine::tEngineContext::context;
	auto& device = context.device;
	tWorld world(device.get());
	auto camera = Camera::Create();
	camera->getComponent<Camera>()->transform.m_windowSize = glm::uvec2(context.swapChain->getExtent().width, context.swapChain->getExtent().height);
	camera->getComponent<Camera>()->transform.update();
	world.AddGameObject(camera);
	
	CameraSystem cam_sys;
	cam_sys.setCamera(&camera->getComponent<Camera>()->transform);
	//RenderWorld world(device.get());
	
	
	
	SpringSolver::MSpring spring;
	spring.Start(device.get());
	GameObject cloth = createGameObject();
	GameObject sphere = createGameObject();
	auto shadowShader = tShader::Create(device.get());
	shadowShader->SetShaderModule({ "draw.vsh","draw.fsh" }, { vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment });
	cloth->AddComponent<MeshRenderer>(std::make_shared<Material>(shadowShader.get()));
	sphere->AddComponent<MeshRenderer>(std::make_shared<Material>(shadowShader.get()));
	world.AddGameObject(cloth);
	world.AddGameObject(sphere);
	sphere->transform.setPosition( { 0,0.4,-1.4 });
	sphere->transform.setScale( { 0.2,0.2,0.2 });
	auto meshAsset = LoadMesh("sphere.obj");
	sphere->AddComponent<MeshBuffer>()->setMeshUpload(meshAsset->mesh,device.get());
	cloth->AddComponent<MeshBuffer>()->setMesh(spring.mesh);
	oneTimeSubmit(device.get(), [&](const CommandBufferHandle& cb) {
		cloth->getComponent<MeshBuffer>()->createVertexBuffer(device.get(),cb,BufferDomain::Host );
		cloth->getComponent<MeshBuffer>()->createIdxBuffer(device.get(),cb,BufferDomain::Device );
		
		});
	
	world.AddSystem(&cam_sys);


	context.Update([&](double deltaTime) {
		auto& io = ImGui::GetIO();
		world.update(deltaTime);
		spring.Update(deltaTime);
		cloth->getComponent<MeshBuffer>()->setMesh(spring.mesh);
		cloth->getComponent<MeshBuffer>()->uploadVertexBuffer(device.get(),nullptr);
		ImGui::Begin("Fluid");
		Eigen::Vector3f f = spring.fluid.cast<float>();
		ImGui::InputFloat3("x", f.data());
		spring.fluid = f.cast<double>();
		ImGui::End();
		//cloth->meshbuffer->uploadVertexBuffer(device.get(), nullptr);
		
		});

	context.Record([&](double timeDelta, CommandBufferHandle& cb) {
		world.getRenderWorld().Render(cb, context.swapChain, context.getImageIdx());
		});
	context.Loop(context.AddThreadContext());

}