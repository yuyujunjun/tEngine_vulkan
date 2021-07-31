#include"collide.h"
#include"GameObject.h"
#include"tTransform.h"
#include"SimpleGeometry.h"
namespace tEngine {
	void SphereCollider::UpdateDerivedData() {
		this->coarseAABB.CLEAR();
		worldCenter = center + gameObject->transform.getPosition();
		Mat4 M = gameObject->transform.getMat3();M[3][3] = 1;
		Mat4 S = Mat4(scale*scale);S[3][3] = -1;
		M = M * S * glm::transpose(M);
		auto x = (M[0][3] + std::sqrt(M[0][3] * M[0][3] - M[3][3] * M[0][0])) / M[3][3];// +M[0][2] * M[0][2]);
		auto y = (M[1][3] + std::sqrt(M[1][3] * M[1][3] - M[3][3] * M[1][1])) / M[3][3];// +M[1][2] * M[1][2]);
		auto z = (M[2][3] + std::sqrt(M[2][3] * M[2][3] - M[3][3] * M[2][2])) / M[3][3];// +M[2][2] * M[2][2]);
		this->coarseAABB.SET(worldCenter, Vector3(x,y,z));

	}
	Vector3 SphereSupport(const void* obj_, const Vector3& direction) {
		auto normalizedDir = glm::normalize(direction);
		SphereCollider* obj = (SphereCollider*)obj_;
		auto dir=obj->gameObject->transform.rotationInverseDirection(normalizedDir);
		dir *= obj->scale;
		dir += obj->center;
		auto pos = obj->gameObject->transform.transformDirection(dir);
		pos += obj->gameObject->transform.getPosition();
		return pos;
	}
	void BoxCollider::UpdateDerivedData() {
		this->coarseAABB.CLEAR();
		worldCenter = center + gameObject->transform.getPosition();
		glm::vec3 worldExtent=absMat(gameObject->transform.getMat3())*halfSize;
		this->coarseAABB.SET(worldCenter , worldExtent);
	}
	Vector3 BoxSupport(const void* obj_, const Vector3& direction) {
		BoxCollider* obj = (BoxCollider*)obj_;
		
		auto dir = obj->gameObject->transform.rotationInverseDirection(direction);
		Vector3 localPoint(sign(dir.x)*obj->halfSize.x,
			sign(dir.y)*obj->halfSize.y,
			sign(dir.z)*obj->halfSize.z);
		localPoint += obj->center;
		localPoint = obj->gameObject->transform.transformDirection(localPoint);
		localPoint = obj->worldCenter + localPoint;
		return localPoint;
	}
	void MeshCollider::setMesh(const tEngine::Mesh& mesh) {
		vertices.resize(mesh.vertices.size());
		worldVertices.resize(mesh.vertices.size());
		for (int i = 0; i < mesh.vertices.size();++i) {
			vertices[i] = mesh.vertices[i].Position;
		}
	}
	void MeshCollider::UpdateDerivedData() {
		worldCenter = gameObject->transform.getPosition();
		const Mat3& transform = gameObject->transform.getMat3();
		const Vector3& position = gameObject->transform.getPosition();
		Vector3 maxPoint(MINREAL);
		Vector3 minPoint(MAXREAL);
		for (int i = 0; i < vertices.size(); ++i) {
			worldVertices[i] = transform * vertices[i] + position;
			maxPoint = glm::max(worldVertices[i], maxPoint);
			minPoint = glm::min(worldVertices[i], minPoint);
		}
		coarseAABB.SET((maxPoint+minPoint)/2.f,(maxPoint-minPoint)/2.f);
	}
	Vector3 MeshCollider::SupportPoint(const Vector3& direction)const {
		Vector3 normalied_dir =glm::normalize( direction);
		real max = MINREAL;
		Vector3 result(0, 0, 0);
		for (int i = 0; i < worldVertices.size(); ++i) {
			auto dotv = glm::dot(normalied_dir, worldVertices[i]);
			if (max < dotv) {
				max = dotv;
				result = worldVertices[i];
			}
		}
		return result;
	}

}