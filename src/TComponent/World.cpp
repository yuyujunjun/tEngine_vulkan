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



	void RenderWorld::updateMainCameraRT(const SwapChainHandle& swapChain,uint32_t identityPerComponent,Camera& camera) {
		camera.setRenderTexture(swapChain->getImage(identityPerComponent),swapChain->getImage(identityPerComponent)->get_view()->getDefaultView());
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
	void RenderWorld::Render(CommandBufferHandle& cb, const SwapChainHandle& swapChain, uint32_t identityPerComponent) {
		//assert(cameras.size() > 0 && "Require at least a camera to transfer swapchain image to present layout");
		if (mainCamera!=-1) {
			updateMainCameraRT(swapChain, identityPerComponent,*ecsManager->GetComponent<Camera>(mainCamera));
		}
		std::vector<Camera*> cameras;
		for (auto entity : SceneView<Camera>(*ecsManager)) {
			cameras.emplace_back(ecsManager->GetComponent<Camera>(entity));
			PrepareRenderPass(*cameras.back());
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
			
			RenderWithCamera(renderers,cb, info, cameraBuffer);
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
	void RenderWorld::RenderWithMaterial(std::vector<Renderer*>& renderers,CommandBufferHandle& cb, const RenderInfo& renderInfo,  Material* mat, BufferRangeManager* camera_buffer) {
		for (auto& renderer : renderers) {
			if(NeedRender(renderer,renderInfo)){
				
				mat->SetBuffer("CameraMatrix", camera_buffer->buffer(), camera_buffer->getOffset());
				
				renderer->DrawWithMaterial(cb, renderInfo, mat);
			}
	
				
		}
	}
	void RenderWorld::RenderWithCamera(std::vector<Renderer*>& renderers,CommandBufferHandle& cb, const RenderInfo& renderInfo, BufferRangeManager* cameraBuffer) {
		Renderer::BufferMap bufferMap;
		bufferMap.emplace_back("CameraMatrix", cameraBuffer);
		for (auto& renderer : renderers) {
			if (NeedRender(renderer, renderInfo)) {
				renderer->Draw(cb, renderInfo, bufferMap, {});
			}
		}
	}
	

}