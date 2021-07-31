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
using namespace tEngine;
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
		vk::ImageLayout finalLayout = camera.getRenderTexture()->is_swapchain_image() ? vk::ImageLayout::ePresentSrcKHR: vk::ImageLayout::eShaderReadOnlyOptimal;
		auto renderPass = forwardRenderPass.requestRenderPass(device, camera.getRenderTexture()->getFormat(), finalLayout);
		renderPass->setClearValue("back", { 0,0,0,1 });
		renderPass->setDepthStencilValue("depth", 1);
		camera.getRenderPass() = renderPass;
		
	}
	void RenderWorld::Render(CommandBufferHandle& cb, const SwapChainHandle& swapChain, uint32_t idx) {
		assert(cameras.size() > 0 && "Require at least a camera to transfer swapchain image to present layout");
		if (mainCamera) {
			updateMainCameraRT(swapChain, idx, *mainCamera);
		}
		for (auto cam : cameras) {
			PrepareRenderPass(*cam);
		}
		for (auto cam : cameras) {
			auto frameBuffer=PrepareFrameBuffer(*cam);
			auto cameraBuffer=UpdateCameraBuffer(device, cam);
			cb->beginRenderPass(cam->getRenderPass(), frameBuffer, true);
			auto viewPort = frameBuffer->getViewPort();
			viewPort.width *= cam->getViewPortRatio().x;
			viewPort.height *= cam->getViewPortRatio().y;
			auto scissor = frameBuffer->getRenderArea();
			scissor.extent.width *= cam->getScissorRatio().x;
			scissor.extent.height *= cam->getScissorRatio().y;
			RenderInfo info;
			info.renderPass = cam->getRenderPass().get();
			info.subpass = 0;
			info.layer = cam->getLayer();
			info.isRender = nullptr;
			if (cam->beforeRender) {
				cam->beforeRender(cb, cameraBuffer,info,frameBuffer);
			}
			cb->setViewport(viewPort);
			cb->setScissor(0,scissor);
	
			RenderWithCamera(cb, info, cameraBuffer);
			if (cam->afterRender) {
				cam->afterRender(cb, cameraBuffer,info,frameBuffer);
			}
			cb->endRenderPass();
		}
	}
	bool NeedRender(Renderer* renderer, const RenderInfo& renderInfo) {
		bool isRender = true;
		isRender &= (renderer->getLayer() & renderInfo.layer)!=0;
		if (renderInfo.isRender) {
			isRender &= renderInfo.isRender(renderer);
		}
		return isRender;
	}
	BufferRangeManager* UpdateCameraBuffer(const Device* device,const Camera* camera) {
		return UpdateCameraBuffer(device, camera->transform.m_matrix, camera->transform.p_matrix);
	}
	
	BufferRangeManager* UpdateCameraBuffer(const Device* device, const glm::mat4& view, const glm::mat4& projection) {
		auto camera_buffer = requestCameraBufferRange(device);
		camera_buffer->NextRangenoLock();
		uploadCameraMatrix(view, projection, camera_buffer->buffer().get(), camera_buffer->getOffset());
		return camera_buffer;
	}
	void RenderWorld::RenderWithMaterial(CommandBufferHandle& cb, const RenderInfo& renderInfo,  Material* mat, BufferRangeManager* camera_buffer) {
		
		for (auto& renderer : renderers) {
			if(NeedRender(renderer,renderInfo)){
				mat->SetBuffer("CameraMatrix", camera_buffer->buffer(), camera_buffer->getOffset());
				
				renderer->DrawWithMaterial(cb, renderInfo, mat);
			}
	
				
		}
	}
	void RenderWorld::RenderWithCamera(CommandBufferHandle& cb, const RenderInfo& renderInfo, BufferRangeManager* bufferRange) {

		for (auto& renderer : renderers) {
			if (NeedRender(renderer, renderInfo)) {
				renderer->material->SetBuffer("CameraMatrix", bufferRange->buffer(), bufferRange->getOffset());
				renderer->material->flushBuffer();
				renderer->Draw(cb, renderInfo);
			}
		}
	}
	

}