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
	class tWorld {
		std::vector<GameObject> gameObjects;
		std::vector<System*> systems;
		std::vector<Renderer*> renderers;
	//	RenderEngine renderEngine;
		const Device* device;
	public:
		tWorld(const Device* device) :device(device) {};
		void AddSystem(System* sys) {
			systems.push_back(sys);
		}
		void RegistryMeshRenderer(const GameObject& obj);
		void RemoveMeshRenderer(const GameObject& obj);
		void update(float dt) {
			for (auto& sys : systems) {
				sys->ExecuteAllComponents(dt);
			}
			
		}
		std::vector<Renderer*>& getRenderers() { return renderers; }
		void renderWithCamera(CommandBufferHandle& cb, const RenderInfo& renderInfo,const CameraTransform* cam);
	protected:
		

	};
	void RenderWithMaterial(CommandBufferHandle& cb, const RenderInfo& renderInfo, std::vector<Renderer*>& gobjs, Material* mat);
	
}