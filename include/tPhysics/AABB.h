#pragma once
#include"numerical.h"

namespace tEngine {
	class AABB {
		Vector3 center;
		Vector3 halfSizes;
		Vector3 minCoord;
		Vector3 maxCoord;

	public:

		AABB() :center(0, 0, 0), halfSizes(0, 0, 0), minCoord(MAXREAL), maxCoord(MINREAL) {}
		void setCenter(const Vector3& center) {
			this->center = center;
			minCoord = center - halfSizes;
			maxCoord = center + halfSizes;
		}
		void setSize(const Vector3& size) {
			
			setHalfSize(size / 2.f);
			
		}
		void setHalfSize(const Vector3& halfSize) {
			halfSizes = halfSize;
			minCoord = center - halfSizes;
			maxCoord = center + halfSizes;
		}
		const Vector3& getCenter()const { return center; }
		const Vector3& getHalfSize()const { return halfSizes; }
		void ADD(const AABB* box) {
			this->minCoord = glm::min(minCoord, box->minCoord);
			this->maxCoord = glm::max(maxCoord, box->maxCoord);
			this->center = (maxCoord + minCoord) / (real)2;
			this->halfSizes = (maxCoord - minCoord) / (real)2;
		}

		void ADD(const Vector3& pt) {
			this->minCoord = glm::min(pt, minCoord);
			this->maxCoord = glm::max(pt, maxCoord);
			this->center = (maxCoord + minCoord) / (real)2;
			this->halfSizes = (maxCoord - minCoord) / (real)2;
		}
		void ADD(real x, real y, real z) {
			Vector3 pt(x, y, z);
			ADD(pt);
		}
		void CLEAR() {
			this->minCoord = Vector3(MAXREAL);
			this->maxCoord = Vector3(MINREAL);
			this->center = Vector3(0, 0, 0);
			this->halfSizes = Vector3(0, 0, 0);
		}
		void SET(const Vector3& center, const Vector3& halfSize) {
			this->center = center;
			this->halfSizes = halfSize;
			this->minCoord = center - halfSize;
			this->maxCoord = center + halfSize;
		}
		void EXPAND(const Vector3& extents) {
			this->minCoord -= extents;
			this->maxCoord += extents;
			this->center = (this->minCoord + this->maxCoord)/2.f;
			this->halfSizes = (this->maxCoord - this->minCoord)/2.f;
		}
		const Vector3& MinCoords() { return minCoord; }
		const Vector3& MaxCoords() { return maxCoord; }
		friend bool Overlap(const AABB* a, const AABB* b);
	};
	
}