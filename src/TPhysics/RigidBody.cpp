#include"RigidBody.h"
#include"numerical.h"
#include"tTransform.h"
#include"GameObject.h"
namespace tEngine {

	void RigidBody::integrate(const real duration) {
		if (!isAwake)return;
		
		Vector3 angularAccleration = inverseInertiaTensorWorld * torqueAccum;
		accumInducedByForce = accleration;
		accumInducedByForce += forceAccum * inverseMass;
		
		velocity += accumInducedByForce * duration;
		angularVelocity += angularAccleration * duration;
		velocity *= std::pow(linearDamping,duration);
		angularVelocity *= std::pow(angularDamping,duration);
		gameObject->transform.setPosition(gameObject->transform.getPosition()+velocity*duration);
		gameObject->transform.rotate(angularVelocity, duration);
		
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
		gameObject->transform.updateMtx();
		transformInertiaTensor();
	}
	void RigidBody::setInertiaTensor(const Mat3& inertiaTensor) {
		inverseInertiaTensor = glm::inverse(inertiaTensor);
	}
	//BMB^{-1}, so inverseworld=BM^{-1}B^{-1}
	void RigidBody::transformInertiaTensor() {
		inverseInertiaTensorWorld = gameObject->transform.getMat3()* inverseInertiaTensor * gameObject->transform.getInverseMat3();
	}
	Mat3 RigidBody::getInverseInertiaTensorWorld()const {
		return inverseInertiaTensorWorld;
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
		Vector4 point_in_world = gameObject->transform.getMtx() * Vector4(point,1);
		addForceAtPoint(force, point_in_world);
		isAwake = true;
	}
	void RigidBody::addForceAtPoint(const Vector3& force, const Vector3& point) {
		addForce(force);
		addTorque( glm::cross(point - gameObject->transform.getPosition(),force));
		isAwake = true;
	}
	void RigidBody::addTorque(const Vector3& torque) {
		torqueAccum += torque;
		isAwake = true;
	}
	void RigidBody::addVelocity(const Vector3& v) {
		velocity += v;
	}
	void RigidBody::addAngularVelocity(const Vector3& v) {
		angularVelocity += v;
	}
	void RigidBody::setVelocity(const Vector3& v) {
		this->velocity = v;
	}
	void RigidBody::setAngularVelocity(const Vector3& v) {
		this->angularVelocity = v;
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
			motion = sleepEpsilon * 2.0f;
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
	const tEngine::Transform& RigidBody::getTransform() const {
		return  gameObject->transform;
		; }
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
	const Vector3& RigidBody::getAngularVelocity()const {
		return angularVelocity;
	}
	const Vector3& RigidBody::getPosition()const {
		return gameObject->transform.getPosition();//position;
	}
	const Vector3& RigidBody::getAcceleration()const {
		return accleration;
	}
	const Vector3& RigidBody::getCurrentAcceleration()const {
		return accumInducedByForce;
	}
	real RigidBody::getInverseMass()const {
		return inverseMass;
	}
}