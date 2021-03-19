#pragma once
#include"Tgine.h"
#include"SimpleGeometry.h"
#include"tResource.h"
namespace tEngine {
	class MeshBuffer
	{
	public:
		MeshBuffer() {}
		void setMesh(const Mesh& mesh) {
			this->mesh = mesh;
		}
		void setMeshUpload(const Mesh& mesh,Device* device) {
			setMesh(mesh);
			createBuffers(device,nullptr);
		}
		void createBuffers(Device* device, CommandBufferHandle cb, BufferDomain domain = BufferDomain::Device) {
			createVertexBuffer(device,cb,domain);
			createIdxBuffer(device,cb,domain);
		}
		void createVertexBuffer(Device* device, CommandBufferHandle& cb, BufferDomain domain = BufferDomain::Device) {
			assert(mesh.vertices.size() > 0);
			if (VBO && VBO->getSize() > mesh.vertices.size() * sizeof(Vertex)) { return; }
			BufferCreateInfo info;
			info.domain = domain;
			info.size = mesh.vertices.size() * sizeof(Vertex);
			info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			VBO = createBuffer(device,info, mesh.vertices.data(), cb);
		}
		void createIdxBuffer(Device* device, CommandBufferHandle& cb, BufferDomain domain = BufferDomain::Device) {
			if (mesh.indices.size() == 0)return;
			if (IBO && IBO->getSize() >= mesh.indices.size() * sizeof(uint32_t)) { return; }
			BufferCreateInfo info;
			info.domain = domain;
			info.size = mesh.indices.size() * sizeof(uint32_t);
			info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			IBO = createBuffer(device,info, mesh.indices.data(), cb);
		}
		void uploadVertexBuffer(Device* device, CommandBufferHandle& cb) {
			updateBufferUsingStageBuffer(device, VBO, cb, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));
			
		}
		void uploadIdxBuffer(Device* device, CommandBufferHandle& cb) {
			updateBufferUsingStageBuffer(device, IBO, cb, mesh.indices.data(), mesh.indices.size() * sizeof(uint32_t));
		}
		const BufferHandle& getVBO()const {
			return VBO;
		}
		const BufferHandle& getIBO()const {
			return IBO;
		}
		const Mesh& getMesh()const {
			return mesh;
		}
	private:
		
		Mesh mesh;
		BufferHandle VBO;
		BufferHandle IBO;
	};
	void DrawMesh(MeshBuffer* mb, CommandBufferHandle& cb, uint32_t instanceCount = 1);
}