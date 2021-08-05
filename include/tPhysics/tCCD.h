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
	struct GJKSupportObj {
		ColliderSystem* system;
		EntityID id;
	};
	int GJKPenetration(const GJKSupportObj obj1, const GJKSupportObj obj2, ContactInfo* info);
	bool GJKIntersect(const GJKSupportObj obj1, const GJKSupportObj obj2);
}