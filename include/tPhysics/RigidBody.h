#pragma once
#include"physicsCore.h"
#include"tTransform.h"
#include"Component.h"
namespace tPhysics {
	class RigidBody:public tEngine::Component {
	
		
		real inverseMass=0;
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
		RigidBody(tEngine::GameObject_* gameObject) :tEngine::Component(gameObject), inverseInertiaTensor(1), inverseInertiaTensorWorld(1),inverseMass(1), velocity(0, 0, 0), angularVelocity(0, 0, 0),
			linearDamping(0.9), angularDamping(0.9), forceAccum(0, 0, 0), torqueAccum(0, 0, 0), accleration(0, 0, 0), isAwake(true), canSleep(true), motion(sleepEpsilon*2.0f) {}
	//	tEngine::Transform transform;
		const tEngine::Transform& getTransform() const; 
		void setMass(const real mass);
		void setInverseMass(const real inverse_mass);
		void integrate(const real duration);
		void clearAccumulator();
		void addForce(const Vector3& force);
		void addForceAtLocalPoint(const Vector3& force, const Vector3& point);
		void addForceAtPoint(const Vector3& force, const Vector3& point);
		void addTorque(const Vector3& torque);
		void setVelocity(const Vector3& v);
		void setAngularVelocity(const Vector3& v);
		
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

	inline void setInertiaTensorCoeffs(Mat3& mat,real ix, real iy, real iz, real ixy = 0, real ixz = 0, real iyz = 0) {
		mat[0] = { ix,-ixy,-ixz };
		mat[1] = {-ixy,iy,-iyz};
		mat[2] = {-ixz,-iyz,iz};
	}
	/**
   * Sets the value of the matrix as an inertia tensor of
   * a rectangular block aligned with the body's coordinate
   * system with the given axis half-sizes and mass.
   */
	inline void setBlockInertiaTensor(Mat3& mat, const Vector3& halfSizes, real mass)
	{
		Vector3 squares = halfSizes * (halfSizes);
		setInertiaTensorCoeffs(mat, 0.3f * mass * (squares.y + squares.z),
			0.3f * mass * (squares.x + squares.z),
			0.3f * mass * (squares.x + squares.y));
	}
}