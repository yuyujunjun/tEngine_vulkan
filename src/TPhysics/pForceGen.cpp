#include"pForceGen.h"
#include"tParticles.h"
#include"RigidBody.h"
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
	void Gravity::updateForce(RigidBody* body, real duration) {
		if (!body->hasFiniteMass())return;
		body->addForce(gravity * body->getMass());
	}
	void Spring::updateForce(RigidBody* body, real duration) {
		Vector3 pos0 = body->getTransform().getMtx() * Vector4(anchor,1);
		Vector3 pos1 = connectedBody ? connectedBody->getTransform().getMtx() * Vector4(connectedAnchor, 1) : connectedAnchor;
		Vector3 force = pos0 - pos1;
		real magnitude = glm::length(force);
		magnitude =abs( magnitude - restLength);
		magnitude *= springConstant;
		force = glm::normalize(force);
		force *= -magnitude;
		body->addForceAtPoint(force, pos0);
		if (autoConfigureConnectedAnchor && connectedBody) {
			connectedBody->addForceAtPoint(-force, pos1);
		}
	}
	void Aero::updateForce(RigidBody* body, real duration) {
		Aero::updateForceFromTensor(body, duration, tensor);
	}
	
	void Aero::updateForceFromTensor(RigidBody* body, real duration, const Mat3& tensor) {
		// Calculate total velocity (windspeed and body's velocity).
		Vector3 velocity = body->getVelocity();
		velocity += windSpeed;

		// Calculate the velocity in body coordinates
		Vector3 bodyVel = body->getTransform().transformInverseDirection(velocity);// *Vector4(velocity, 0);

		// Calculate the force in body coordinates
		Vector3 bodyForce = tensor*(bodyVel);
		// Calculate the force in world coordinates
		Vector3 force = body->getTransform().transformDirection(bodyForce);

		// Apply the force
		body->addForceAtLocalPoint(force, position);
	}
	void AeroControl::updateForce(RigidBody* body, real duration) {
		Mat3 tensor = getTensor();
		Aero::updateForceFromTensor(body, duration, tensor);
	}
	Mat3 AeroControl::getTensor() {
		if (controlSetting <= -1.0f) return minTensor;
		else if (controlSetting >= 1.0f) return maxTensor;
		else if (controlSetting < 0)
		{
			real prop = controlSetting + 1;
			return minTensor * (1 - prop) + tensor * prop;// , controlSetting + 1.0f);
		}
		else if (controlSetting > 0)
		{
			return tensor*(1-controlSetting)+ maxTensor* controlSetting;
		}
		else return tensor;
	}
}