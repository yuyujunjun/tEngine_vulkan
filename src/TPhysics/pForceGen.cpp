#include"pForceGen.h"
#include"tParticles.h"
namespace tPhysics {
	void ParticleForceRegistry::add(Particle* particle, ParticleForceGenerator* fg) {
		registry.emplace_back(ParticleForceRegistration(particle,fg));
	}
	void ParticleForceRegistry::remove(Particle* particle, ParticleForceGenerator* fg) {
		for (auto iter = registry.begin(); iter != registry.end(); ++iter) {
			if (iter->particle == particle && iter->fg == fg) {
				registry.erase(iter);
				break;
			}
		}
	}
	void ParticleForceRegistry::clear() {
		registry.clear();
	}
	void ParticleForceRegistry::updateForces(real duration) {
		for (auto& re : registry) {
			re.fg->updateForce(re.particle,duration);
		}
	}
	void ParticleGravity::updateForce(Particle* particle, real duration) {
		if (!particle->hasFiniteMass()) { return; }
		particle->addForce(gravity * particle->getMass());
	}
	void ParticleDrag::updateForce(Particle* particle, real duration) {
		Vector3 force= particle->getVelocity();
		real dragCoeff=glm::length(force);
		dragCoeff = k1 * dragCoeff + k2 * dragCoeff * dragCoeff;
		force = glm::normalize(force);
		force *= -dragCoeff;
		particle->addForce(force);
		
	}
	void ParticleSpring::updateForce(Particle* particle, real duration) {
		Vector3 force = particle->getPosition();
		force -= other->getPosition();
		real magnitude = glm::length(force);
		magnitude = abs(magnitude - restLength)*springConstant;
		force = glm::normalize(force);
		force *= -magnitude;
		particle->addForce(force);
	}
	void ParticleAnchoredSpring::setAnchor(const Vector3& anchor) { this->anchor = anchor; }
	void ParticleAnchoredSpring::updateForce(Particle* particle, real duration) {
		Vector3 force = particle->getPosition();
		force -= anchor;
		real magnitude = glm::length(force);
		magnitude = abs(magnitude - restLength) * springConstant;
		force = glm::normalize(force);
		force *= -magnitude;
		particle->addForce(force);
	}
	void ParticleBungee::updateForce(Particle* particle, real duration) {
		Vector3 force = particle->getPosition();
		force -= other->getPosition();
		real magnitude = glm::length(force);
		if (magnitude <= restLength)return;
		magnitude = (magnitude - restLength) * springConstant;
		force = glm::normalize(force);
		force *= -magnitude;
		particle->addForce(force);
	}
	void ParticleAnchoredBungee::setAnchor(const Vector3& anchor) { this->anchor = anchor; }
	void ParticleAnchoredBungee::updateForce(Particle* particle, real duration) {
		Vector3 force = particle->getPosition();
		force -= anchor;
		real magnitude = glm::length(force);
		magnitude = abs(magnitude - restLength) * springConstant;
		force = glm::normalize(force);
		force *= -magnitude;
		particle->addForce(force);
	} 
}