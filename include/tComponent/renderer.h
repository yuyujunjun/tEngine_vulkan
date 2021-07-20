#pragma once
#include<memory>
#include"glm/glm.hpp"
#include"Component.h"
namespace tEngine {
	class tShaderInterface;
	class Material;
	class MeshBuffer;
	class CommandBuffer;
	using CommandBufferHandle = std::shared_ptr<CommandBuffer>;
	class tRenderPass;

	/// <summary>
	/// Store some values shared by multiple renderers
	/// </summary>
	struct RenderInfo {
		tRenderPass* renderPass=0;
		unsigned subpass = 0;
		glm::mat4 viewMtx;
		glm::mat4 perspectiveMtx;
		glm::mat4 vpMtx;
	};
	class Renderer:public Component {
	public:
		Renderer(std::shared_ptr<Material>& mat) :material(mat) {}
		Renderer(GameObject_* gameObject, std::shared_ptr<Material>& mat) :material(mat),Component(gameObject) {}
		std::shared_ptr<Material> material;
		void setMaterial(std::shared_ptr<Material>& mat) { material = mat; }
		
		virtual void Draw(CommandBufferHandle& cb,const RenderInfo& renderInfo)const {};
		/// <summary>
		/// Ignore self material and use the given material
		/// </summary>
		/// <param name="cb"></param>
		/// <param name="renderInfo"></param>
		/// <param name="material"></param>
		virtual void DrawWithMaterial(CommandBufferHandle& cb, const RenderInfo& renderInfo, Material* material)const {}
		virtual ~Renderer() {}
	};
	class MeshRenderer:public Renderer {
		
	public:
		//std::shared_ptr<MeshBuffer> meshBuffer;
		unsigned instanceCount = 1;
		MeshRenderer(std::shared_ptr<Material>& mat);
		MeshRenderer(GameObject_* gameObject, std::shared_ptr<Material>& mat);
		void Draw(CommandBufferHandle& cb, const RenderInfo& renderInfo)const override;
		void DrawWithMaterial(CommandBufferHandle& cb, const RenderInfo& renderInfo, Material* material)const override;

	};
}