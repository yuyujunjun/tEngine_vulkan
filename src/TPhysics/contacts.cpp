#include"contacts.h"
#include"collide.h"
#include"GameObject.h"
#include"RigidBody.h"
namespace tEngine {
	void Contact::SetData(RigidBody* obj1,Transform* t1, RigidBody* obj2,Transform* t2, const ContactInfo& info) {
		//collider[0] = obj1;
		rigidBody[0] = obj1;
		transform[0] = t1;
		if (obj2) {
			rigidBody[1] = obj2;
			transform[1] = t2;
		//	collider[1] = obj2;
		}
		contactPoint = info.pos;
		contactNormal = info.dir;
		penetration = info.depth;
	}
	Vector3 Contact::calculateLocalVelocity(unsigned bodyIdx, real duration) {
	//	Collider* obj = collider[bodyIdx];
		RigidBody* body = rigidBody[bodyIdx];
		Mat3 world2Contact = glm::transpose(contact2World);
		
		Vector3 velocity = glm::cross(body->getAngularVelocity(), relativeContactPosition[bodyIdx]) + body->getVelocity();
		Vector3 contactVelocity = world2Contact * velocity;
		//add extra force affect on the planar(notice that we have add veocity by rigidBody->integrate())
		//we need remove this velocity to avoid obj from sliding when force direction is not the same as the contact normal
		Vector3 accVelocity = body->getCurrentAcceleration() * duration;
		accVelocity = world2Contact * accVelocity;
		accVelocity.y = 0;
		contactVelocity += accVelocity;
		return contactVelocity;

	}
	
	void Contact::calculateDesiredDeltaVelocity(real duration) {
		const static real velocityLimit = (real)0.25f;
		//we need remove velocity caused by force this frame to avoid instability
		//it's better for considering torque but most reaction force is caused by gravity, so it's ok now
		real velocityFromAcc = 0;
		if (rigidBody[0]->getAwake()) {
			velocityFromAcc += glm::dot(rigidBody[0]->getCurrentAcceleration() * duration,  contactNormal);
		}
		if (rigidBody[1] && rigidBody[1]->getAwake()) {
			velocityFromAcc -= glm::dot(rigidBody[1]->getCurrentAcceleration() * duration, contactNormal);
		}
		// if velocity is slow, limit the restitution
		real thisRestitution = restitution;
		if (abs(contactVelocity.y) < velocityLimit) {
			thisRestitution = 0;
		}
		desiredDeltaVelocity = -contactVelocity.y - thisRestitution * (contactVelocity.y-velocityFromAcc);
	}
	void Contact::calculateAngularInertiaPerUnit() {
		for (int i = 0; i < 2; ++i) {
			if (rigidBody[i]) {
				Vector3 impulsetorquePerUnitImpulse = glm::cross(relativeContactPosition[i], contactNormal);
				Vector3 rotationPerUnitImpulse = rigidBody[i]->getInverseInertiaTensorWorld() * impulsetorquePerUnitImpulse;
				Vector3 velocityPerUnitImpulse = glm::cross(rotationPerUnitImpulse, relativeContactPosition[i]);
				velocityPerUnitImpulse[i] = glm::dot(velocityPerUnitImpulse, contactNormal);
			}
		}
	}
	void Contact:: calculateInternals(real duration) {
		contact2World = calculateContactsBasis(contactNormal);
		relativeContactPosition[0] = contactPoint - transform[0]->getPosition();
		if (rigidBody[1]) {
			relativeContactPosition[1] = contactPoint - transform[1]->getPosition();
		}
		contactVelocity = calculateLocalVelocity(0, duration);
		if (rigidBody[1]) {
			contactVelocity -= calculateLocalVelocity(1, duration);//point to the 0th object
		}
		
		calculateDesiredDeltaVelocity(duration);
		calculateAngularInertiaPerUnit();
		

	}
	Vector3 Contact::calculateFrictionlessImpulse() {
	//calculateAngularInertiaPerUnit();
		Vector3 impulseContact;
		real deltaVelocity = velocityPerUnitImpulse[0];
		deltaVelocity += rigidBody[1]->getInverseMass();
		if (rigidBody[1]) {
			deltaVelocity += velocityPerUnitImpulse[1];
			deltaVelocity += rigidBody[1]->getInverseMass();
		}
		impulseContact.y = desiredDeltaVelocity / deltaVelocity;
		impulseContact.x = impulseContact.z = 0;
		return impulseContact;
	}

