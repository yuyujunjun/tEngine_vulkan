#pragma once
#include<memory>
#include"ecs.h"
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
	class tImage;
	using ImageHandle = std::shared_ptr<tImage>;
	class MeshRenderer;
	class Light;
	class tShader;
	struct CollectShadowPass:public System {
		RenderPassHandle renderPass;
		
		std::shared_ptr<Material> shadowMaterial;
		/// <summary>
		/// only support the first light has the shadowMap
		/// </summary>
		/// <returns></returns>
		ImageHandle getCollectedImage();
		Light* getFirstLight();
		std::shared_ptr<tShader> shadowShader;
		CollectShadowPass(const Device* device,EcsManager* ecsManager);
		void collectShadow(Renderer* renderer,CommandBufferHandle& cb);
		const Device* device;
	};
}