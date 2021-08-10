#include"RigidBody.h"
#include"numerical.h"
#include"tTransform.h"
#include"GameObject.h"
namespace tEngine {

	void RigidBodySystem::integrate(EntityID id,const real duration) {
		auto body = ecsManager->GetComponent<RigidBody>(id);
		auto transform = ecsManager->GetComponent<Transform>(id);
		if (!body->isAwake)return;
		
		Vector3 angularAccleration = body->inverseInertiaTensorWorld * body->torqueAccum;
		body->accumInducedByForce = body->accleration;
		body->accumInducedByForce += body->forceAccum * body->inverseMass;
		
		body->velocity += body->accumInducedByForce * duration;
		body->angularVelocity += angularAccleration * duration;
		body->velocity *= std::pow(body->linearDamping,duration);
		body->angularVelocity *= std::pow(body->angularDamping,duration);
		transform->setPosition( transform->getPosition()+ body->velocity*duration);
		transform->rotate(body->angularVelocity, duration);
		
		body->calculateDerivedData(transform);
		body->clearAccumulator();
		if (body->canSleep) {
			real currentMotion = glm::dot(body->velocity, body->velocity) + glm::dot(body->angularVelocity, body->angularVelocity);
			real bias = std::pow(0.5, duration);
			body->motion = bias * body->motion + (1 - bias) * currentMotion;
			if (body->motion < sleepEpsilon)body->setAwake(false);
			else if (body->motion > 10 * sleepEpsilon)body->motion = 10 * sleepEpsilon;
		}

	}
	void RigidBody::calculateDerivedData(Transform* transform) {

	
		transformInertiaTensor(transform);
	}
	void RigidBody::setInertiaTensor(const Mat3& inertiaTensor) {
		inverseInertiaTensor = glm::inverse(inertiaTensor);
	}
	//BMB^{-1}, so inverseworld=BM^{-1}B^{-1}
	void RigidBody::transformInertiaTensor(Transform* transform) {
		this->inverseInertiaTensorWorld =  transform->getMat3()* this->inverseInertiaTensor *  transform->getInverseMat3();
	}
	Mat3 RigidBody::getInverseInertiaTensorWorld()const {
		return inverseInertiaTensorWorld;
	}
	void RigidBody::clearAccumulator() {
		
		auto& forceAccum = this->forceAccum;
		auto& torqueAccum = this->torqueAccum;
		forceAccum.x = forceAccum.y = forceAccum.z = 0;
		torqueAccum.x = torqueAccum.y = torqueAccum.z = 0;
	}
	void RigidBody::addForce( const Vector3& force) {

		this->forceAccum += force;
		this->isAwake = true;
	}
	void RigidBody::addForceAtLocalPoint(Transform* transform, const Vector3& force, const Vector3& point) {
		Vector4 point_in_world =  transform->getMtx() * Vector4(point,1);
		addForceAtPoint(transform,force, point_in_world);
		this->isAwake = true;
	}
	void RigidBody::addForceAtPoint( Transform* transform, const Vector3& force, const Vector3& point) {
		addForce(force);
		addTorque( glm::cross(point -  transform->getPosition(),force));
		this->isAwake = true;
	}
	void RigidBody::addTorque( const Vector3& torque) {
		this->torqueAccum += torque;
		this->isAwake = true;
	}
	void RigidBody::addVelocity( const Vector3& v) {
		this->velocity += v;
	}
	void RigidBody::addAngularVelocity( const Vector3& v) {
		this->angularVelocity += v;
	}
	void RigidBody::setVelocity( const Vector3& v) {
		this->velocity = v;
	}
	void RigidBody::setAngularVelocity(const Vector3& v) {
		this->angularVelocity = v;
	}
	void RigidBody::setLinearDamping( real damping) {
		this->linearDamping = damping;
	}void RigidBody::setAngularDamping( real damping) {
		this->angularDamping = damping;
	}
	void RigidBody::setAcceleration( const Vector3& accleration) {
		this->accleration = accleration;
	}
	bool RigidBody::hasFiniteMass() {
		return this->inverseMass > 0;
	}
	void RigidBody::setAwake( const bool awake) {
		auto body = this;
		body->isAwake = awake;
		if (awake) {
			body->motion = sleepEpsilon * 2.0f;
		}
		else {
			body->velocity.x = body->velocity.y = body->velocity.z = 0;
			body->angularVelocity.x = body->angularVelocity.y = body->angularVelocity.z=0;
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
	const Vector3& RigidBody::getAngularVelocity()const {
		return angularVelocity;
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