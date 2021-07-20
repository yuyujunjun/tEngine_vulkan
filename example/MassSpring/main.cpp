#include"EngineContext.h"
#include"Camera.h"
#include"ShaderVariable.h"
#include"GameObject.h"
#include"World.h"
#include"renderer.h"
#include"Material.h"
#include"MassSpring.h"
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

	CameraTransform cam;
	cam.m_windowSize= glm::uvec2(context.swapChain->getExtent().width, context.swapChain->getExtent().height);
	CameraSystem cam_sys;
	cam_sys.setCamera(&cam);
	tWorld world(device.get());
	
	
	
	SpringSolver::MSpring spring;
	spring.Start(device.get());
	GameObject cloth = createGameObject();
	GameObject sphere = createGameObject();
	auto shadowShader = tShader::Create(device.get());
	shadowShader->SetShaderModule({ "draw.vsh","draw.fsh" }, { vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment });
	cloth->AddComponent<MeshRenderer>(std::make_shared<Material>(shadowShader.get()));
	sphere->AddComponent<MeshRenderer>(std::make_shared<Material>(shadowShader.get()));
	world.RegistryMeshRenderer(cloth);
	world.RegistryMeshRenderer(sphere);
	sphere->transform.position = { 0,0.4,-1.4 };
	sphere->transform.scale = { 0.2,0.2,0.2 };
	auto meshAsset = LoadMesh("sphere.obj");
	sphere->AddComponent<MeshBuffer>()->setMeshUpload(meshAsset->mesh,device.get());
	cloth->AddComponent<MeshBuffer>()->setMeshUpload(spring.mesh, device.get());
	world.AddSystem(&cam_sys);


	context.Update([&](double deltaTime) {
		auto& io = ImGui::GetIO();
		world.update(deltaTime);
		spring.Update(deltaTime);
		cloth->getComponent<MeshBuffer>()->setMeshUpload(spring.mesh,device.get());
		ImGui::Begin("Fluid");
		Eigen::Vector3f f = spring.fluid.cast<float>();
		ImGui::InputFloat3("x", f.data());
		spring.fluid = f.cast<double>();
		ImGui::End();
		//cloth->meshbuffer->uploadVertexBuffer(device.get(), nullptr);
		
		});

	defaultRender(&world, cam);

}