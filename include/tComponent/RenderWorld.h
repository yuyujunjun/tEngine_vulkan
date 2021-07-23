#pragma once
#include<memory>
#include<vector>
#include"Component.h"
#include"renderer.h"

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

	class Component;

	class RenderWorld {
		
		std::vector<Renderer*> renderers;
		//	RenderEngine renderEngine;
		const Device* device;
	public:
		RenderWorld(const Device* device) :device(device) {};
		
		void RegistryMeshRenderer(const Renderer* obj);
		void RemoveMeshRenderer(const Renderer* obj);
		std::vector<Renderer*>& getRenderers() { return renderers; }
		void renderWithCamera(CommandBufferHandle& cb, const RenderInfo& renderInfo, const CameraTransform* cam);
	protected:
	};
	void RenderWithMaterial(CommandBufferHandle& cb, const RenderInfo& renderInfo, std::vector<Renderer*>& gobjs, Material* mat);
}