#include"tParticles.h"
#include"pContacts.h"
namespace tEngine {
	void ParticleContact::resolve(real duration) {
		resolveVelocity(duration);
		resolveInterpenetration(duration);
	}
	real ParticleContact::calculateSepartingVelocity()const {
		Vector3 relativeVelocity = particle[0]->getVelocity();
		if (particle[1])relativeVelocity -= particle[1]->getVelocity();
		return glm::dot(relativeVelocity, contactNormal);
	}
	void ParticleContact::resolveVelocity(real duration) {
		real separatingVelocity = calculateSepartingVelocity();
		if (separatingVelocity > 0) {
			return;
		}
		real newSepVelocity = -separatingVelocity * restitution;
		//ignore the velocity affected by current frame's force to handle resting contact
		Vector3 accCausedVelocity = particle[0]->getAcceleration();
		if (particle[1])accCausedVelocity -= particle[1]->getAcceleration();
		real accCausedSepVelocity = dot(accCausedVelocity, contactNormal) * duration;
		if (accCausedSepVelocity < 0) {
			newSepVelocity += accCausedSepVelocity*restitution;
			if (newSepVelocity < 0)newSepVelocity = 0;
		}

		real deltaVelocity = newSepVelocity - separatingVelocity;

		real totalInverseMass = particle[0]->getInverseMass();
		if (particle[1])totalInverseMass += particle[1]->getInverseMass();
		if (totalInverseMass <= 0)return;
		real impulse = deltaVelocity / totalInverseMass;// p = mdv
		Vector3 impulsePerIMass = impulse * contactNormal;
		
		particle[0]->setVelocity(particle[0]->getVelocity() + impulsePerIMass * particle[0]->getInverseMass());
		if (particle[1]) {
			particle[1]->setVelocity(particle[1]->getVelocity() - impulsePerIMass * particle[1]->getInverseMass());
		}
	}
	void ParticleContact::resolveInterpenetration(real duration) {
		if (penetration <= 0)return;
		real totalInverseMass = particle[0]->getInverseMass();
		if (particle[1])totalInverseMass += particle[1]->getInverseMass();
		if (totalInverseMass <= 0)return;
		Vector3 movePerIMass = contactNormal * penetration/totalInverseMass;
		particleMovement[0] = movePerIMass * particle[0]->getInverseMass();
		particle[0]->setPosition(particle[0]->getPosition()+ particleMovement[0]);
		if (particle[1]) {
			particleMovement[1] = -movePerIMass * particle[1]->getInverseMass();
			particle[1]->setPosition(particle[1]->getPosition() + particleMovement[1]);
		}
		else {
			particleMovement[1] = Vector3(0, 0, 0);
		}

	}
	void ParticleContactResolver::setInterations(unsigned iterations) {
		this->iterations = iterations;
	}
	void ParticleContactResolver::resolveContacts(std::vector<ParticleContact>& particleContacts, real duration) {
		resolveContacts(particleContacts.data(), particleContacts.size(), duration);
	}
	void ParticleContactResolver::resolveContacts(ParticleContact* particleContacts, unsigned numContacts, real duration) {
		iterationUsed = 0;
		while (iterationUsed < iterations) {
			real min = 0;
			unsigned identityPerComponent = numContacts;
			for (unsigned i = 0; i < numContacts; ++i) {
				real sepVel = particleContacts[i].calculateSepartingVelocity();
				if (sepVel < min) {
					min = sepVel;
					identityPerComponent = i;
				}
			}
			if (identityPerComponent == numContacts)break;
			particleContacts[identityPerComponent].resolve(duration);
	

			// Update the interpenetrations for all particles
			Vector3* move = particleContacts[identityPerComponent].particleMovement;
			for (unsigned i = 0; i < numContacts; i++)
			{
				if (particleContacts[i].particle[0] == particleContacts[identityPerComponent].particle[0])
				{
					particleContacts[i].penetration -= dot(move[0] , particleContacts[i].contactNormal);
				}
				else if (particleContacts[i].particle[0] == particleContacts[identityPerComponent].particle[1])
				{
					particleContacts[i].penetration -= dot( move[1] , particleContacts[i].contactNormal);
				}
				if (particleContacts[i].particle[1])
				{
					if (particleContacts[i].particle[1] == particleContacts[identityPerComponent].particle[0])
					{
						particleContacts[i].penetration += dot(move[0] , particleContacts[i].contactNormal);
					}
					else if (particleContacts[i].particle[1] == particleContacts[identityPerComponent].particle[1])
					{
						particleContacts[i].penetration += dot( move[1] , particleContacts[i].contactNormal);
					}
				}
			}
			iterationUsed++;
		}
	}
	 unsigned PlaneContactGenerator::addContact(ParticleContact* contact, unsigned limit) const{
		 unsigned maxLimit = limit;
		 for (auto& p : particle) {
			 if (limit <= 0)break;
			 if (p->getPosition().y >= planeHeight)continue;
			 Vector3 normal = Vector3(0, 1, 0);
			 contact->contactNormal = Vector3(0, 1, 0);
			 contact->particle[0] = p;
			 contact->particle[1] = nullptr;
			 contact->penetration =  planeHeight - p->getPosition().y;
			 contact->restitution = 0.9;
			 limit--;
			 contact++;
		 }
		 return maxLimit - limit;
	}
}