#include"EngineContext.h"
#include"Camera.h"
#include"ShaderVariable.h"
#include"GameObject.h"
using namespace tEngine;

void defaultRender(std::vector<GameObject>objs, const CameraComponent& cam) {
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
		camBuffer->NextRangenoLock();
		shader->SetBuffer("CameraMatrix", camBuffer->buffer(), camBuffer->getOffset());
		uploadCameraMatrix(cam.m_matrix, Perspective(context.swapChain->getExtent()), shader.get());
		renderPass->SetImageView("back", context.swapChain->getImage(context.getImageIdx()));
		renderPass->setTransientImageView("depth");
		for (auto& r : objs) {
			r->material->SetBuffer("CameraMatrix", camBuffer->buffer(), camBuffer->getOffset());
			r->material->flushBuffer();
		}
		auto frameBuffer = renderPass->requestFrameBuffer();
		cb->beginRenderPass(renderPass, frameBuffer, true);
		cb->setViewport(frameBuffer->getViewPort());
		cb->setScissor(0, frameBuffer->getRenderArea());
		for (auto& r : objs) {
			flushGraphicsShaderState(r->material->shader.get(), r->material->graphicsState, cb, renderPass.get(), 0);
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
	GameObject character = std::make_shared<GameObject_>();
	character->setMaterial(tShaderInterface::requestTexturedShader(device.get()));
	auto meshAsset = tEngine::LoadMesh("Obj/Marry.obj");
	character->setMesh(meshAsset->mesh);
	auto imageAsset = tEngine::LoadImage("Obj/MC003_Kozakura_Mari.png");
	auto image = createImage(device.get(),
		ImageCreateInfo::immutable_2d_image(imageAsset->width, imageAsset->height, VK_FORMAT_R8G8B8A8_UNORM), imageAsset, nullptr);
	character->material->SetImage("_MainTex", image);
	character->material->SetValue(ShaderString(SV::_MATRIX_M), character->transform.Matrix());

	CameraComponent cam;
	CameraSystem cam_sys;
	cam_sys.setCamera(&cam);
	context.Update([&](double timeDelta) {
		auto& io = ImGui::GetIO();
		updateCameraBehavior(io, cam_sys);
		});
	defaultRender({ character }, cam);
	return 0;
}