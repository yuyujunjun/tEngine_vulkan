#include"MassSpring.h"
#include"EngineContext.h"
#include"GameObject.h"
#include"Camera.h"
#include"ShaderVariable.h"
#include"GameObject.h"
#include"TextureFormatLayout.h"
#include"Material.h"
#include<random>
#include"Log.h"
#include"GraphicsState.h"
#include<glm/gtx/quaternion.hpp>
#include"ComponentFactory.h"
using namespace tEngine;
void defaultRender(std::vector<GameObject>objs, const CameraTransform& cam) {
	auto& context = tEngineContext::context;
	if (!context.hasInitialized()) {
		ContextInit();
	}
	auto device = tEngineContext::context.device.get();
	static auto renderPass = getSingleRenderpass(device, context.swapChain->getFormat());
	renderPass->setClearValue("back", { 0,0,0,1 });
	renderPass->setDepthStencilValue("depth", 1);
	auto shader = tShaderInterface::requestTexturedShader(device);
	auto camBuffer = shader->getShader()->requestBufferRange("CameraMatrix");
	context.Record([&](double timeDelta, CommandBufferHandle& cb) {
	

		uploadCameraMatrix(cam.m_matrix, Perspective(context.swapChain->getExtent()), shader);
		camBuffer->NextRangenoLock();
		shader->SetBuffer("CameraMatrix", camBuffer->buffer(), camBuffer->getOffset());
		renderPass->SetImageView("back", context.swapChain->getImage(context.getImageIdx()));
		renderPass->setTransientImageView("depth");
		for (auto& r : objs) {
			r->material->SetBuffer("CameraMatrix", camBuffer->buffer(), camBuffer->getOffset());
			r->material->SetValue(ShaderString(SV::_MATRIX_M), r->transform.Matrix());
			r->material->flushBuffer();
		}
		auto frameBuffer = renderPass->requestFrameBuffer();
		cb->beginRenderPass(renderPass, frameBuffer, true);
		cb->setViewport(frameBuffer->getViewPort());
		cb->setScissor(0, frameBuffer->getRenderArea());
		for (auto& r : objs) {
			flushGraphicsShaderState(r->material->shader, r->material->graphicsState, cb, renderPass.get(), 0);
			DrawMesh(r->meshbuffer.get(), cb);
		}
		cb->endRenderPass();
		});
	context.Loop(context.AddThreadContext());
}

int main() {
	ContextInit();
	auto& context = tEngine::tEngineContext::context;
	auto& device = context.device;

	CameraTransform cam;
	CameraSystem cam_sys;
	cam_sys.setCamera(&cam);

	
	
	
	SpringSolver::MSpring spring;
	spring.Start(device.get());
	GameObject cloth = std::make_shared<GameObject_>();
	GameObject sphere = std::make_shared<GameObject_>();
	auto shadowShader = tShader::Create(device.get());
	shadowShader->SetShaderModule({ "draw.vsh","draw.fsh" }, { vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment });
	cloth->setMaterial(shadowShader->getInterface());
	sphere->setMaterial(shadowShader->getInterface());
	sphere->transform.position = { 0,0.4,-1.4 };
	sphere->transform.scale = { 0.2,0.2,0.2 };
	auto meshAsset = LoadMesh("sphere.obj");
	sphere->setMesh(meshAsset->mesh);
	cloth->setMesh(spring.mesh);


	context.Update([&](double deltaTime) {
		auto& io = ImGui::GetIO();
		updateCameraBehavior(io, cam_sys);
		spring.Update(deltaTime);
		cloth->meshbuffer->setMeshUpload(spring.mesh,device.get());
		ImGui::Begin("Fluid");
		Eigen::Vector3f f = spring.fluid.cast<float>();
		ImGui::InputFloat3("x", f.data());
		spring.fluid = f.cast<double>();
		ImGui::End();
		//cloth->meshbuffer->uploadVertexBuffer(device.get(), nullptr);
		
		});

	defaultRender({ sphere,cloth }, cam);

}