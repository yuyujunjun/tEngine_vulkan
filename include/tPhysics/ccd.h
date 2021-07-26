#pragma once
#include"physicsCore.h"
#include"collide.h"
namespace tPhysics {
	class Collider;
	struct Support {
		Vector3 v;// Support point in minkowski sum
		Vector3 v1;//Support point in obj1
		Vector3 v2;//Support point in obj2
	};
	struct Simplex {
		Support pt[4];
		unsigned last_id = -1;
	};
	
	int GJK(const Collider* obj1, const Collider* obj2,unsigned maxIterations);
}