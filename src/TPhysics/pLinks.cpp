#include"pLinks.h"
#include"pContacts.h"
#include"tParticles.h"
namespace tEngine {
	real ParticleLink::currentLength()const {
		return glm::length(particle[0]->getPosition() - particle[1]->getPosition());
	}
	unsigned ParticleCable::addContact(ParticleContact* contact, unsigned limit)const {
		//normal is inversed because we want them to get closer rather than separating
		Vector3 normal = particle[1]->getPosition() - particle[0]->getPosition();
		real length = glm::length(normal);
		if (length < maxLength)return 0;
		contact->particle[0] = particle[0];
		contact->particle[1] = particle[1];
		contact->contactNormal = glm::normalize(normal);
		contact->penetration = length - maxLength;
		contact->restitution = restitution;
		return 1;
		
	}
	unsigned ParticleRod::addContact(ParticleContact* contact, unsigned limit)const {
		Vector3 normal = particle[0]->getPosition() - particle[1]->getPosition();
		real currentLen = glm::length(normal);
		if (currentLen == this->length) { return 0; }
		contact->particle[0] = particle[0];
		contact->particle[1] = particle[1];
		normal = glm::normalize(normal);
		if (currentLen > length) {
			contact->contactNormal = -normal;
			contact->penetration = currentLen - length;
		}
		else {
			contact->contactNormal = normal;
			contact->penetration = length - currentLen;
		}
		contact->restitution = 0;
		return 1;
	}
}