	Vector3 Contact::calculateFrictionImpulse() {
		Mat3 deltaVelWorld = Mat3(0);
		real inverseMass = 0;
		///
		///for (unsigned i = 0; i < 2; ++i)if (rigidBody[i]) {
		///	Mat3 R = createSkewMat3(relativeContactPosition[i]);
		///	//similar to r.cross(contact Normal), but happen for three axis, store in column
		///	Mat3 impulseTorquePerUnitImpulse = R * contact2World;
		///	Mat3 rotationPerUnitImpulse = rigidBody[i]->getInverseInertiaTensorWorld() * impulseTorquePerUnitImpulse;
		///	//Get velocity: w.cross(r)
		///	Mat3 velocityPerUnitImpulse = -R * rotationPerUnitImpulse;
		///	//change to contact frame
		/////	Mat3 velocityPerUnitImpulseContact = velocityPerUnitImpulse * contact2World;//=world2contact*velocityPerUnitImpulse
		///	deltaVelWorld += velocityPerUnitImpulse;
		///	inverseMass += rigidBody[i]->getInverseMass();
		///}
		///Mat3 deltaVelContact = glm::transpose(contact2World) * deltaVelWorld;
		///We extract some operations above and rearange
		for (unsigned i = 0; i < 2; ++i)if (rigidBody[i]) {
			Mat3 R = createSkewMat3(relativeContactPosition[i]);
			deltaVelWorld += -R * rigidBody[i]->getInverseInertiaTensorWorld()*R;
			inverseMass += rigidBody[i]->getInverseMass();
		}
		Mat3 impulse2VelocityContact = glm::transpose(contact2World) * deltaVelWorld * contact2World;
		impulse2VelocityContact[0][0] += inverseMass;
		impulse2VelocityContact[1][1] += inverseMass;
		impulse2VelocityContact[2][2] += inverseMass;
		// Invert to get the impulse needed per unit velocity.
		Mat3 impulseMatrix = glm::inverse(impulse2VelocityContact);
		//Find the target velocities to remove
		Vector3 vel(-contactVelocity.x, desiredDeltaVelocity, -contactVelocity.z);
		Vector3 impulseContact = impulseMatrix * vel;
		//if use dynamic friction, don't kill all velocity, use firction kill velocity
		real planarImpulse = sqrt(impulseContact.x * impulseContact.x + impulseContact.z * impulseContact.z);
		if (planarImpulse > impulseContact.y * friction) {
			impulseContact.x /= planarImpulse;
			impulseContact.z /= planarImpulse;
			//velocity induced by impulse
			impulseContact.y = impulse2VelocityContact[0][1] * friction * impulseContact.x + impulse2VelocityContact[1][1] +
				impulse2VelocityContact[2][1] * friction * impulseContact.z;
			impulseContact.y = desiredDeltaVelocity / impulseContact.y;
			impulseContact.x *= friction * impulseContact.y;
			impulseContact.z *= friction * impulseContact.y;
		}
		return impulseContact;

	}
	void Contact::ApplyVelocityChange(Vector3 linearChange[2], Vector3 angularChange[2]) {
		Vector3 impulseContact;
		if (isZero(friction)) {
			impulseContact = calculateFrictionlessImpulse();
		}
		else {
			impulseContact = calculateFrictionImpulse();
		}
		Vector3 impulseWorld = contact2World * impulseContact;
		//split impulse into linear and rotational components
		Vector3 impulseTorque = glm::cross(relativeContactPosition[0], impulseWorld);
		angularChange[0] = rigidBody[0]->getInverseInertiaTensorWorld() * impulseTorque;
		linearChange[0] = impulseWorld * rigidBody[0]->getInverseMass();
		rigidBody[0]->addVelocity(angularChange[0]);
		rigidBody[0]->addAngularVelocity(linearChange[0]);
		if (rigidBody[1]) {
			//use -impulseWorld
			Vector3 impulseTorque = glm::cross( impulseWorld, relativeContactPosition[1]);
			angularChange[1] = rigidBody[1]->getInverseInertiaTensorWorld() * impulseTorque;
			linearChange[1] = -impulseWorld * rigidBody[1]->getInverseMass();
			rigidBody[1]->addVelocity(linearChange[1]);
			rigidBody[1]->addAngularVelocity(angularChange[1]);
		}
	}
	void Contact::MatchAwakeState() {
		if (!rigidBody[1])return;
		if (rigidBody[0]->getAwake() ^ rigidBody[1]->getAwake()) {
			if (rigidBody[0]->getAwake()) {
				rigidBody[1]->setAwake(true);
			}
			else {
				rigidBody[0]->setAwake(true);
			}
		}
	}
	void Contact::ApplyPositionChange(Vector3 linearChange[2], Vector3 angularChange[2],real penetration) {
		real totalInertia = 0;
		real linearInertia[2];
		const real angularLimit = (real)0.2f;
		//calculateAngularInertiaPerUnit();
		//real angularInertia[2];
		for (unsigned i = 0; i < 2; ++i) {
			if (rigidBody[i]) {
				linearInertia[i]= rigidBody[i]->getInverseMass();
				totalInertia += velocityPerUnitImpulse[i];
				totalInertia += linearInertia[i];
			}
		}
		real angularMove[2];
		real linearMove[2];
		
		real inverseTotalInertia = 1.0 / totalInertia;
		for (unsigned i = 0; i < 2; ++i) {
			if (rigidBody[i]) {
				real sign = (i == 0) ? 1 : -1;
				
				angularMove[i] = sign * inverseTotalInertia * penetration * velocityPerUnitImpulse[i];
				linearMove[i] = sign * inverseTotalInertia * penetration *linearInertia[i];
				
				//Avoid angular Move if it's too large
				//calculate sin()
				Vector3 projection = relativeContactPosition[i] - contactNormal * glm::dot(contactNormal,relativeContactPosition[i]);
				real maxMagnitude = angularLimit * glm::length(projection);
				if (angularMove[i] < -maxMagnitude) {
					real totalMove = angularMove[i] + linearMove[i];
					angularMove[i] -= maxMagnitude;
					linearMove[i] = totalMove - angularMove[i];
				}
				else if (angularMove[i] > maxMagnitude) {
					real totalMove = angularMove[i] + linearMove[i];
					angularMove[i] = maxMagnitude;
					linearMove[i] = totalMove - angularMove[i];
				}
				if (angularMove[i] != 0) {
			
					Vector3 impulseDirection = glm::cross(relativeContactPosition[i], contactNormal);
					//angularMove[i]/angularInertia: the impulse required to move angularMove[i] along direction
					angularChange[i] =rigidBody[i]->getInverseInertiaTensorWorld()* impulseDirection * angularMove[i] / velocityPerUnitImpulse[i];
					glm::quat q = transform[i]->getOrientation();
					transform[i]->rotate(angularChange[i]);
				}
				linearChange[i] = contactNormal * linearMove[i];
				transform[i]->translate(linearChange[i]);
				//for sleep objects, we have to update mannully
				if (!rigidBody[i]->getAwake())rigidBody[i]->calculateDerivedData(transform[i]);
				
			}
		}
		
	}
	ContactResolver::ContactResolver(unsigned iterations, real velocityEpsilon , real positionEpsilon ) :velocityIterations(iterations),positionIterations(iterations),velocityEpsilon(velocityEpsilon),positionEpsilon(positionEpsilon){
		
	
	}
	ContactResolver::ContactResolver(unsigned velocityIterations, unsigned positionIterations, real velocityEpsilon, real positionEpsilon ) : velocityIterations(velocityIterations), positionIterations(positionIterations), velocityEpsilon(velocityEpsilon), positionEpsilon(positionEpsilon) {

	}
	void ContactResolver::resolveContacts(Contact* contactArray, unsigned numContacts, real duration){
		if (numContacts == 0)return;
		prepareContacts(contactArray, numContacts, duration);
		adjustPositions(contactArray, numContacts, duration);
		adjustVelocities(contactArray, numContacts, duration);

	}
	void ContactResolver::prepareContacts(Contact* contactArray, unsigned numContacts, real duration) {
		for(unsigned i=0;i<numContacts;++i){
			contactArray[i].calculateInternals(duration);
		}
	}
	void ContactResolver::adjustPositions(Contact* c, unsigned numContacts, real duration) {
		positionIterationsUsed = 0;
		Vector3 linearChange[2], angularChange[2];
		while(positionIterationsUsed<positionIterations) {
			//Contact* worstContact = nullptr;
			real worstPenetration = positionEpsilon;
			unsigned index=numContacts;
			for (unsigned cid = 0; cid < numContacts; ++cid) {
				if (c[cid].penetration > worstPenetration) {
					index = cid;
					
					worstPenetration = c[cid].penetration;
				}
			}
			if (index==numContacts)break;
		
			c[index].MatchAwakeState();
			
			c[index].ApplyPositionChange(linearChange,angularChange,c[index].penetration);
			Vector3 deltaPosition;
			for (unsigned cid = 0; cid < numContacts; ++cid) {
				for (unsigned bodyId = 0; bodyId < 2; ++bodyId)if (c[cid].rigidBody[bodyId]) { 
					for (unsigned d = 0; d < 2; ++d)if(c[index].rigidBody[d]) {
						if (c[cid].rigidBody[bodyId]==c[index].rigidBody[d]) {
							//although contact position change slightly, we only concern about the contact normal direction, so it's ok
							deltaPosition = linearChange[d] + glm::cross(angularChange[d], c[cid].relativeContactPosition[bodyId]);
							c[cid].penetration += glm::dot(deltaPosition, c[cid].contactNormal)*(bodyId?1:-1);

						}
					}
					
				}
			}
			positionIterationsUsed++;
			
		}
	}
	void ContactResolver::adjustVelocities(Contact* c, unsigned numContacts, real duration) {
		velocityIterationsUsed = 0;
		Vector3 linearChange[2], angularChange[2];
		while (velocityIterationsUsed < velocityIterations) {
			real max = velocityEpsilon;
			unsigned index = numContacts;
			for (unsigned i = 0; i < numContacts; ++i) {
				if (c[i].desiredDeltaVelocity > max) {
					max = c[i].desiredDeltaVelocity;
					index = i;
				}
			}
			if (index == numContacts)break;
			c[index].MatchAwakeState();
			c[index].ApplyVelocityChange(linearChange,angularChange);
			for (unsigned i = 0; i < numContacts; ++i) {
				for (unsigned b = 0; b < 2; ++b)if (c[i].rigidBody[b]) {
					for (unsigned d = 0; d < 2; ++d)if (c[index].rigidBody[d]) {
						if (c[index].rigidBody[d] == c[i].rigidBody[b]) {
							//update contact velocity
							Vector3 deltaVe=linearChange[d] + glm::cross(angularChange[d], c[i].relativeContactPosition[b]);
							c[i].contactVelocity += glm::transpose(c[i].contact2World) * deltaVe * (real)(b ? 1 : -1);
							c[i].calculateDesiredDeltaVelocity(duration);
						}
					}
				}
			}
			velocityIterationsUsed++;
		}
	}
}