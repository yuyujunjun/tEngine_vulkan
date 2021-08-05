#include"AABB.h"
namespace tEngine {
	bool Overlap(const AABB* a, const AABB* b) {
		auto overlapAxis = [a, b](unsigned identityPerComponent)->bool {
			return a->minCoord[identityPerComponent] <= b->maxCoord[identityPerComponent] && a->maxCoord[identityPerComponent] >= b->minCoord[identityPerComponent];
		};
		return overlapAxis(0) && overlapAxis(1) && overlapAxis(2);
	}
}