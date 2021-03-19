#include "MeshBuffer.h"
#include"CommandBufferBase.h"
namespace tEngine {
	void DrawMesh(MeshBuffer* mb, CommandBufferHandle& cb, uint32_t instanceCount ) {
		cb->bindVertexBuffer(mb->getVBO(), 0, 0);
		if (mb->getIBO() != nullptr) {
			cb->bindIndexBuffer(mb->getIBO(), 0, vk::IndexType::eUint32);
			cb->drawIndexed(0, mb->getMesh().indices.size(), 0, 0, instanceCount);
		}
		else {
			cb->draw(0, mb->getMesh().vertices.size(), 0, instanceCount);
		}
	}
}