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
#include"MeshBuffer.h"
#include"Shader.h"
#include"ShaderInterface.h"
#include"AABB.h"
#include"Light.h"
using namespace tEngine;
namespace tEngine {
	void tWorld::updateTransform() {
		for (auto entity : SceneView<Transform>(ecsManager)) {
			auto transform = ecsManager.GetComponent<Transform>(entity);
			transform->updateMtx();
		}
	}
	void tWorld::updateAABB() {
		for (auto entity : SceneView<Transform, MeshFilter, AABB>(ecsManager)) {
			auto transformMat = ecsManager.GetComponent<Transform>(entity)->getMat3();
			auto position = ecsManager.GetComponent<Transform>(entity)->getPosition();
			auto data = &ecsManager.GetComponent<MeshFilter>(entity)->getMesh();
			auto bounds = ecsManager.GetComponent<AABB>(entity);
			bounds->CLEAR();
			if (data == nullptr)continue;
			Vector3 maxPoint(MINREAL);
			Vector3 minPoint(MAXREAL);
			for (int i = 0; i < data->vertices.size(); ++i) {
				glm::vec3 worldPos = transformMat * data->vertices[i].Position+ position;
				maxPoint = glm::max(worldPos, maxPoint);
				minPoint = glm::min(worldPos, minPoint);
			}
			bounds->ADD(maxPoint);
			bounds->ADD(minPoint);
		}
	}
	EntityID createCamera(EcsManager* ecsManager, glm::uvec2 screenSize) {
		auto entity = ecsManager->NewEntity();
		ecsManager->entity_map[entity] = "camera";
		ecsManager->AddComponent<Camera>(entity);
		ecsManager->AddComponent<CameraTransform>(entity)->m_windowSize = screenSize;
		ecsManager->GetComponent<CameraTransform>(entity)->update();
		return entity;
	}
	EntityID createGameObject(EcsManager* ecsManager) {
		auto entity = ecsManager->NewEntity();
		ecsManager->AddComponent<Transform>(entity);
		ecsManager->AddComponent<View>(entity);
		ecsManager->AddComponent<MeshFilter>(entity);
		ecsManager->AddComponent<AABB>(entity);
	
		return entity;
	}


	void RenderWorld::updateMainCameraRT(const SwapChainHandle& swapChain, uint32_t identityPerComponent, Camera& camera) {
		camera.setRenderTexture(swapChain->getImage(identityPerComponent), swapChain->getImage(identityPerComponent)->get_view()->getDefaultView());
	}
	FrameBufferHandle RenderWorld::PrepareFrameBuffer(Camera& camera) {
		camera.getRenderPass()->SetImageView("back", camera.getImage(), camera.getImageView());
		camera.getRenderPass()->setTransientImageView("depth");
		return camera.getRenderPass()->requestFrameBuffer();
	}

