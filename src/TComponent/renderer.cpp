#include"renderer.h"
#include"Shader.h"
#include"Material.h"
#include"ShaderInterface.h"
#include"MeshBuffer.h"
#include"GameObject.h"
#include"ShaderVariable.h"
#include"ecs.h"

namespace tEngine {

	void MeshRenderer::Draw(CommandBufferHandle& cb, const RenderInfo& renderInfo, const Renderer::BufferMap& bufferMap, const Renderer::ImageMap& imageMap)const {
		for (auto entity : SceneView<MeshFilter,View>(*ecsManager)) {
			auto material =  ecsManager->GetComponent<View>(entity)->material;
			
			auto meshBuffer = ecsManager->GetComponent<MeshFilter>(entity);
			for (const auto& k_v : bufferMap) {
				material->SetBuffer(k_v.first, k_v.second->buffer(), k_v.second->getOffset());
			}
			for (const auto& k_v : imageMap) {
				material->SetImage(k_v.first, k_v.second);
			}
			if (ecsManager->hasComponent<Transform>(entity)) {
				auto transform = ecsManager->GetComponent<Transform>(entity);
				material->SetValue(ShaderString(SV::_MATRIX_M), transform->updateMtx());
			}
		
			material->flushBuffer();
			flushGraphicsShaderState(material->shader, material->graphicsState, cb, renderInfo.renderPass, renderInfo.subpass);
			DrawMesh(meshBuffer, cb, instanceCount);
		}
	
	}
	void MeshRenderer::DrawWithMaterial(CommandBufferHandle& cb, const RenderInfo& renderInfo, Material* material)const {
		for (auto entity : SceneView<Transform, MeshFilter,View>(*ecsManager)) {
			auto transform = ecsManager->GetComponent<Transform>(entity);
			auto meshBuffer = ecsManager->GetComponent<MeshFilter>(entity);
			material->SetValue(ShaderString(SV::_MATRIX_M), transform->updateMtx());
			material->flushBuffer();
			flushGraphicsShaderState(material->shader, material->graphicsState, cb, renderInfo.renderPass, renderInfo.subpass);
			DrawMesh(meshBuffer, cb, instanceCount);
		}
		
	}
}