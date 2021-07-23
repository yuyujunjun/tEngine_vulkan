#pragma once
#include"physicsCore.h"
#include"pForceGen.h"
#include"tTransform.h"
#include"RigidBody.h"
using namespace tPhysics;
using namespace tEngine;

class FlightSims
{
public:
	AeroControl left_wing;
	AeroControl right_wing;
	AeroControl rudder;
    Aero tail;
	Vector3 windSpeed;
    RigidBody* aircraft;
	float left_wing_control;
	float right_wing_control;
	float rudder_control;

    FlightSims(RigidBody* aircraft) :windSpeed(0, 0, 0), right_wing(Mat3(0, 0, 0, -1, -0.5f, 0, 0, 0, 0),
        Mat3(0, 0, 0, -0.995f, -0.5f, 0, 0, 0, 0),
        Mat3(0, 0, 0, -1.005f, -0.5f, 0, 0, 0, 0),
        Vector3(-1.0f, 0.0f, 2.0f), windSpeed),
        left_wing(Mat3(0, 0, 0, -1, -0.5f, 0, 0, 0, 0),
            Mat3(0, 0, 0, -0.995f, -0.5f, 0, 0, 0, 0),
            Mat3(0, 0, 0, -1.005f, -0.5f, 0, 0, 0, 0),
            Vector3(-1.0f, 0.0f, -2.0f), windSpeed),
        rudder(Mat3(0, 0, 0, 0, 0, 0, 0, 0, 0),
            Mat3(0, 0, 0, 0, 0, 0, 0.01f, 0, 0),
            Mat3(0, 0, 0, 0, 0, 0, -0.01f, 0, 0),
            Vector3(2.0f, 0.5f, 0), windSpeed),
        tail(Mat3(0, 0, 0, -1, -0.5f, 0, 0, 0, -0.1f),
            Vector3(2.0f, 0, 0), windSpeed),
        left_wing_control(0), right_wing_control(0), rudder_control(0),
        aircraft(aircraft) {
        
        aircraft->setMass(2.5f);
        Vector3 halfSizes(1, 1, 1);
        auto square = halfSizes* halfSizes;
        Mat3 inertiaTensor;
        setBlockInertiaTensor(inertiaTensor, { 2,1,1 }, 1.f);
        aircraft->setInertiaTensor(inertiaTensor);
        aircraft->setLinearDamping(0.8f);
        aircraft->setAngularDamping(0.8f);
        aircraft->setAcceleration({ 0,-10,0 });
        aircraft->calculateDerivedData();
        aircraft->setAwake(true);
        aircraft->setCanSleep(false);

    }
   
};

