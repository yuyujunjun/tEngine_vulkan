#include"tParticles.h"
#include"pContacts.h"
namespace tPhysics {
	void ParticleContact::resolve(real duration) {
		resolveVelocity(duration);
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
		if (totalInverseMass < 0)return;
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
		particle[0]->setPosition(particle[0]->getPosition()+movePerIMass*particle[0]->getInverseMass());
		if (particle[1]) {
			particle[1]->setPosition(particle[1]->getPosition() - movePerIMass * particle[1]->getInverseMass());
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
			unsigned idx = numContacts;
			for (unsigned i = 0; i < numContacts; ++i) {
				real sepVel = particleContacts[i].calculateSepartingVelocity();
				if (sepVel < min) {
					min = sepVel;
					idx = i;
				}
			}
			if (idx == numContacts)break;
			particleContacts[idx].resolve(duration);
			iterationUsed++;
		}
	}
}