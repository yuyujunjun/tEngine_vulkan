#include "MeshBuffer.h"
#include"CommandBufferBase.h"
#include"Log.h"
namespace tEngine {
	void MeshBuffer::Upload(Device* device) {
		if (mesh.vertices.size() == 0)return;
		auto createUpload = [&](CommandBufferHandle cb) {
			if (!createVertexBuffer(device, cb)) {
				uploadVertexBuffer(device, cb);
			}
			if (!createIdxBuffer(device, cb)) {
				uploadIdxBuffer(device, cb);
			}
		};
		//if commandbuffer is not requirement.
		if (VBO && IBO && VBO->getCreateInfo().domain == IBO->getCreateInfo().domain && IBO->getCreateInfo().domain == BufferDomain::Host) {
			createUpload(nullptr);
		}
		else {
			oneTimeSubmit(device, createUpload);
		}
	}
	void MeshFilter::setMeshUpload(const Mesh& mesh, Device* device) {
		setMesh(mesh);
		meshBuffer->Upload(device);
	}
	void DrawMesh(MeshFilter* mb, CommandBufferHandle& cb, uint32_t instanceCount ) {
		cb->bindVertexBuffer(mb->getVBO(), 0, 0);
		if (mb->getIBO() != nullptr) {
			cb->bindIndexBuffer(mb->getIBO(), 0, vk::IndexType::eUint32);
			cb->drawIndexed(0, mb->getMesh().indices.size(), 0, 0, instanceCount);
		}
		else {
			cb->draw(0, mb->getMesh().vertices.size(), 0, instanceCount);
		}
	}
	int MeshBuffer::createVertexBuffer(Device* device, CommandBufferHandle cb, BufferDomain domain ) {
		assert(mesh.vertices.size() > 0);
		if (VBO && VBO->getSize() >= mesh.vertices.size() * sizeof(Vertex)&&VBO->getCreateInfo().domain==domain) { 
			//LOGD(tEngine::LogLevel::Warning, "No need to create Vertex buffer, no operations. Call uploadVertexBuffer to upload data.");
			return 0;
		}
		BufferCreateInfo info;
		info.domain = domain;
		info.size = mesh.vertices.size() * sizeof(Vertex);
		info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VBO = createBuffer(device, info, mesh.vertices.data(), cb);
		return 1;
	}
	int MeshBuffer::createIdxBuffer(Device* device, CommandBufferHandle cb, BufferDomain domain ) {
		if (mesh.indices.size() == 0)return 0;
		if (IBO && IBO->getSize() >= mesh.indices.size() * sizeof(uint32_t) && IBO->getCreateInfo().domain == domain) {
			//LOG(tEngine::LogLevel::Warning, "No need to create Index buffer, no operations. Call uploadIdxBuffer to upload data.");
			return 0;
		}
		BufferCreateInfo info;
		info.domain = domain;
		info.size = mesh.indices.size() * sizeof(uint32_t);
		info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		IBO = createBuffer(device, info, mesh.indices.data(), cb);
		return 1;
	}
	int MeshFilter::createVertexBuffer(Device* device, CommandBufferHandle cb, BufferDomain domain ) {
		return meshBuffer->createVertexBuffer(device, cb, domain);
	}
	int MeshFilter::createIdxBuffer(Device* device, CommandBufferHandle cb, BufferDomain domain ) {
		return meshBuffer->createIdxBuffer(device, cb, domain);
	}
}