#include"pForceGen.h"
#include"tParticles.h"
#include"RigidBody.h"

namespace tEngine {
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
	void Gravity::updateForce(EntityID id, real duration) {
		auto body = ecsManager->GetComponent<RigidBody>(id);
		if (!body->hasFiniteMass())return;
		body->addForce(gravity * body->getMass());
	}

	void SpringSystem::updateForce(EntityID id, real duration) {
	
		auto spring = ecsManager->GetComponent<Spring>(id);
		Transform* t0 = nullptr;
		Transform* t1 = nullptr;
		Vector3 pos0, pos1;
		if (spring->connectedBody[0] != -1) {
			t0 = ecsManager->GetComponent<Transform>(spring->connectedBody[0]);
			pos0 = t0->getMtx() * Vector4(spring->anchor[0], 1);
		}
		else {
			pos0 = spring->connectedAnchor[0];
		}
		if (spring->connectedBody[1] != -1) {
			t1= ecsManager->GetComponent<Transform>(spring->connectedBody[1]);
			pos1= t1->getMtx() * Vector4(spring->anchor[1], 1);
		}
		else {
			pos1 = spring->connectedAnchor[1];
		}
		

		Vector3 force = pos0 - pos1;
		real magnitude = glm::length(force);
		magnitude =abs( magnitude - spring->restLength);
		magnitude *= spring->springConstant;
		force = glm::normalize(force);
		force *= -magnitude;
		if (t0) {
			ecsManager->GetComponent<RigidBody>(spring->connectedBody[0])->addForceAtPoint(t0,force,pos0);
		}
		if (t1) {
			ecsManager->GetComponent<RigidBody>(spring->connectedBody[1])->addForceAtPoint(t1, -force, pos1);
		}

	}
	void Aero::updateForce(EntityID id, real duration) {
		Aero::updateForceFromTensor(id, duration, tensor);
	}
	
	void Aero::updateForceFromTensor(EntityID id, real duration, const Mat3& tensor) {
		auto body = ecsManager->GetComponent<RigidBody>(id);
		auto transform = ecsManager->GetComponent<Transform>(id);
		// Calculate total velocity (windspeed and body's velocity).
		Vector3 velocity = body->getVelocity();
		velocity += windSpeed;

		// Calculate the velocity in body coordinates
		Vector3 bodyVel = transform->transformInverseDirection(velocity);// *Vector4(velocity, 0);

		// Calculate the force in body coordinates
		Vector3 bodyForce = tensor*(bodyVel);
		// Calculate the force in world coordinates
		Vector3 force = transform->transformDirection(bodyForce);

		// Apply the force
		body->addForceAtLocalPoint(transform,force, position);
	}
	void AeroControl::updateForce(EntityID id, real duration) {
		Mat3 tensor = getTensor();
		
		Aero::updateForceFromTensor(id, duration, tensor);
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