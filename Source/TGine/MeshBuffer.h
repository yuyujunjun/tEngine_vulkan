#pragma once
#include"SimpleGeometry.h"
#include"tBuffer.h"
namespace tEngine {
	class CommandBuffer;
	using CommandBufferHandle = std::shared_ptr <CommandBuffer>;
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
		void createVertexBuffer(Device* device, CommandBufferHandle& cb, BufferDomain domain = BufferDomain::Device);
		void createIdxBuffer(Device* device, CommandBufferHandle& cb, BufferDomain domain = BufferDomain::Device);
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