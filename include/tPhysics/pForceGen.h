#pragma once
#include<vector>
#include"tPhysics/physicsCore.h"
//#include"tParticles.h"
namespace tPhysics {
	class Particle;
	class ParticleForceGenerator {
	public:
		virtual void updateForce(Particle* particle, real duration) = 0;
	};
	/// <summary>
	/// store all the relations between a particle and a force
	/// </summary>
	class ParticleForceRegistry {
	protected:
		struct ParticleForceRegistration {
			ParticleForceRegistration() :particle(0), fg(0) {}
			ParticleForceRegistration(Particle* p, ParticleForceGenerator* f) :particle(p), fg(f) {}
			Particle* particle;
			ParticleForceGenerator* fg;
		};
		std::vector<ParticleForceRegistration> registry;
	public:
		//don't add the same fg to a same particle
		void add(Particle* particle, ParticleForceGenerator* fg);
		void remove(Particle* particle, ParticleForceGenerator* fg);
		void clear();
		void updateForces(real duration);
	};
	class ParticleGravity :public ParticleForceGenerator {
		Vector3 gravity;
	public:
		ParticleGravity(const Vector3& gravity) :gravity(gravity) {};
		virtual void updateForce(Particle* particle, real duration);
	};
	class ParticleDrag :public ParticleForceGenerator {
		real k1;
		real k2;
	public:
		ParticleDrag(real k1, real k2) :k1(k1), k2(k2) {};
		virtual void updateForce(Particle* particle, real duration);

	};
	class ParticleSpring :public ParticleForceGenerator {
		Particle* other;
		real springConstant;
		real restLength;
	public:
		ParticleSpring(Particle* other, real springConstant, real restLength) :other(other), springConstant(springConstant), restLength(restLength) {}
		virtual void updateForce(Particle* particle, real duration);
	};
	class ParticleAnchoredSpring :public ParticleForceGenerator {
		Vector3 anchor;
		real springConstant;
		real restLength;
	public:
		void setAnchor(const Vector3& anchor);
		void setSpringConstant(const real constant) { springConstant = constant; }
		void setRestLength(const real length) { restLength = length; }
		ParticleAnchoredSpring() = default;
		ParticleAnchoredSpring(const Vector3& anchor, real springConstant, real restLength) :anchor(anchor), springConstant(springConstant), restLength(restLength) {}
		virtual void updateForce(Particle* particle, real duration);
	};
	class ParticleBungee :public ParticleForceGenerator {
		Particle* other;
		real springConstant;
		real restLength;
		ParticleBungee(Particle* other, real springConstant, real restLength) :other(other), springConstant(springConstant), restLength(restLength) {}
		virtual void updateForce(Particle* particle, real duration);
	};
	class ParticleAnchoredBungee :public ParticleForceGenerator {
		Vector3 anchor;
		real springConstant;
		real restLength;
	public:
		void setAnchor(const Vector3& anchor);
		void setSpringConstant(const real constant) { springConstant = constant; }
		void setRestLength(const real length) { restLength = length; }
		ParticleAnchoredBungee() {};
		ParticleAnchoredBungee(const Vector3& anchor, real springConstant, real restLength) :anchor(anchor), springConstant(springConstant), restLength(restLength) {}
		virtual void updateForce(Particle* particle, real duration);
	};
}