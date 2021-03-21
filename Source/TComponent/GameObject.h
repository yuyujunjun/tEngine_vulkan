#pragma once
#include"MeshBuffer.h"
#include"ShaderInterface.h"
#include"EngineContext.h"
#include"tTransform.h"
namespace tEngine {
	class GameObject {
		GameObject() = default;
		GameObject(const Mesh& mesh) {
			meshbuffer.setMeshUpload(mesh,tEngineContext::context.device.get());
		}
		void setMesh(const Mesh& mesh) {
			meshbuffer.setMeshUpload(mesh, tEngineContext::context.device.get());
		}
		void setMaterial(std::shared_ptr<tShaderInterface>& material) {
			this->material = material;
		}
		Transform transform;
		MeshBuffer meshbuffer;
		std::shared_ptr<tShaderInterface> material;
	};
}