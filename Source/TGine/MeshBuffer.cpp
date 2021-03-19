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
	void MeshBuffer::createVertexBuffer(Device* device, CommandBufferHandle& cb, BufferDomain domain ) {
		assert(mesh.vertices.size() > 0);
		if (VBO && VBO->getSize() > mesh.vertices.size() * sizeof(Vertex)) { return; }
		BufferCreateInfo info;
		info.domain = domain;
		info.size = mesh.vertices.size() * sizeof(Vertex);
		info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VBO = createBuffer(device, info, mesh.vertices.data(), cb);
	}
	void MeshBuffer::createIdxBuffer(Device* device, CommandBufferHandle& cb, BufferDomain domain ) {
		if (mesh.indices.size() == 0)return;
		if (IBO && IBO->getSize() >= mesh.indices.size() * sizeof(uint32_t)) { return; }
		BufferCreateInfo info;
		info.domain = domain;
		info.size = mesh.indices.size() * sizeof(uint32_t);
		info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		IBO = createBuffer(device, info, mesh.indices.data(), cb);
	}
}