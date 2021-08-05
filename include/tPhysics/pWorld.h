#pragma once
#include"numerical.h"
#include"pForceGen.h"
#include"pContacts.h"
#include"ecs.h"
#include<list>
#include"RigidBody.h"
namespace tEngine {
	class Particle;

	class ParticleWorld:public System {
		using ContactGenerators = std::vector<ParticleContactGenerator*>;
		std::list<Particle*> pList;
		ParticleForceRegistry registry;//force generator and particle
		ContactGenerators contactGenerators;//contact generator
		std::vector<ParticleContact> contacts;//contact list
		unsigned maxContacts;
		ParticleContactResolver resolver;


	public:
		bool calculateIterations=true;
		ParticleWorld(unsigned maxContacts, unsigned iterations = 0)  ;
		~ParticleWorld();
		void addParticle(Particle* particle);
		void removeParticle(Particle* particle);
		void startFrame();
		unsigned generateContacts();
		void integrate(real duration);
		void runPhysics(real duration);
		ContactGenerators& getContactGenerators() { return contactGenerators; }
		ParticleForceRegistry& getForceRegistry() { return registry; }
	};
	class Collider;
	class PhysicsWorld:public System {
		//std::list<RigidBody*> rigidBodys;
		std::vector<ForceRegistration> forceRegistration;
		RigidBodySystem rigidBodySystem;
	public:
		PhysicsWorld() = default;
	//	void addRigidBody(RigidBody* body) { rigidBodys.emplace_back(body); }
	//	void removeRigidBody(RigidBody* body) { rigidBodys.remove(body); }
		void startFrame();
		void registerForce(ForceGenerator* force,EntityID id);
		void unregisterForce(ForceGenerator* force, EntityID id);
		void runPhysics(real duration);
		
	};
	/// <summary>
	/// First detect contact using AABB
	/// </summary>
	struct PotentialContact {
		Collider* obj1;
		Collider* obj2;
	};
	class ContactWorld {
		std::list<Collider*> colliders;
		std::list<PotentialContact> broadContacts;
		

	};
}