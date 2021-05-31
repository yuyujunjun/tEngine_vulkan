#pragma once
#include"MeshBuffer.h"
#include"ShaderInterface.h"
#include"EngineContext.h"
#include"tTransform.h"
#include"tComponent.h"
#include"bufferHelper.h"
namespace tEngine {

	

	class GameObject_ {
	public:
		GameObject_() = default;
		GameObject_(const Mesh& mesh) {
			if (meshbuffer == nullptr) {
				meshbuffer = std::make_shared<MeshBuffer>();
			}
			meshbuffer->setMeshUpload(mesh,tEngineContext::context.device.get());
		}
		void setMesh(const Mesh& mesh) {
			if (meshbuffer == nullptr) {
				meshbuffer = std::make_shared<MeshBuffer>();
			}
			meshbuffer->setMeshUpload(mesh, tEngineContext::context.device.get());
		}
		void setMaterial(std::shared_ptr<tShaderInterface>& shader) {
			material = std::make_shared<Material>(shader);
		}
		Transform transform;
		std::shared_ptr<MeshBuffer> meshbuffer;
		std::shared_ptr<Material> material;
	};
	using GameObject = std::shared_ptr<GameObject_>;
}