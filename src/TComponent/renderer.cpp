#include"renderer.h"
#include"Shader.h"
#include"Material.h"
#include"ShaderInterface.h"
#include"MeshBuffer.h"
#include"GameObject.h"
#include"ShaderVariable.h"
namespace tEngine {
	MeshRenderer::MeshRenderer(std::shared_ptr<Material>& mat) :Renderer(mat) {
	}
	MeshRenderer::MeshRenderer(GameObject_* gameObject, std::shared_ptr<Material>& mat):Renderer(gameObject,mat) {

	}
	void MeshRenderer::Draw(CommandBufferHandle& cb, const RenderInfo& renderInfo)const {
		DrawWithMaterial(cb, renderInfo, material.get());
	}
	void MeshRenderer::DrawWithMaterial(CommandBufferHandle& cb, const RenderInfo& renderInfo, Material* material)const {
		material->SetValue(ShaderString(SV::_MATRIX_M), gameObject->transform.updateMtx());
		material->flushBuffer();
		flushGraphicsShaderState(material->shader, material->graphicsState, cb, renderInfo.renderPass, renderInfo.subpass);
		auto meshBuffer=gameObject->getComponent<MeshBuffer>();
		DrawMesh(meshBuffer, cb, instanceCount);
	}
}