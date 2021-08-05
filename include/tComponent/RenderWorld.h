#pragma once
#include<memory>
#include<vector>
#include"ecs.h"
#include"renderer.h"
#include"renderHelper.h"
namespace tEngine {
	class System;
	struct RenderInfo;
	class Device;
	class Renderer;
	class Material;
	class CommandBuffer;
	class GameObject_;
	using GameObject = std::shared_ptr<GameObject_>;
	struct CameraTransform;
	using CommandBufferHandle = std::shared_ptr <CommandBuffer>;
	class Camera;
	class Component;
	class tRenderPass;
	using RenderPassHandle = std::shared_ptr<tRenderPass>;
	class tFrameBuffer;
	using FrameBufferHandle = std::shared_ptr<tFrameBuffer>;
	class BufferRangeManager;
	BufferRangeManager* UpdateCameraBuffer(const Device* device,const Camera* camera);
	BufferRangeManager* UpdateCameraBuffer(const Device* device, const glm::mat4& view, const glm::mat4& projection);
	class RenderWorld:public System {
		
		//std::vector<EntityID> renderers;
		//	RenderEngine renderEngine;
		const Device* device;
		ForwardRenderPass forwardRenderPass;
		//std::vector<Camera*> cameras;
		EntityID mainCamera;
		std::vector<Renderer*> renderers;
	public:
		
		RenderWorld(const Device* device) :device(device),mainCamera(-1){
			
		
		};
		void AddCamera(EntityID cam) { mainCamera = cam; }
		void AddRenderer(Renderer* renderer) {
			renderers.emplace_back(renderer);
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
		void RenderWithMaterial(std::vector<Renderer*>& renderer,CommandBufferHandle& cb, const RenderInfo& renderInfo,  Material* mat, BufferRangeManager* cameraBuffer);
		
	protected:
		
		void updateMainCameraRT(const SwapChainHandle& swapChain, uint32_t idx, Camera& camera);
		void PrepareRenderPass(Camera& camera);
		FrameBufferHandle PrepareFrameBuffer(Camera& camera);
		void RenderWithCamera(std::vector<Renderer*>& renderer,CommandBufferHandle& cb, const RenderInfo& renderInfo, BufferRangeManager* cameraBuffer);
		
	};
	
}