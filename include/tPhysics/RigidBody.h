#pragma once
#include"physicsCore.h"
#include"tTransform.h"
namespace tPhysics {
	class RigidBody {
	
		
		real inverseMass;
		Mat3 inverseInertiaTensor;
		Mat3 inverseInertiaTensorWorld;
		Vector3 velocity;
		Vector3 angularVelocity;
		real linearDamping;
		real angularDamping;
		Vector3 forceAccum;
		Vector3 torqueAccum;
		Vector3 accleration;//linear accleration
		bool isAwake;
		bool canSleep;
		real motion;
	public:
		tEngine::Transform transform;
		const tEngine::Transform& getTransform() const{ return transform; }
		void setMass(const real mass);
		void setInverseMass(const real inverse_mass);
		void integrate(const real duration);
		void clearAccumulator();
		void addForce(const Vector3& force);
		void addForceAtLocalPoint(const Vector3& force, const Vector3& point);
		void addForceAtPoint(const Vector3& force, const Vector3& point);
		void addTorque(const Vector3& torque);
		void setVelocity(const Vector3& v);
		void setLinearDamping(real damping);
		void setAngularDamping(real damping);
		void setAcceleration(const Vector3& accleration);
		bool hasFiniteMass();
		bool getAwake()const { return isAwake; }
		void setAwake(const bool awake);
		void setCanSleep(const bool canSleep);
		real getMass()const;
		real getInverseMass()const;
		const Vector3& getAcceleration()const;
		const Vector3& getVelocity()const;
		const Vector3& getPosition()const;
		void setInertiaTensor(const Mat3& inertiaTensor);
		void calculateDerivedData();
		void transformInertiaTensor();


	};
}