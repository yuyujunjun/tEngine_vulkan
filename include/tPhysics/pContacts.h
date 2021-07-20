#pragma once
#include<vector>
#include"physicsCore.h"
namespace tPhysics {
	class Particle;
	class ParticleContact {
	
		
		
	public:
		real restitution;
		Vector3 contactNormal;//contact normal from the first object's perspective
		real penetration;
		Particle* particle[2];
		//resolve this contact
		void resolve(real duration);
		real calculateSepartingVelocity()const;
		
	private:
		//Impulse calcution
		void resolveVelocity(real duration);
		void resolveInterpenetration(real duration);
	}; 
	class ParticleContactResolver {
	protected:
		unsigned iterations;
		unsigned iterationUsed;
	public:
		ParticleContactResolver(unsigned iterations) :iterations(iterations), iterationUsed(0) {};
		void setInterations(unsigned iterations);
		void resolveContacts(std::vector<ParticleContact>& particleContacts, real duration);
		void resolveContacts(ParticleContact* particleContacts,unsigned numContacts, real duration);
	};
	class ParticleContactGenerator {
	public:
		virtual unsigned addContact(ParticleContact* contact, unsigned limit)const = 0;
	};
}