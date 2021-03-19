#pragma once
#include"MeshBuffer.h"
#include"ShaderInterface.h"
#include"EngineContext.h"
namespace tEngine {
	class GameObject {
		GameObject() = default;
		GameObject(const Mesh& mesh) {
			meshbuffer.setMeshUpload(mesh,tEngineContext::context.device.get());
		}
		void setMesh(const Mesh& mesh) {
			meshbuffer.setMeshUpload(mesh, tEngineContext::context.device.get());
		}
		MeshBuffer meshbuffer;
		std::shared_ptr<tShaderInterface> material;
	};
}