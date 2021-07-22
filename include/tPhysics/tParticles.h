#pragma once
#include"physicsCore.h"
namespace tPhysics {
	class Particle {
	public:
		Particle() :position(0, 0, 0), velocity(0, 0, 0), accleration(0, 0, 0), forceAccum(0, 0, 0), damping(0.99), inverseMass(1) {};
		void setMass(const real mass);
		void setInverseMass(const real inverse_mass);
		void integrate(const real duration);
		void clearAccumulator();
		void addForce(const Vector3& force);
		void setVelocity(const Vector3& v);
		void setPosition(const Vector3& v);
		void setPosition(real x, real y, real z);
		void setDamping(real damping);
		void setAcceleration(const Vector3& accleration);
		bool hasFiniteMass();
		real getMass()const;
		real getInverseMass()const;
		const Vector3& getAcceleration()const;
		const Vector3& getVelocity()const;
		const Vector3& getPosition()const;
		
		
	protected:
		Vector3 position;
		Vector3 velocity;
		Vector3 accleration;
		Vector3 forceAccum;
		real damping;
		real inverseMass;
	};
}