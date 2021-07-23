#include"World.h"
#include"GameObject.h"
#include"renderer.h"
#include"Material.h"
#include"Camera.h"
#include"Buffer.h"
#include"CommandBufferBase.h"
#include"SwapChain.h"
#include"Image.h"
#include"FrameBuffer.h"
#include"RigidBody.h"
using namespace tPhysics;
namespace tEngine {

	void tWorld::AddGameObject(GameObject& obj) {
		if (obj->hasComponent<MeshRenderer>()) {
			renderWorld.RegistryMeshRenderer(obj->getComponent<MeshRenderer>());
		}
	
		if (obj->hasComponent<RigidBody>()) {
			rigidBodyWorld.addRigidBody(obj->getComponent<RigidBody>());
		}
		if (obj->hasComponent<Camera>()) {
			renderWorld.AddCamera(obj->getComponent<Camera>());
		}
	}
	void RenderWorld::AddCamera(Camera* cam) { 
		cameras.emplace_back(cam);
		if (cam->getRenderTexture() == nullptr) {
			mainCamera = cam;
		}
	}
	void RenderWorld::RemoveCamera(Camera* cam) {
		for (auto it = cameras.begin(); it != cameras.end(); ++it) {
			if ((*it)->gameObject->Identity() == cam->gameObject->Identity()) {
				cameras.erase(it);
			}
		}
	}
	void RenderWorld::RegistryMeshRenderer( Renderer* obj) {
		renderers.emplace_back(obj);
	}
	void RenderWorld::RemoveMeshRenderer(const Renderer* obj) {
		for (auto it = renderers.begin(); it != renderers.end(); ++it) {
			if ((*it)->gameObject->Identity()==obj->gameObject->Identity()) {
				renderers.erase(it);
			}
		}
	}
	void RenderWorld::updateMainCameraRT(const SwapChainHandle& swapChain,uint32_t idx,Camera& camera) {
		camera.setRenderTexture(swapChain->getImage(idx),swapChain->getImage(idx)->get_view()->getDefaultView());
	}
	FrameBufferHandle RenderWorld::PrepareFrameBuffer(Camera& camera) {
		camera.getRenderPass()->SetImageView("back", camera.getImage(), camera.getImageView());
		camera.getRenderPass()->setTransientImageView("depth");
		return camera.getRenderPass()->requestFrameBuffer();
	}

	void RenderWorld::PrepareRenderPass( Camera& camera) {
		auto renderPass = forwardRenderPass.requestRenderPass(device, camera.getRenderTexture()->getFormat());
		renderPass->setClearValue("back", { 0,0,0,1 });
		renderPass->setDepthStencilValue("depth", 1);
		camera.getRenderPass() = renderPass;
		
	}
	void RenderWorld::Render(CommandBufferHandle& cb, const SwapChainHandle& swapChain, uint32_t idx) {
		if (mainCamera) {
			updateMainCameraRT(swapChain, idx, *mainCamera);
		}
		for (auto cam : cameras) {
			PrepareRenderPass(*cam);
		}
		for (auto cam : cameras) {
			auto frameBuffer=PrepareFrameBuffer(*cam);
			cb->beginRenderPass(cam->getRenderPass(), frameBuffer, true);
			auto viewPort = frameBuffer->getViewPort();
			viewPort.width *= cam->getViewPortRatio().x;
			viewPort.height *= cam->getViewPortRatio().y;
			auto scissor = frameBuffer->getRenderArea();
			scissor.extent.width *= cam->getScissorRatio().x;
			scissor.extent.height *= cam->getScissorRatio().y;
			cb->setViewport(viewPort);
			cb->setScissor(0,scissor);
			RenderInfo info;
			info.renderPass = cam->getRenderPass().get();
			info.subpass = 0;
			RenderWithCamera(cb, info, renderers, cam);
			cb->endRenderPass();
		}
	}
	void RenderWorld::RenderWithMaterial(CommandBufferHandle& cb, const RenderInfo& renderInfo, std::vector<Renderer*>& gobjs, Material* mat, Camera* cam) {
		auto camera_buffer = requestCameraBufferRange(device);
		camera_buffer->NextRangenoLock();
		uploadCameraMatrix(cam->transform.m_matrix, cam->transform.p_matrix, camera_buffer->buffer().get(), camera_buffer->getOffset());
		for (auto& renderer : gobjs) {
			mat->SetBuffer("CameraMatrix", camera_buffer->buffer(), camera_buffer->getOffset());
			mat->SetValue(ShaderString(SV::_MATRIX_M), renderer->gameObject->transform.updateMtx());
			mat->flushBuffer();
			renderer->DrawWithMaterial(cb, renderInfo, mat);
		}
	}
	void RenderWorld::RenderWithCamera(CommandBufferHandle& cb, const RenderInfo& renderInfo, std::vector<Renderer*>& renderers,Camera* camera) {
		

		auto bufferRange = requestCameraBufferRange(device);
		bufferRange->NextRangenoLock();
		uploadCameraMatrix(camera->ViewMatrix(), camera->ProjectionMatrix(), bufferRange->buffer().get(), bufferRange->getOffset());
		for (auto& renderer : renderers) {
			renderer->material->SetBuffer("CameraMatrix", bufferRange->buffer(), bufferRange->getOffset());
			renderer->material->SetValue(ShaderString(SV::_MATRIX_M), renderer->gameObject->transform.updateMtx());
			renderer->material->flushBuffer();
			renderer->Draw(cb, renderInfo);
		}
	}
	

}