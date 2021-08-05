#pragma once
#include"numerical.h"
#include"tTransform.h"
#include"Component.h"
namespace tEngine {
	class RigidBody{
		
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
		Vector3 accumInducedByForce;
	public:
		RigidBody() :inverseInertiaTensor(1), inverseInertiaTensorWorld(1),inverseMass(1), velocity(0, 0, 0), angularVelocity(0, 0, 0),
			linearDamping(0.9), angularDamping(0.9), forceAccum(0, 0, 0), torqueAccum(0, 0, 0), accleration(0, 0, 0), isAwake(true), canSleep(true), motion(sleepEpsilon*2.0f), accumInducedByForce(0,0,0){}
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
		void addVelocity(const Vector3& v);
		void addAngularVelocity(const Vector3& v);
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
		//get Constant Acceleration(not induced by force)
		const Vector3& getAcceleration()const;
		//get accuration this frame
		const Vector3& getCurrentAcceleration()const;
		
		const Vector3& getVelocity()const;
		const Vector3& getAngularVelocity()const;
		const Vector3& getPosition()const;
		void setInertiaTensor(const Mat3& inertiaTensor);
		void calculateDerivedData();
		void transformInertiaTensor();
		Mat3 getInverseInertiaTensorWorld()const;

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
	inline Mat3 CuboidInertiaTensor(const real mass, const real dx, const real dy, const real dz) {
		Mat3 re;
		real coff = 0.08333 * mass;
		setInertiaTensorCoeffs(re, coff*(dy*dy+dz*dz), coff*(dx*dx+dz*dz), coff * (dx * dx + dy * dy));
		return re;
	}
	inline Mat3 SphereInertiaTensor(const real mass, const real r) {
		Mat3 re;
		real coff = 0.4 * mass * r * r;
		setInertiaTensorCoeffs(re, coff, coff, coff);
		return re;
	}
	inline Mat3 SphereShellInertialTensor(const real mass, const real r) {
		Mat3 re;
		real coff = 0.6667 * mass * r * r;
		setInertiaTensorCoeffs(re, coff, coff, coff);
		return re;
	}
}