	void RenderWorld::PrepareRenderPass(Camera& camera) {
		vk::ImageLayout finalLayout = camera.getRenderTexture()->is_swapchain_image() ? vk::ImageLayout::ePresentSrcKHR : vk::ImageLayout::eShaderReadOnlyOptimal;
		auto renderPass = forwardRenderPass.requestRenderPass(device, camera.getRenderTexture()->getFormat(), finalLayout);
		renderPass->setClearValue("back", { 0,0,0,1 });
		renderPass->setDepthStencilValue("depth", 1);
		camera.getRenderPass() = renderPass;

	}
	void updateLightBuffer(Light* light, Transform* light_transform) {
		//	auto light = ecsManager->GetComponent<Light>(entity);
			//auto light_transform = ecsManager->GetComponent<Transform>(entity);
		size_t offset = 0;
		light->lightPropertyBuffer->setRange(&light_transform->getPosition(), offset + light->offset, sizeof(glm::vec3));
		offset += sizeof(glm::vec3);
		light->lightPropertyBuffer->setRange(&light->halfblockSize, offset + light->offset, sizeof(int));
		offset += sizeof(int);
		light->lightPropertyBuffer->setRange(&light->world_to_shadow, offset + light->offset, sizeof(glm::mat4));
		offset += sizeof(glm::mat4);
		light->lightPropertyBuffer->setRange(&glm::vec2(light->shadowMap->get_width(),light->shadowMap->get_height()), offset + light->offset, sizeof(glm::vec2));
		offset += sizeof(glm::vec2);
		light->lightPropertyBuffer->setRange(&light->maxKernelSize, offset + light->offset, sizeof(int));
		offset += sizeof(int);
		light->lightPropertyBuffer->setRange(&light->lightIntensity, offset + light->offset, sizeof(float));
		offset += sizeof(float);
		light->lightPropertyBuffer->setRange(&light->lightArea, offset + light->offset, sizeof(float));
	}
	void RenderWorld::Render(CommandBufferHandle& cb, const SwapChainHandle& swapChain, uint32_t identityPerComponent) {
		//assert(cameras.size() > 0 && "Require at least a camera to transfer swapchain image to present layout");
		if (mainCamera != -1) {
			updateMainCameraRT(swapChain, identityPerComponent, *ecsManager->GetComponent<Camera>(mainCamera));
		}

		if (collectShadowPass == nullptr) {
			collectShadowPass = std::make_shared<CollectShadowPass>(device, ecsManager);
		}
		//Update Light
		
		collectShadowPass->collectShadow(cb);
		for (auto entity : SceneView<Light>(*ecsManager)) {
			auto light = ecsManager->GetComponent<Light>(entity);
			light->updateBuffer();
			auto light_transform = ecsManager->GetComponent<Transform>(entity);
			updateLightBuffer(light, light_transform);
		}
		for (auto entity : SceneView<Camera, CameraTransform>(*ecsManager)) {
			auto cam = ecsManager->GetComponent<Camera>(entity);
			//check if the camera can be renderered with meshRenderer
			
			auto camTransform = ecsManager->GetComponent<CameraTransform>(entity);
			PrepareRenderPass(*cam);
			auto frameBuffer = PrepareFrameBuffer(*cam);
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
			auto cameraBuffer = UpdateCameraBuffer(device, *camTransform);
			if (cam->beforeRender) {
				cam->beforeRender(cb, cameraBuffer, info, frameBuffer);
			}
			cb->setViewport(viewPort);
			cb->setScissor(0, scissor);
			RenderWithCamera(cb, info, cameraBuffer);
			if (cam->afterRender) {
				cam->afterRender(cb, cameraBuffer, info, frameBuffer);
			}
			cb->endRenderPass();
		}
	}



	BufferRangeManager* UpdateCameraBuffer(const Device* device, const CameraTransform& trans) {
		auto camera_buffer = requestCameraBufferRange(device);
		camera_buffer->NextRangenoLock();
		uploadCameraMatrix(trans, camera_buffer->buffer().get(), camera_buffer->getOffset());
		return camera_buffer;
	}

	void RenderWorld::RenderWithMaterial(CommandBufferHandle& cb, const RenderInfo& renderInfo, Material* mat, BufferRangeManager* camera_buffer) {
		//auto renderer = meshRenderer;
		{

			mat->SetBuffer("CameraMatrix", camera_buffer->buffer(), camera_buffer->getOffset());
			for (auto entity : SceneView<View>(*ecsManager)) {

				MeshRenderer::DrawWithMaterial(ecsManager,entity,cb, renderInfo, mat);
			}
		}


	}

	void RenderWorld::RenderWithCamera(CommandBufferHandle& cb, const RenderInfo& renderInfo, BufferRangeManager* cameraBuffer) {
		for (auto entity : SceneView<MeshFilter, View>(*ecsManager)) {
			auto view = ecsManager->GetComponent<View>(entity);
			if (!renderInfo.layer.test(static_cast<uint8_t>(view->layer)))continue;
			auto material = view->material;
			material->SetBuffer("CameraMatrix", cameraBuffer->buffer(), cameraBuffer->getOffset());
			if (ecsManager->hasComponent<Transform>(entity)) {
				auto transform = ecsManager->GetComponent<Transform>(entity);
				material->SetValue(ShaderString(SV::_MATRIX_M), transform->updateMtx());
			}
			if (view->recieveShadow) {
			
				material->SetImage("_ShadowMap", collectShadowPass->getCollectedImage(), collectShadowPass->getCollectedImage()->get_view()->get_float_view(), tEngine::StockSampler::NearestShadow);
				material->SetBuffer("LightProperty", collectShadowPass->getFirstLight()->lightPropertyBuffer, collectShadowPass->getFirstLight()->offset);
			}
			MeshRenderer::Draw(ecsManager,entity, cb, renderInfo);
		}

		

	}
}


