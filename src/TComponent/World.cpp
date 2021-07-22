#include"World.h"
#include"GameObject.h"
#include"renderer.h"
#include"Material.h"
#include"Camera.h"
#include"Buffer.h"
#include"CommandBufferBase.h"

namespace tEngine {
	void tWorld::RegistryMeshRenderer(const GameObject& obj) {
		renderers.emplace_back(obj->getComponent<MeshRenderer>());
	}
	void tWorld::RemoveMeshRenderer(const GameObject& obj) {
		for (auto it = renderers.begin(); it != renderers.end(); ++it) {
			if ((*it)->gameObject->Identity()==obj->Identity()) {
				renderers.erase(it);
			}
		}
	}
	
	void tWorld::renderWithCamera(CommandBufferHandle& cb, const RenderInfo& renderInfo,const  CameraTransform* cam) {
		auto bufferRange=requestCameraBufferRange(device);
		bufferRange->NextRangenoLock();
		uploadCameraMatrix(cam->m_matrix, cam->p_matrix, bufferRange->buffer().get(), bufferRange->getOffset());
		for (auto& renderer : renderers) {
			renderer->material->SetBuffer("CameraMatrix", bufferRange->buffer(), bufferRange->getOffset());
			renderer->material->SetValue(ShaderString(SV::_MATRIX_M), renderer->gameObject->transform.updateMtx());
			renderer->material->flushBuffer();
			renderer->Draw(cb, renderInfo);
		}
	}
	void RenderWithMaterial(CommandBufferHandle& cb, const RenderInfo& renderInfo, std::vector<Renderer*>& gobjs, Material* mat) {
		for (auto& renderer : gobjs) {
			mat->SetValue(ShaderString(SV::_MATRIX_M), renderer->gameObject->transform.updateMtx());
			mat->flushBuffer();
			renderer->DrawWithMaterial(cb, renderInfo, mat);
		}
	}
}