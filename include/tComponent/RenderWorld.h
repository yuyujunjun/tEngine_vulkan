#pragma once
#include<memory>
#include<vector>
#include"ecs.h"
#include"renderer.h"
#include"renderHelper.h"
#include"CollectShadowPass.h"
namespace tEngine {
	class System;
	struct RenderInfo;
	class Device;
	class Renderer;
	class Material;
	class CommandBuffer;
	struct CameraTransform;
	using CommandBufferHandle = std::shared_ptr <CommandBuffer>;
	class Camera;
	class Component;
	class tRenderPass;
	using RenderPassHandle = std::shared_ptr<tRenderPass>;
	class tFrameBuffer;
	using FrameBufferHandle = std::shared_ptr<tFrameBuffer>;
	class BufferRangeManager;
	struct Light;
	BufferRangeManager* UpdateCameraBuffer(const Device* device,const CameraTransform& trans);
	
	class RenderWorld:public System {
		
		const Device* device;
		ForwardRenderPass forwardRenderPass;
	
		std::shared_ptr<CollectShadowPass>  collectShadowPass;
		EntityID mainCamera;
	public:
	//	MeshRenderer* meshRenderer;
		RenderWorld(const Device* device) :device(device),mainCamera(-1){
		
		};
		void SetCamera(EntityID cam) { mainCamera = cam; }
	
		~RenderWorld(){
		//	delete meshRenderer;
		}
		//std::vector<Renderer*>& getRenderers() { return renderers; }

		void Render(CommandBufferHandle& cb, const SwapChainHandle& swapChain, uint32_t idx);
		/// <summary>
		/// Ignore objects' self material and use a specific material to draw
		/// 
		/// </summary>
		/// <param name="cb"></param>
		/// <param name="renderInfo"></param>
		/// <param name="mat"></param>
		/// <param name="view"></param>
		/// <param name="projection"></param>
		void RenderWithMaterial(CommandBufferHandle& cb, const RenderInfo& renderInfo,  Material* mat, BufferRangeManager* cameraBuffer);
		
	protected:
		
		void updateMainCameraRT(const SwapChainHandle& swapChain, uint32_t idx, Camera& camera);
		void PrepareRenderPass(Camera& camera);
		FrameBufferHandle PrepareFrameBuffer(Camera& camera);
		void RenderWithCamera(CommandBufferHandle& cb, const RenderInfo& renderInfo, BufferRangeManager* cameraBuffer);
		
	};
	
}