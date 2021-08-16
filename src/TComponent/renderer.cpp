#include"renderer.h"
#include"Shader.h"
#include"Material.h"
#include"ShaderInterface.h"
#include"MeshBuffer.h"
#include"ShaderVariable.h"
#include"ecs.h"
#include"tTransform.h"
namespace tEngine {

	 void MeshRenderer::Draw(EcsManager* ecsManager,EntityID entity, CommandBufferHandle& cb, const RenderInfo& renderInfo) {
		auto material = ecsManager->GetComponent<View>(entity)->material;
		auto meshBuffer = ecsManager->GetComponent<MeshFilter>(entity);
		if (material != nullptr && meshBuffer != nullptr&&meshBuffer->getMeshBuffer()!=nullptr) {
			material->flushBuffer();
			flushGraphicsShaderState(material->shader, material->graphicsState, cb, renderInfo.renderPass, renderInfo.subpass);
			DrawMesh(meshBuffer, cb, 1);
		}
	}
	
	void MeshRenderer::DrawWithMaterial(EcsManager* ecsManager, EntityID id, CommandBufferHandle& cb, const RenderInfo& renderInfo, Material* material) {
		auto transform = ecsManager->GetComponent<Transform>(id);
		auto meshBuffer = ecsManager->GetComponent<MeshFilter>(id);
		if (ecsManager->hasComponent<Transform>(id)) {
			material->SetValue(ShaderString(SV::_MATRIX_M), transform->updateMtx());
		}
		material->flushBuffer();
		flushGraphicsShaderState(material->shader, material->graphicsState, cb, renderInfo.renderPass, renderInfo.subpass);
		DrawMesh(meshBuffer, cb, 1);
	}
}