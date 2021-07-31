#pragma once
#include"numerical.h"
#include"pContacts.h"
namespace tEngine {
	class Particle;
	class ParticleContact;
	class ParticleLink:public ParticleContactGenerator {
	public:
		Particle* particle[2];
	protected:
		real currentLength()const;
	public:

		// unsigned addContact(ParticleContact* contact,unsigned limit)const = 0;

	};
	class ParticleCable :public ParticleLink {
	public:
		real maxLength;
		real restitution;
	public:
		unsigned addContact(ParticleContact* contact, unsigned limit)const override;
	//	unsigned fillContact(ParticleContact* contact, unsigned limit)const override;

	};
	class ParticleRod :public ParticleLink {
	public:
		real length;
	public:
		unsigned addContact(ParticleContact* contact, unsigned limit)const override;
	//	unsigned fillContact(ParticleContact* contact, unsigned limit)const override;
	};
}