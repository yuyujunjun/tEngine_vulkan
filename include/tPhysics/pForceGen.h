#pragma once
#include<vector>
#include"tPhysics/numerical.h"
#include"ecs.h"
//#include"tParticles.h"
namespace tEngine {
	class Particle;
	class RigidBody;
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
	class ForceGenerator:public System {
	public:
		virtual void updateForce(EntityID body, real duration) {};
	};
	struct ForceRegistration {
		ForceRegistration() :body(-1), fg(0) {}
		ForceRegistration(EntityID id, ForceGenerator* f) :body(id), fg(f) {}
		EntityID body;
		ForceGenerator* fg;
	};
	class Gravity :public ForceGenerator {
		Vector3 gravity;
	public:
		Gravity(const Vector3& gravity) :gravity(gravity) {}
		void updateForce(EntityID id, real duration)override;
	};
	struct Spring {
		Spring() {
			connectedBody[0] = -1;
			connectedBody[1] = -1;
		}
		/*if connectedBody is nullptr then the spring will be connected to a fixed point in space*/
		EntityID connectedBody[2];
		/** The point of connection of the spring, in local coordinates. */
		Vector3 anchor[2];
		Vector3 connectedAnchor[2];
		real springConstant;
		real restLength;
		
	};
	class SpringSystem :public ForceGenerator {
	public:
		bool autoConfigureConnectedAnchor=true;
		void updateForce(EntityID id, real duration)override;
	};
	class Aero :public ForceGenerator {
	protected:
		Mat3 tensor;
		Vector3 position;
	public:
		Vector3& windSpeed;
		Aero(const Mat3& tensor, const Vector3& position,  Vector3& windSpeed) :tensor(tensor), position(position), windSpeed(windSpeed) {}
		void updateForce(EntityID id, real duration)override;
		void updateForceFromTensor(EntityID id, real duration, const Mat3& tensor);
	};
	class AeroControl :public Aero {
		Mat3 maxTensor;
		Mat3 minTensor;
		real controlSetting;//current tensor should be interpolated by maxTensor and minTensor
		Mat3 getTensor();
	public:
		AeroControl(const Mat3& base, const Mat3& min, const Mat3& max,  Vector3& position,  Vector3& windSpeed) :Aero(base, position, windSpeed), maxTensor(max), minTensor(min) {};
		void setControl(real value) { controlSetting = value; }
		void updateForce(EntityID id, real duration)override;
	};
}