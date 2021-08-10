#pragma once
#include<memory>
#include"glm/glm.hpp"

#include"RenderLayers.h"
#include"ecs.h"
namespace tEngine {
	class tShaderInterface;
	class Material;
	class MeshFilter;
	class CommandBuffer;
	using CommandBufferHandle = std::shared_ptr<CommandBuffer>;
	class tRenderPass;
	class Renderer;
	class BufferRangeManager;
	class tImage;
	using ImageHandle = std::shared_ptr<tImage>;
	/// <summary>
	/// Store some values shared by multiple renderers
	/// </summary>
	struct RenderInfo {
		tRenderPass* renderPass=0;
		unsigned subpass = 0;
		/// <summary>
		/// A renderer only be rendered if its layer match RenderInfo
		/// </summary>
		std::bitset<8> layer;
		/// <summary>
		/// Condition if a renderer need to be rendered
		/// </summary>
	//	std::function<bool(Renderer* renderer)> isRender=nullptr;
		//glm::mat4 viewMtx;
		//glm::mat4 perspectiveMtx;
		//glm::mat4 vpMtx;
	};
	
	class Renderer:public System {
		

	public:
		using BufferMap = std::vector<std::pair<std::string, BufferRangeManager*>>;
		using ImageMap = std::vector<std::pair<std::string, ImageHandle>>;


		Renderer() {}
		//Renderer(GameObject_* gameObject, std::shared_ptr<Material>& mat) :material(mat), recieveShadow(true), shadowCastingMode(ShadowCastingMode::On), layer(RenderLayer::Default) {}
		//std::shared_ptr<Material> material;
	//	void setMaterial(std::shared_ptr<Material>& mat) { material = mat; }
		/// <summary>
		/// Draw Function: Must execute after setting all material varibles, except self transform             
		/// </summary>
		/// <param name="cb"></param>
		/// <param name="renderInfo"></param>
		virtual void Draw(EntityID id, CommandBufferHandle& cb,const RenderInfo& renderInfo)const {};
		/// <summary>
		/// Ignore self material and use the given material to draw
		/// </summary>
		/// <param name="cb"></param>
		/// <param name="renderInfo"></param>
		/// <param name="material"></param>
	//	virtual void DrawWithMaterial(CommandBufferHandle& cb, const RenderInfo& renderInfo, Material* material)const {}
		virtual void DrawWithMaterial(EntityID id, CommandBufferHandle& cb, const RenderInfo& renderInfo, Material* material)const {}
		virtual ~Renderer() {}
	};
	class MeshRenderer:public Renderer {
		
	public:

		//std::shared_ptr<MeshFilter> meshBuffer;
		unsigned instanceCount = 1;
		void Draw(EntityID id, CommandBufferHandle& cb, const RenderInfo& renderInfo)const override;
	//	void DrawWithMaterial(CommandBufferHandle& cb, const RenderInfo& renderInfo, Material* material)const override;
		void DrawWithMaterial(EntityID id, CommandBufferHandle& cb, const RenderInfo& renderInfo, Material* material)const override;

	};
	
}