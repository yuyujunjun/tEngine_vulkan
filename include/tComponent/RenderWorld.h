#pragma once
#include<memory>
#include<vector>
#include"Component.h"
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
	class RenderWorld {
		
		std::vector<Renderer*> renderers;
		//	RenderEngine renderEngine;
		const Device* device;
		ForwardRenderPass forwardRenderPass;
		std::vector<Camera*> cameras;
		Camera* mainCamera;
	public:
		
		RenderWorld(const Device* device) :device(device){};
		void AddCamera(Camera* cam);// { cameras.emplace_back(cam); }
		void RemoveCamera(Camera* cam);
		void RegistryMeshRenderer(Renderer* obj);
		void RemoveMeshRenderer(const Renderer* obj);
		std::vector<Renderer*>& getRenderers() { return renderers; }
		void Render(CommandBufferHandle& cb, const SwapChainHandle& swapChain, uint32_t idx);
		void RenderWithMaterial(CommandBufferHandle& cb, const RenderInfo& renderInfo, std::vector<Renderer*>& gobjs, Material* mat, Camera* cam);
	protected:
		void updateMainCameraRT(const SwapChainHandle& swapChain, uint32_t idx, Camera& camera);
		void PrepareRenderPass(Camera& camera);
		FrameBufferHandle PrepareFrameBuffer(Camera& camera);
		void RenderWithCamera(CommandBufferHandle& cb, const RenderInfo& renderInfo, std::vector<Renderer*>& renderers, Camera* camera);
	};
	
}