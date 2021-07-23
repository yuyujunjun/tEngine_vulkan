#pragma once
#include<memory>
#include<vector>
#include"Component.h"
#include"renderer.h"
#include"RenderWorld.h"
#include"tPhysics/pWorld.h"
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


	class tWorld {
		RenderWorld renderWorld;
		tPhysics::PhysicsWorld rigidBodyWorld;
		std::vector<GameObject> gameObjects;
		std::vector<System*> systems;
	public:

		tWorld(Device* device) :renderWorld(device) {};// = default;
		void AddGameObject(GameObject& gameObj);
		void AddSystem(System* sys) {
			systems.push_back(sys);
		}
		void update(float dt) {
			for (auto& sys : systems) {
				sys->ExecuteAllComponents(dt);
			}
			rigidBodyWorld.startFrame();
			rigidBodyWorld.runPhysics(dt);
		}
		//void render(CommandBufferHandle cb,const RenderInfo& info){}
		tPhysics::PhysicsWorld& getPhysicsWorld() { return rigidBodyWorld; }
		RenderWorld& getRenderWorld() { return renderWorld; }
	};
}