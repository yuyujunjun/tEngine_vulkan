#pragma once
#include"SimpleGeometry.h"
#include"Buffer.h"
#include"Component.h"
namespace tEngine {
	class CommandBuffer;
	using CommandBufferHandle = std::shared_ptr <CommandBuffer>;
	class MeshBuffer {
		
	public:

		void setMeshUpload(Device* device);
		void setMesh(const Mesh& mesh) {
			this->mesh = mesh;
		}
		int createIdxBuffer(Device* device, CommandBufferHandle cb, BufferDomain domain = BufferDomain::Device);
		int createVertexBuffer(Device* device, CommandBufferHandle cb, BufferDomain domain = BufferDomain::Device);
		void uploadVertexBuffer(Device* device, CommandBufferHandle cb) {
			updateBufferUsingStageBuffer(device, VBO, cb,mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));

		}
		void uploadIdxBuffer(Device* device, CommandBufferHandle cb) {
			updateBufferUsingStageBuffer(device, IBO, cb, mesh.indices.data(), mesh.indices.size() * sizeof(uint32_t));
		}
		Mesh mesh;
		BufferHandle VBO;
		BufferHandle IBO;
	};
	class MeshFilter
	{
		
	public:
		MeshFilter() {}

		void setMesh(const Mesh& mesh) {
			this->meshBuffer = std::make_shared<MeshBuffer>();
			this->meshBuffer->setMesh(mesh);

		}
		void setMeshUpload(const Mesh& mesh, Device* device);
		void createBuffers(Device* device, CommandBufferHandle cb, BufferDomain domain = BufferDomain::Device) {
			createVertexBuffer(device,cb,domain);
			createIdxBuffer(device,cb,domain);
		}
		/// <summary>
		/// return 1 if create buffer and upload successful, return 0 if no operations made
		/// </summary>
		/// <param name="device"></param>
		/// <param name="cb"></param>
		/// <param name="domain"></param>
		/// <returns></returns>
		int createVertexBuffer(Device* device, CommandBufferHandle cb, BufferDomain domain = BufferDomain::Device);
		int createIdxBuffer(Device* device, CommandBufferHandle cb, BufferDomain domain = BufferDomain::Device);
	
		void uploadVertexBuffer(Device* device, CommandBufferHandle cb) {
			meshBuffer->uploadVertexBuffer(device,cb);
			
		}
		void uploadIdxBuffer(Device* device, CommandBufferHandle cb) {
			meshBuffer->uploadIdxBuffer(device, cb);

		}
		const BufferHandle& getVBO()const {
			return meshBuffer->VBO;
		}
		const BufferHandle& getIBO()const {
			return meshBuffer->IBO;
		}
		const Mesh& getMesh()const {
			return meshBuffer->mesh;
		}
		Mesh& getMesh() {
			return meshBuffer->mesh;
		}

	private:
		
		std::shared_ptr<MeshBuffer> meshBuffer;
	};
	void DrawMesh(MeshFilter* mb, CommandBufferHandle& cb, uint32_t instanceCount = 1);
}