#pragma once
#include<memory>
#include<vector>
#include"renderer.h"
#include"RenderWorld.h"
#include"tPhysics/pWorld.h"
#include"ecs.h"
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
		EcsManager ecsManager;
		RenderWorld renderWorld;
		PhysicsWorld rigidBodyWorld;
		std::vector<System*> systems;
	public:
		EcsManager& getEcsManager() { return ecsManager; }
		tWorld(Device* device) :renderWorld(device) {
			renderWorld.ecsManager = &ecsManager;
			rigidBodyWorld.ecsManager = &ecsManager;
			renderWorld.meshRenderer->ecsManager = &ecsManager;
		
		};// = default;
		void AddSystem(System* sys) {
			systems.push_back(sys);
		}
		void update(float dt) {
			updateTransform();
			updateAABB();
			rigidBodyWorld.startFrame();
			rigidBodyWorld.runPhysics(dt);
		}
		//void render(CommandBufferHandle cb,const RenderInfo& info){}
		PhysicsWorld& getPhysicsWorld() { return rigidBodyWorld; }
		RenderWorld& getRenderWorld() { return renderWorld; }
		void updateTransform();
		void updateAABB();
	
	};
	EntityID createCamera(EcsManager* ecsManager, glm::uvec2 screenSize);
	EntityID createGameObject(EcsManager* ecsManager);
}