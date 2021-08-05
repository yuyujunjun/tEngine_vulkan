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
		
		calculateDerivedData(transform,body);
		clearAccumulator(body);
		if (body->canSleep) {
			real currentMotion = glm::dot(body->velocity, body->velocity) + glm::dot(body->angularVelocity, body->angularVelocity);
			real bias = std::pow(0.5, duration);
			body->motion = bias * body->motion + (1 - bias) * currentMotion;
			if (body->motion < sleepEpsilon)body->setAwake(false);
			else if (body->motion > 10 * sleepEpsilon)body->motion = 10 * sleepEpsilon;
		}

	}
	void RigidBodySystem::calculateDerivedData(Transform* transform, RigidBody* body) {

		transform->updateMtx();
		transformInertiaTensor(transform,body);
	}
	void RigidBody::setInertiaTensor(const Mat3& inertiaTensor) {
		inverseInertiaTensor = glm::inverse(inertiaTensor);
	}
	//BMB^{-1}, so inverseworld=BM^{-1}B^{-1}
	void RigidBodySystem::transformInertiaTensor(Transform* transform, RigidBody* body) {
		body->inverseInertiaTensorWorld =  transform->getMat3()* body->inverseInertiaTensor *  transform->getInverseMat3();
	}
	Mat3 RigidBody::getInverseInertiaTensorWorld()const {
		return inverseInertiaTensorWorld;
	}
	void RigidBodySystem::clearAccumulator(RigidBody* body) {
		auto& forceAccum = body->forceAccum;
		auto& torqueAccum = body->torqueAccum;
		forceAccum.x = forceAccum.y = forceAccum.z = 0;
		torqueAccum.x = torqueAccum.y = torqueAccum.z = 0;
	}
	void RigidBodySystem::addForce(RigidBody* body, const Vector3& force) {
		body->forceAccum += force;
		body->isAwake = true;
	}
	void RigidBodySystem::addForceAtLocalPoint(RigidBody* body,Transform* transform, const Vector3& force, const Vector3& point) {
		Vector4 point_in_world =  transform->getMtx() * Vector4(point,1);
		addForceAtPoint(body,transform,force, point_in_world);
		body->isAwake = true;
	}
	void RigidBodySystem::addForceAtPoint(RigidBody* body, Transform* transform, const Vector3& force, const Vector3& point) {
		addForce(body,force);
		addTorque( body,glm::cross(point -  transform->getPosition(),force));
		body->isAwake = true;
	}
	void RigidBodySystem::addTorque(RigidBody* body, const Vector3& torque) {
		body->torqueAccum += torque;
		body->isAwake = true;
	}
	void RigidBodySystem::addVelocity(RigidBody* body, const Vector3& v) {
		body->velocity += v;
	}
	void RigidBodySystem::addAngularVelocity(RigidBody* body, const Vector3& v) {
		body->angularVelocity += v;
	}
	void RigidBodySystem::setVelocity(RigidBody* body, const Vector3& v) {
		body->velocity = v;
	}
	void RigidBodySystem::setAngularVelocity(RigidBody* body, const Vector3& v) {
		body->angularVelocity = v;
	}
	void RigidBodySystem::setLinearDamping(RigidBody* body, real damping) {
		body->linearDamping = damping;
	}void RigidBodySystem::setAngularDamping(RigidBody* body, real damping) {
		body->angularDamping = damping;
	}
	void RigidBodySystem::setAcceleration(RigidBody* body, const Vector3& accleration) {
		body->accleration = accleration;
	}
	bool RigidBodySystem::hasFiniteMass(RigidBody* body) {
		return body->inverseMass > 0;
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