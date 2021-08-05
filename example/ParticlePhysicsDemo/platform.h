#pragma once
#include"physicsCore.h"
#include"Component.h"
#include"tParticles.h"
#include"pWorld.h"
#include"pLinks.h"
#include"SimpleGeometry.h"
#include"GameObject.h"
#include"MeshBuffer.h"
#include"Device.h"
#include"Log.h"
#define ROD_COUNT 15
#define BASE_MASS 1
#define EXTRA_MASS 10
namespace tEngine {
	using namespace tEngine;
	class Platform :public Component
	{
        Particle particleArray[6];
        ParticleRod* rods;
        ParticleAnchoredBungee bungee[2];
        ParticleGravity gravity;
        PlaneContactGenerator planeContact;
		//Vector3 massPos;
		Vector3 massPosDisplay;
        Mesh mesh;
        std::array<unsigned,24> identityPerComponent;
    public:
        Platform(GameObject_* gameObject,ParticleWorld* pWorld):Component(gameObject),gravity(Vector3(0,-10,0)) {
            setParticleArray(pWorld);
            mesh.vertices.resize(24);
            mesh.indices.resize(24);
            identityPerComponent = { 0,2,1,1,2,3,0,1,4,4,1,5,0,4,2,1,3,5,3,2,4,5,3,4 };
            for (uint32_t i = 0; i < 24; ++i) {
                mesh.indices[i] = i;
            }
           
        }
        const Mesh& getMesh()const { return mesh; }
        void updateAdditionalMass(real xp,real zp)
        {
            //massPos = Vector3();
            for (unsigned i = 2; i < 6; i++)
            {
                particleArray[i].setMass(BASE_MASS);
            }

            // Find the coordinates of the mass as an index and proportion
            //real xp = massPos.x;
            if (xp < 0) xp = 0;
            if (xp > 1) xp = 1;

            //real zp = massPos.z;
            if (zp < 0) zp = 0;
            if (zp > 1) zp = 1;
           // LOG(LogLevel::Information, xp, zp);
            // Calculate where to draw the mass
           // massDisplayPos.clear();
            massPosDisplay = Vector3(0, 0, 0);
            // Add the proportion to the correct masses
            particleArray[2].setMass(BASE_MASS + EXTRA_MASS * (1 - xp) * (1 - zp));
            massPosDisplay += particleArray[2].getPosition() * (1 - xp) * (1 - zp);
            

            if (xp > 0)
            {
                particleArray[4].setMass(BASE_MASS + EXTRA_MASS * xp * (1 - zp));
                massPosDisplay += (
                    particleArray[4].getPosition()* xp * (1 - zp)
                );

                if (zp > 0)
                {
                    particleArray[5].setMass(BASE_MASS + EXTRA_MASS * xp * zp);
                    massPosDisplay+=(
                        particleArray[5].getPosition(), xp * zp
                    );
                }
            }
            if (zp > 0)
            {
                particleArray[3].setMass(BASE_MASS + EXTRA_MASS * (1 - xp) * zp);
                massPosDisplay+=(
                    particleArray[3].getPosition(), (1 - xp) * zp
                );
            }
        }

