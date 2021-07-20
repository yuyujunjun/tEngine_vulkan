#include<assert.h>
#include"tPhysics/tParticles.h"

namespace tPhysics {
	void Particle::integrate(real duration) {
		assert(duration > 0);
		position += velocity * duration;

		Vector3 resultingAcc = accleration + forceAccum * inverseMass;

		velocity += resultingAcc * duration;
		velocity *= damping;
		clearAccumulator();
	}
	void Particle::clearAccumulator() {
		forceAccum = Vector3(0);
	}
	void Particle::addForce(const Vector3& force) {
		forceAccum += force;
	}
	void Particle::setVelocity(const Vector3& v) {
		this->velocity = v;
	}
	void Particle::setPosition(const Vector3& v) {
		this->position = v;
	}
	bool Particle::hasFiniteMass() {
		return inverseMass >= 0;
	}
	void Particle::setMass(real mass) {
		assert(mass != 0);
		mass = mass;
		inverseMass = 1.0 / mass;
	}
	void Particle::setInverseMass(real inverse_mass) {
		inverseMass = inverse_mass;
	}
	real Particle::getMass() const{
		if (inverseMass == 0) { return std::numeric_limits<real>::max(); }
		return (real)(1.0) / inverseMass;
	}
	const Vector3& Particle::getVelocity()const {
		return velocity;
	}
	const Vector3& Particle::getPosition()const {
		return position;
	}
	const Vector3& Particle::getAcceleration()const {
		return accleration;
	}

	real Particle::getInverseMass()const {
		return inverseMass;
	}
}