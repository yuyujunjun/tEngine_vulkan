#include"pWorld.h"
#include"tParticles.h"
#include"RigidBody.h"
#include"ecs.h"
#include"collide.h"
#include"contacts.h"
#include<iostream>
#include"Log.h"
#include"fcl/narrowphase/collision.h"
namespace tEngine {
	ParticleWorld::ParticleWorld(unsigned maxContacts, unsigned iterations):maxContacts(maxContacts),resolver(iterations) {
		contacts.resize(maxContacts);
		calculateIterations = (iterations == 0);
	}
	ParticleWorld::~ParticleWorld() {
		
	}
	void ParticleWorld::startFrame() {
		for (auto& particle : pList) {
			particle->clearAccumulator();
		}
	}
	void ParticleWorld::addParticle(Particle* particle) {
		pList.push_back(particle);
	}
	void ParticleWorld::removeParticle(Particle* particle) {
		pList.remove(particle);
	}
	unsigned ParticleWorld::generateContacts() {
		unsigned limit = maxContacts;
		//ParticleContact* nextContact = contacts.data();
		unsigned nextContact = 0;
		for (const auto& generator : contactGenerators) {
			unsigned used=generator->addContact(&contacts[nextContact], limit);
			limit -= used;
			nextContact += used;
			if (limit <= 0)break;
		}
		return maxContacts - limit;
	}
	void ParticleWorld::integrate(real duration) {
		for (auto& p : pList) {
			p->integrate(duration);
		}
	}
	void ParticleWorld::runPhysics(real duration) {
		registry.updateForces(duration);
		integrate(duration);
		unsigned usedContacts = generateContacts();
		if (usedContacts) {
			if (calculateIterations)resolver.setInterations(usedContacts * 2);
			resolver.resolveContacts(&contacts[0],usedContacts,duration);
		}
	}
	void PhysicsWorld::startFrame() {
		
		for (auto id : SceneView<RigidBody>(*ecsManager)) {
			auto body = ecsManager->GetComponent<RigidBody>(id);
			body->clearAccumulator();
			body->calculateDerivedData(ecsManager->GetComponent<Transform>(id));
		}
	}
	void PhysicsWorld::registerForce(ForceGenerator* force) {
		forceRegistration.emplace_back(force);
	}
	void PhysicsWorld::unregisterForce(ForceGenerator* force, EntityID id) {
		for (int i = forceRegistration.size() - 1; i >= 0; --i) {
			if (forceRegistration[i].fg == force ) {
				forceRegistration.erase(forceRegistration.begin()+i);
				break;
			}
		}
	}
	void PhysicsWorld::runPhysics(real duration) {
		for (auto& fg : forceRegistration) {
			fg.fg->updateForce(ecsManager, duration);
		}
		for (auto body : SceneView<RigidBody>(*ecsManager)) {
			RigidBodySystem::integrate(ecsManager,body,duration);
		}
		std::vector<Contact> contacts;
	//	std::cout << "colliderDetect\n";
		contactWorld.colliderDetect(ecsManager, contacts);
		
	//	 std::cout<< "resolveContacts\n";
		resolver.resolveContacts(contacts.data(),contacts.size(),duration); //std::cout << "EndresolveContacts\n";
		
	}
	void ContactWorld::colliderDetect(EcsManager* ecsManager, std::vector<Contact>& allContacts) {
		
		gjkSupportObj.clear();
		contactlist.clear();
		contactlist.reserve(100);

		Eigen::Matrix<real, 3, 3> rotation;
		Eigen::Matrix<real, 3, 1> translate;
		for (auto entity : SceneView<SphereCollider,BoxCollider,MeshCollider,PlaneCollider>(*ecsManager,true)) {
			GJKSupportCollider collider;
	
			collider.entity = entity;
			auto trans = ecsManager->GetComponent<Transform>(entity);
			GLM2Eigen<3, 3>(trans->getOrientationMat3(), rotation);
			GLM2Eigen<3>(trans->getPosition(), translate);
			collider.tf.setIdentity();
			collider.tf.linear() = rotation;
			collider.tf.translation() = translate;
			if (ecsManager->hasComponent<SphereCollider>(entity)) {
			//	ecsManager->GetComponent<SphereCollider>(entity)->fclData->computeLocalAABB();
				collider.geometry = ecsManager->GetComponent<SphereCollider>(entity)->fclData.get();
			}
			else if (ecsManager->hasComponent<BoxCollider>(entity)) {
			//	ecsManager->GetComponent<BoxCollider>(entity)->fclData->computeLocalAABB();
				collider.geometry = ecsManager->GetComponent<BoxCollider>(entity)->fclData.get();
			}
			else if (ecsManager->hasComponent<MeshCollider>(entity)) {
			//	ecsManager->GetComponent<MeshCollider>(entity)->fclData->computeLocalAABB();
				collider.geometry = ecsManager->GetComponent<MeshCollider>(entity)->fclData.get();
			}
			else if (ecsManager->hasComponent<PlaneCollider>(entity)) {
				collider.geometry = ecsManager->GetComponent<PlaneCollider>(entity)->fclData.get();
			}
			gjkSupportObj.emplace_back(collider);
		}
		//std::vector<Contact> allContacts;
		for (int e1 = 0; e1 < gjkSupportObj.size();++e1) {
			const auto& obj1 = gjkSupportObj[e1];
			for (int e2 = e1 + 1; e2 < gjkSupportObj.size();++e2) {
				const auto& obj2 = gjkSupportObj[e2];
				
				

				fcl::CollisionRequest<real> request(1000, true);
				fcl::CollisionResult<real> result;
				int num_contacts = fcl::collide(obj1.geometry, obj1.tf, obj2.geometry, obj2.tf, request, result);
				std::vector<fcl::Contact<real>> contacts;
				result.getContacts(contacts);
			//	std::cout << contacts.size() << std::endl;
				for(const auto& c:contacts) {
				
					Contact contact;
					Eigen2GLM<3>(c.normal,contact.contactNormal);
					Eigen2GLM<3>(c.pos,contact.contactPoint);
					contact.penetration = c.penetration_depth;
					
					auto rigidBody1 = ecsManager->GetComponent<RigidBody>(obj1.entity);
					auto rigidBody2 = ecsManager->GetComponent<RigidBody>(obj2.entity);
					if (!rigidBody1->hasFiniteMass() && !rigidBody2->hasFiniteMass())continue;
					if(!rigidBody1->hasFiniteMass()){
						
						contact.SetData(ecsManager->GetComponent<RigidBody>(obj2.entity), ecsManager->GetComponent<Transform>(obj2.entity), nullptr, nullptr);
					}
					else if (!rigidBody2->hasFiniteMass()) {
						contact.contactNormal = -contact.contactNormal;
						contact.SetData(ecsManager->GetComponent<RigidBody>(obj1.entity), ecsManager->GetComponent<Transform>(obj1.entity), nullptr, nullptr);
					}
					else {
						contact.contactNormal = -contact.contactNormal;
						contact.SetData(ecsManager->GetComponent<RigidBody>(obj1.entity), ecsManager->GetComponent<Transform>(obj1.entity), ecsManager->GetComponent<RigidBody>(obj2.entity), ecsManager->GetComponent<Transform>(obj2.entity));

					}
					auto contactNormal = contact.contactNormal;
					
					allContacts.push_back(contact);
				//	LOG(LogLevel::Debug, "ContactNormal: ",contactNormal.x, contactNormal.y, contactNormal.z);
				//	LOG(LogLevel::Debug, "ContactPosition: ",contact.contactPoint.x, contact.contactPoint.y, contact.contactPoint.z);
				//	LOG(LogLevel::Debug, "PenetrationDepth: ",contact.penetration);
				}
				
			}
		}
	}
}