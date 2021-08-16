#include"CollectShadowPass.h"
#include"Shader.h"
#include"Material.h"
#include"FrameBuffer.h"
#include"renderHelper.h"
#include"Image.h"
#include"glm.hpp"
#include"Buffer.h"
#include"ecs.h"
#include"AABB.h"
#include"CommandBufferBase.h"
#include"renderer.h"
#include"tTransform.h"
#include"Camera.h"
#include"Light.h"
namespace tEngine {
	CollectShadowPass::CollectShadowPass(const Device* device, EcsManager* scene) {
		ecsManager = scene;
		this->device = device;
	//	renderer = std::make_shared<MeshRenderer>();

		if (shadowMaterial == nullptr) {
			shadowShader = tShader::Create(device);
			shadowShader->SetShaderModule({ "ShadowPass.vsh","ShadowPass.fsh" }, { vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment });
			shadowMaterial = std::make_shared<Material>(shadowShader.get());
			GraphicsState shadowMapState;
			shadowMapState.depthBias.depthBiasEnable = true;
			shadowMapState.depthBias.depthBiasConstantFactor = 1e5;
			shadowMapState.depthBias.depthBiasSlopeFactor = 1;
			shadowMapState.depthBias.depthBiasClamp = 0.2;
			shadowMaterial->graphicsState = shadowMapState;
		}
		if (renderPass == nullptr) {
			renderPass = getCollectShadowPass(device, vk::Format::eR32Sfloat);
			
		}
		
	}

	void CollectShadowPass::collectShadow( CommandBufferHandle& cb) {
		for (auto entity : SceneView<Light>(*ecsManager)) {
			auto light = ecsManager->GetComponent<Light>(entity);

			auto light_transform = ecsManager->GetComponent<Transform>(entity);

			auto inverser_view = light_transform->getInverseMat3();
			//auto inverser_view = glm::inverse(view_mtx);
			AABB box;
			std::vector<EntityID> objs;
			for (auto obj : SceneView<AABB, View>(*ecsManager)) {
				auto view = ecsManager->GetComponent<View>(obj);
				if (view->castingShadow) {
					auto bounds = ecsManager->GetComponent<AABB>(obj);
					//auto center = inverser_view * bounds->getCenter();
					auto center = Vector3(0, 0, 0);
					auto halfSize = absMat( inverser_view )* bounds->getHalfSize();
					AABB empty;
					empty.setCenter(center);
					empty.setHalfSize(glm::abs(halfSize));
					empty.EXPAND(glm::vec3(0.5, 0.5,1 ));

					box.ADD(&empty);
					objs.emplace_back(obj);
				}
			}
			renderPass->SetImageView("back", light->shadowMap);
			renderPass->setTransientImageView("depth");
			renderPass->setClearValue("back", { 0,0,0,1 });
			renderPass->setDepthStencilValue("depth", 1);
			auto frameBuffer = renderPass->requestFrameBuffer();
			cb->beginRenderPass(renderPass, frameBuffer, true);
			auto viewPort = frameBuffer->getViewPort();
			auto scissor = frameBuffer->getRenderArea();
			RenderInfo info;
			info.renderPass = renderPass.get();
			info.subpass = 0;
			cb->setViewport(viewPort);
			cb->setScissor(0, scissor);
			
			
			auto projection = glm::ortho(box.MinCoords().x, box.MaxCoords().x, box.MinCoords().y, box.MaxCoords().y,box.MinCoords().z,box.MaxCoords().z);
			RenderInfo renderInfo;
			renderInfo.renderPass = renderPass.get();
			renderInfo.subpass = 0;
			auto camera_buffer = requestCameraBufferRange(device);
			camera_buffer->NextRangenoLock();
			uploadCameraMatrix(inverser_view, projection, light_transform->getPosition(),camera_buffer->buffer().get(), camera_buffer->getOffset());
			light->world_to_shadow = projection * glm::mat4(inverser_view);
			shadowMaterial->SetBuffer("CameraMatrix",camera_buffer->buffer(),camera_buffer->getOffset());
			for (auto obj : objs) {
				MeshRenderer::DrawWithMaterial(ecsManager,obj, cb, info, shadowMaterial.get());
			}
			cb->endRenderPass();
		}
	}
	ImageHandle CollectShadowPass::getCollectedImage() {
		for (auto entity : SceneView<Light>(*ecsManager)) {
			auto light = ecsManager->GetComponent<Light>(entity);
			return light->shadowMap;
		}
		assert(false&&"No Light");
		return nullptr;
	}
	Light* CollectShadowPass::getFirstLight() {
		for (auto entity : SceneView<Light>(*ecsManager)) {
			auto light = ecsManager->GetComponent<Light>(entity);
			return light;
		}assert(false && "No Light");
		return nullptr;
	}


}