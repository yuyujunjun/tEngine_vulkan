#include"collide.h"
#include"GameObject.h"
#include"tTransform.h"
namespace tPhysics {
	void SphereCollider::UpdateDerivedData() {
		this->coarseAABB.CLEAR();
		auto worldCenter = center + gameObject->transform.getPosition();
		Mat4 M = gameObject->transform.getMat3();M[3][3] = 1;
		Mat4 S = Mat4(scale*scale);S[3][3] = -1;
		M = M * S * glm::transpose(M);
		auto x = (M[0][3] + std::sqrt(M[0][3] * M[0][3] - M[3][3] * M[0][0])) / M[3][3];// +M[0][2] * M[0][2]);
		auto y = (M[1][3] + std::sqrt(M[1][3] * M[1][3] - M[3][3] * M[1][1])) / M[3][3];// +M[1][2] * M[1][2]);
		auto z = (M[2][3] + std::sqrt(M[2][3] * M[2][3] - M[3][3] * M[2][2])) / M[3][3];// +M[2][2] * M[2][2]);
		this->coarseAABB.SET(worldCenter, Vector3(x,y,z));

	}
	void SphereSupport(const void* obj_, const ccd_vec3_t* dir_, ccd_vec3_t* vec) {
		SphereCollider* obj = (SphereCollider*)obj_;
		auto dir=obj->gameObject->transform.rotationInverseDirection(Vector3(dir_->v[0], dir_->v[1], dir_->v[2]));
		//dir = glm::normalize(dir);
		dir += obj->center;
		auto pos = obj->gameObject->transform.transformDirection(dir);
		pos += obj->gameObject->transform.getPosition();
		ccdVec3Set(vec, pos.x, pos.y, pos.z);
	}
	void BoxCollider::UpdateDerivedData() {
		this->coarseAABB.CLEAR();
		worldCenter = center + gameObject->transform.getPosition();
		glm::vec3 worldExtent=absMat(gameObject->transform.getMat3())*halfSize;
		this->coarseAABB.SET(worldCenter , worldExtent);
	}
	void BoxSupport(const void* obj_, const ccd_vec3_t* dir_, ccd_vec3_t* vec) {
		BoxCollider* obj = (BoxCollider*)obj_;
		Vector3 dir(dir_->v[0],dir_->v[1],dir_->v[2]);
		dir = obj->gameObject->transform.rotationInverseDirection(dir);
		Vector3 localPoint(sign(dir.x)*obj->halfSize.x,
			sign(dir.y)*obj->halfSize.y,
			sign(dir.z)*obj->halfSize.z);
		localPoint = obj->gameObject->transform.transformDirection(localPoint);
		localPoint = obj->worldCenter + localPoint;
		ccdVec3Set(vec, localPoint.x, localPoint.y, localPoint.z);
	}
}