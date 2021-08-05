#include"pWorld.h"
#include"tParticles.h"
#include"RigidBody.h"
#include"ecs.h"
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
	void PhysicsWorld::registerForce(ForceGenerator* force, EntityID id) {
		forceRegistration.emplace_back(id, force);
	}
	void PhysicsWorld::unregisterForce(ForceGenerator* force, EntityID id) {
		for (int i = forceRegistration.size() - 1; i >= 0; --i) {
			if (forceRegistration[i].fg == force && forceRegistration[i].body == id) {
				forceRegistration.erase(forceRegistration.begin()+i);
				break;
			}
		}
	}
	void PhysicsWorld::runPhysics(real duration) {
		for (auto& fg : forceRegistration) {
			fg.fg->updateForce(fg.body, duration);
		}
		for (auto body : SceneView<RigidBody>(*ecsManager)) {
			rigidBodySystem.integrate(body,duration);
		}
	}
}