        void setParticleArray(ParticleWorld* pWorld)
        {
            auto GRAVITY = Vector3(0, -10, 0);
            // Create the masses and connections.
            particleArray[0].setPosition(Vector3(0, 0, 1));
            particleArray[1].setPosition(Vector3(0, 0, -1));
            particleArray[2].setPosition(Vector3(-3, 2, 1));
            particleArray[3].setPosition(-3, 2, -1);
            particleArray[4].setPosition(4, 2, 1);
            particleArray[5].setPosition(4, 2, -1);
            for (unsigned i = 0; i < 6; i++)
            {
                particleArray[i].setMass(BASE_MASS);
                particleArray[i].setVelocity({ 0, 0, 0 });
                particleArray[i].setDamping(0.9f);
           //     particleArray[i].setAcceleration(GRAVITY);
                particleArray[i].clearAccumulator();
            }
           // particleArray[0].setInverseMass(0);
           // particleArray[1].setInverseMass(0);

            rods = new ParticleRod[ROD_COUNT];

            rods[0].particle[0] = &particleArray[0];
            rods[0].particle[1] = &particleArray[1];
            rods[0].length = 2;
            rods[1].particle[0] = &particleArray[2];
            rods[1].particle[1] = &particleArray[3];
            rods[1].length = 2;
            rods[2].particle[0] = &particleArray[4];
            rods[2].particle[1] = &particleArray[5];
            rods[2].length = 2;

            rods[3].particle[0] = &particleArray[2];
            rods[3].particle[1] = &particleArray[4];
            rods[3].length = 7;
            rods[4].particle[0] = &particleArray[3];
            rods[4].particle[1] = &particleArray[5];
            rods[4].length = 7;

            rods[5].particle[0] = &particleArray[0];
            rods[5].particle[1] = &particleArray[2];
            rods[5].length = 3.606;
            rods[6].particle[0] = &particleArray[1];
            rods[6].particle[1] = &particleArray[3];
            rods[6].length = 3.606;

            rods[7].particle[0] = &particleArray[0];
            rods[7].particle[1] = &particleArray[4];
            rods[7].length = 4.472;
            rods[8].particle[0] = &particleArray[1];
            rods[8].particle[1] = &particleArray[5];
            rods[8].length = 4.472;

            rods[9].particle[0] = &particleArray[0];
            rods[9].particle[1] = &particleArray[3];
            rods[9].length = 4.123;
            rods[10].particle[0] = &particleArray[2];
            rods[10].particle[1] = &particleArray[5];
            rods[10].length = 7.28;
            rods[11].particle[0] = &particleArray[4];
            rods[11].particle[1] = &particleArray[1];
            rods[11].length = 4.899;
            rods[12].particle[0] = &particleArray[1];
            rods[12].particle[1] = &particleArray[2];
            rods[12].length = 4.123;
            rods[13].particle[0] = &particleArray[3];
            rods[13].particle[1] = &particleArray[4];
            rods[13].length = 7.28;
            rods[14].particle[0] = &particleArray[5];
            rods[14].particle[1] = &particleArray[0];
            rods[14].length = 4.899;
            
            bungee[0].setAnchor(Vector3(-3, 10, 1));
            bungee[0].setRestLength(3);
            bungee[0].setSpringConstant(20);
            bungee[1].setAnchor(Vector3(-3, 10, -1));
            bungee[1].setRestLength(3);
            bungee[1].setSpringConstant(20);
            for (unsigned i = 0; i < ROD_COUNT; i++)
            {
                pWorld->getContactGenerators().push_back(&rods[i]);
            }
            for (unsigned i = 0; i < 6; ++i) {
                pWorld->addParticle(&particleArray[i]);
                pWorld->getForceRegistry().add(particleArray + i, &gravity);
            }

            for (unsigned i = 0; i < 6; ++i) {
                planeContact.particle.push_back(particleArray + i);
               
            }
            pWorld->getContactGenerators().push_back(&planeContact);
            
            updateAdditionalMass(0,0);
        }
        void updateMesh() {
            for (unsigned i = 0; i < 24; ++i) {
                mesh.vertices[i].Position = particleArray[identityPerComponent[i]].getPosition();
            }
            mesh.UpdateNormal();
            auto meshBuffer=gameObject->getComponent<MeshFilter>();
            meshBuffer->setMesh(mesh);
            meshBuffer->uploadVertexBuffer(nullptr,nullptr);

        }
        void Rope(ParticleWorld* pWorld) {
            pWorld->getForceRegistry().add(particleArray + 2, bungee);
            pWorld->getForceRegistry().add(particleArray + 3, bungee + 1);
        }
        void cutRope(ParticleWorld* pWorld) {
            pWorld->getForceRegistry().remove(particleArray + 2, bungee);
            pWorld->getForceRegistry().remove(particleArray + 3, bungee + 1);
        }
        void initializeMesh(Device* device) {
            auto meshBuffer = gameObject->getComponent<MeshFilter>();// ->setMesh(mesh);
            meshBuffer->setMesh(mesh);
            auto uploadData = [meshBuffer, device](CommandBufferHandle& cmd) {
                meshBuffer->createVertexBuffer(device, cmd, BufferDomain::Host);
                meshBuffer->createIdxBuffer(device, cmd, BufferDomain::Device);
                meshBuffer->uploadIdxBuffer(device, cmd);
            };
            oneTimeSubmit(device, uploadData);
            updateMesh();
        }
        ~Platform() {
            delete[] rods;
        }
	};

}