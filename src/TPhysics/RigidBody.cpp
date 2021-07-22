#include"RigidBody.h"
#include"physicsCore.h"
#include"tTransform.h"
namespace tPhysics {
	void RigidBody::integrate(const real duration) {
		if (!isAwake)return;
		Vector3 angularAccleration = inverseInertiaTensorWorld * torqueAccum;
		accleration += forceAccum * inverseMass;
		velocity += accleration * duration;
		angularVelocity += angularAccleration * duration;
		velocity *= std::pow(linearDamping,duration);
		angularVelocity *= std::pow(angularDamping,duration);
		transform.setPosition(transform.getPosition()+velocity*duration);
		transform.rotate(angularVelocity, duration);
		
		calculateDerivedData();
		clearAccumulator();
		if (canSleep) {
			real currentMotion = glm::dot(velocity, velocity) + glm::dot(angularVelocity, angularVelocity);
			real bias = std::pow(0.5, duration);
			motion = bias * motion + (1 - bias) * currentMotion;
			if (motion < sleepEpsilon)setAwake(false);
			else if (motion > 10 * sleepEpsilon)motion = 10 * sleepEpsilon;
		}

	}
	void RigidBody::calculateDerivedData() {
		transform.updateMtx();
		transformInertiaTensor();
	}
	void RigidBody::setInertiaTensor(const Mat3& inertiaTensor) {
		inverseInertiaTensor = glm::inverse(inertiaTensor);
	}
	void RigidBody::transformInertiaTensor() {
		auto rotationMat = transform.getOrientationMat();
		inverseInertiaTensorWorld = glm::transpose(rotationMat) * inverseInertiaTensor * rotationMat;
	}
	void RigidBody::clearAccumulator() {
		forceAccum.x = forceAccum.y = forceAccum.z = 0;
		torqueAccum.x = torqueAccum.y = torqueAccum.z = 0;
	}
	void RigidBody::addForce(const Vector3& force) {
		forceAccum += force;
		isAwake = true;
	}
	void RigidBody::addForceAtLocalPoint(const Vector3& force, const Vector3& point) {
		Vector4 point_in_world = transform.getMtx() * Vector4(point,1);
		addForceAtPoint(force, point_in_world);
		isAwake = true;
	}
	void RigidBody::addForceAtPoint(const Vector3& force, const Vector3& point) {
		addForce(force);
		addTorque( glm::cross(point - transform.getPosition(),force));
		isAwake = true;
	}
	void RigidBody::addTorque(const Vector3& torque) {
		torqueAccum += torque;
		isAwake = true;
	}
	void RigidBody::setVelocity(const Vector3& v) {
		this->velocity = v;
	}
	void RigidBody::setLinearDamping(real damping) {
		this->linearDamping = damping;
	}void RigidBody::setAngularDamping(real damping) {
		this->angularDamping = damping;
	}
	void RigidBody::setAcceleration(const Vector3& accleration) {
		this->accleration = accleration;
	}
	bool RigidBody::hasFiniteMass() {
		return inverseMass > 0;
	}
	void RigidBody::setAwake(const bool awake) {
		isAwake = awake;
		if (awake) {
			
		}
		else {
			velocity.x = velocity.y = velocity.z = 0;
			angularVelocity.x = angularVelocity.y = angularVelocity.z=0;
		}
	}
	void RigidBody::setCanSleep(const bool canSleep) {
		this->canSleep = canSleep;
		if(!canSleep && !isAwake)setAwake(true);
	}
	void RigidBody::setMass(real mass) {
		assert(mass != 0);
		mass = mass;
		inverseMass = 1.0 / mass;
	}
	void RigidBody::setInverseMass(real inverse_mass) {
		inverseMass = inverse_mass;
	}
	real RigidBody::getMass() const {
		if (inverseMass == 0) { return std::numeric_limits<real>::max(); }
		return (real)(1.0) / inverseMass;
	}
	const Vector3& RigidBody::getVelocity()const {
		return velocity;
	}
	const Vector3& RigidBody::getPosition()const {
		return transform.getPosition();//position;
	}
	const Vector3& RigidBody::getAcceleration()const {
		return accleration;
	}

	real RigidBody::getInverseMass()const {
		return inverseMass;
	}
}