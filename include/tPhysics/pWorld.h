#pragma once
#include"numerical.h"
#include"pForceGen.h"
#include"pContacts.h"
#include"ecs.h"
#include<list>
#include"RigidBody.h"
#include"pForceGen.h"
#include"collide.h"
#include"contacts.h"
namespace tEngine {
	class Particle;
	class BoxColliderSystem;
	class MeshColliderSystem;
	class SphereColliderSystem;
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

	class ContactWorld{
	public:
		std::vector<GJKSupportCollider> gjkSupportObj;
	//	std::vector<ContactInfo> contactlist;
		//std::list<PotentialContact> broadContacts;
		//	BoxColliderSystem boxSystem;
	//	MeshColliderSystem meshSystem;
	//	SphereColliderSystem sphereSystem;
		void colliderDetect(EcsManager* ecsManager,std::vector<Contact>& contacts);
	};
	class PhysicsWorld:public System {
		ContactWorld contactWorld;
		ContactResolver resolver;
		//std::list<RigidBody*> rigidBodys;
		std::vector<ForceRegistration> forceRegistration;
		
	public:
		PhysicsWorld() = default;
	//	void addRigidBody(RigidBody* body) { rigidBodys.emplace_back(body); }
	//	void removeRigidBody(RigidBody* body) { rigidBodys.remove(body); }
		void startFrame();
		void registerForce(ForceGenerator* force);
		void unregisterForce(ForceGenerator* force, EntityID id);
		void runPhysics(real duration);
		
	};
	/// <summary>
	/// First detect contact using AABB
	/// </summary>
	struct PotentialContact {
		EntityID obj1;
		EntityID obj2;
	};

	
}