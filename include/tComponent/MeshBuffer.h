#pragma once
#include"SimpleGeometry.h"
#include"Buffer.h"
#include"Component.h"
namespace tEngine {
	class CommandBuffer;
	using CommandBufferHandle = std::shared_ptr <CommandBuffer>;
	class MeshBuffer:public Component
	{
	public:
		MeshBuffer() {}
		MeshBuffer(GameObject_* gameObject) :Component(gameObject) {}
		void setMesh(const Mesh& mesh) {
			this->mesh = mesh;
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
		void createDynamicBuffer(Device* device, CommandBufferHandle cb, BufferDomain domain = BufferDomain::Host);
		void uploadVertexBuffer(Device* device, CommandBufferHandle cb) {
			updateBufferUsingStageBuffer(device, VBO, cb, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));
			
		}
		void uploadIdxBuffer(Device* device, CommandBufferHandle cb) {
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
		Mesh& getMesh() {
			return mesh;
		}

	private:
		
		Mesh mesh;
		BufferHandle VBO;
		BufferHandle IBO;
	};
	void DrawMesh(MeshBuffer* mb, CommandBufferHandle& cb, uint32_t instanceCount = 1);
}