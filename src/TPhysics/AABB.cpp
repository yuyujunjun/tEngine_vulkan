#include"AABB.h"
namespace tPhysics {
	bool Overlap(const AABB* a, const AABB* b) {
		auto overlapAxis = [a, b](unsigned idx)->bool {
			return a->minCoord[idx] <= b->maxCoord[idx] && a->maxCoord[idx] >= b->minCoord[idx];
		};
		return overlapAxis(0) && overlapAxis(1) && overlapAxis(2);
	}
}