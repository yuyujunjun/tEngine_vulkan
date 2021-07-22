#pragma once
#include"physicsCore.h"
#include"pForceGen.h"
#include"pContacts.h"
#include<list>
namespace tPhysics {
	class Particle;
	class ParticleWorld {
		using ContactGenerators = std::vector<ParticleContactGenerator*>;
		std::list<Particle*> pList;
		ParticleForceRegistry registry;//force generator and particle
		ContactGenerators contactGenerators;//contact generator
		std::vector<ParticleContact> contacts;//contact list
		unsigned maxContacts;
		ParticleContactResolver resolver;


	public:
		bool calculateIterations=true;
		ParticleWorld(unsigned maxContacts, unsigned iterations = 0)  ;
		~ParticleWorld();
		void addParticle(Particle* particle);
		void removeParticle(Particle* particle);
		void startFrame();
		unsigned generateContacts();
		void integrate(real duration);
		void runPhysics(real duration);
		ContactGenerators& getContactGenerators() { return contactGenerators; }
		ParticleForceRegistry& getForceRegistry() { return registry; }
	};
}