#include"collide.h"
#include"GameObject.h"
#include"tTransform.h"
#include"SimpleGeometry.h"
namespace tEngine {
	void SphereColliderSystem::UpdateDerivedData(EntityID id) {
		auto collider = ecsManager->GetComponent<Collider>(id);
		auto transform = ecsManager->GetComponent<Transform>(id);
		auto data = ecsManager->GetComponent<SphereColliderData>(id);
		collider->coarseAABB.CLEAR();
	//	collider->worldCenter = transform->getPosition();
		Mat4 M = transform->getMat3();M[3][3] = 1;
		Mat4 S = Mat4(data->radius* data->radius);S[3][3] = -1;
		M = M * S * glm::transpose(M);
		auto x = (M[0][3] + std::sqrt(M[0][3] * M[0][3] - M[3][3] * M[0][0])) / M[3][3];// +M[0][2] * M[0][2]);
		auto y = (M[1][3] + std::sqrt(M[1][3] * M[1][3] - M[3][3] * M[1][1])) / M[3][3];// +M[1][2] * M[1][2]);
		auto z = (M[2][3] + std::sqrt(M[2][3] * M[2][3] - M[3][3] * M[2][2])) / M[3][3];// +M[2][2] * M[2][2]);
		collider->coarseAABB.SET(transform->getPosition(), Vector3(x,y,z));

	}
	Vector3 SphereColliderSystem::SupportPoint(EntityID id, const Vector3& direction)const  {
		auto collider = ecsManager->GetComponent<Collider>(id);
		auto transform = ecsManager->GetComponent<Transform>(id);
		auto data = ecsManager->GetComponent<SphereColliderData>(id);
		auto normalizedDir = glm::normalize(direction);

		auto dir = transform->rotationInverseDirection(normalizedDir);
		dir *= data->radius;

		auto pos = transform->transformDirection(dir);
		pos += transform->getPosition();
		return pos;
	}
	
	void BoxColliderSystem::UpdateDerivedData(EntityID id) {
		auto collider = ecsManager->GetComponent<Collider>(id);
		auto transform = ecsManager->GetComponent<Transform>(id);
		auto data = ecsManager->GetComponent<BoxColliderData>(id);
		collider->coarseAABB.CLEAR();
	
		glm::vec3 worldExtent=absMat(transform->getMat3())*data->halfSize;
		collider->coarseAABB.SET(transform->getPosition(), worldExtent);
	}
	Vector3 BoxColliderSystem:: SupportPoint(EntityID id, const Vector3& direction)const {
		auto collider = ecsManager->GetComponent<Collider>(id);
		auto transform = ecsManager->GetComponent<Transform>(id);
		auto data = ecsManager->GetComponent<BoxColliderData>(id);
		auto& halfSize =  data->halfSize;
		auto dir =  transform->rotationInverseDirection(direction);
		Vector3 localPoint(sign(dir.x) * halfSize.x,
			sign(dir.y) * halfSize.y,
			sign(dir.z) * halfSize.z);

		localPoint =  transform->transformDirection(localPoint);
		localPoint =  transform->getPosition() + localPoint;
		return localPoint;
	}

	void MeshColliderData::setMesh(const tEngine::Mesh& mesh) {
		vertices.resize(mesh.vertices.size());
		worldVertices.resize(mesh.vertices.size());
		for (int i = 0; i < mesh.vertices.size();++i) {
			vertices[i] = mesh.vertices[i].Position;
		}
	}
	void MeshColliderSystem::UpdateDerivedData(EntityID id) {
		auto collider = ecsManager->GetComponent<Collider>(id);
		auto transform = ecsManager->GetComponent<Transform>(id);
		auto data = ecsManager->GetComponent<MeshColliderData>(id);
		const Mat3& transformMat = transform->getMat3();
		const Vector3& position = transform->getPosition();
		Vector3 maxPoint(MINREAL);
		Vector3 minPoint(MAXREAL);
		for (int i = 0; i < data->vertices.size(); ++i) {
			data->worldVertices[i] = transformMat * data->vertices[i] + position;
			maxPoint = glm::max(data->worldVertices[i], maxPoint);
			minPoint = glm::min(data->worldVertices[i], minPoint);
		}
		collider->coarseAABB.SET((maxPoint+minPoint)/2.f,(maxPoint-minPoint)/2.f);
	}
	Vector3 MeshColliderSystem::SupportPoint(EntityID id,const Vector3& direction)const {
		Vector3 normalied_dir =glm::normalize( direction);
		real max = MINREAL;
		auto data = ecsManager->GetComponent<MeshColliderData>(id);
		Vector3 result(0, 0, 0);
		for (int i = 0; i < data->worldVertices.size(); ++i) {
			auto dotv = glm::dot(normalied_dir, data->worldVertices[i]);
			if (max < dotv) {
				max = dotv;
				result = data->worldVertices[i];
			}
		}
		return result;
	}

}