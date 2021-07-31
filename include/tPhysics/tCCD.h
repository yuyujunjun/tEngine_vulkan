#pragma once
#include"numerical.h"
#include"collide.h"
#include"ccd/ccd.h"
namespace tEngine {
	class Collider;
	struct ContactInfo {
		Vector3 pos;
		Vector3 dir;
		real depth;
	};
	int GJKPenetration(const Collider* obj1, const Collider* obj2, ContactInfo* info);
	bool GJKIntersect(const Collider* obj1, const Collider* obj2);
}