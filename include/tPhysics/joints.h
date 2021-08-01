#pragma once
#include"contacts.h"
namespace tEngine {
	class Joints :public ContactGenerator {
		RigidBody* rigidBody[2];
		Vector3 position[2];
		real error;
		unsigned addContact(Contact* contact, unsigned limit)const override;
	};
}