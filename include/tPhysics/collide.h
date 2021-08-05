#pragma once
#include<functional>
#include<vector>
#include"numerical.h"
#include"AABB.h"
#include"ecs.h"

namespace tEngine {
	struct Mesh;
	class GameObject_;
	class Transform;
}


namespace tEngine {
	
	struct SphereColliderData {
		real radius;
	};
	struct BoxColliderData {
		Vector3 halfSize;
	};
	struct MeshColliderData {
		std::vector<Vector3> vertices;
		std::vector<Vector3> worldVertices;
		void setMesh(const tEngine::Mesh& mesh);
	};
	struct Collider {
		enum class ColliderType {
			Box,
			Sphere,
			Mesh
		};
	public:
	//	Vector3 worldCenter;
		AABB coarseAABB;
		/// <summary>
		/// if isTrigger is true, only detect if collide, otherwise detect the penetration at the same time
		/// </summary>
		bool isTrigger;
		Collider() :isTrigger(false) {}
		const AABB* getAABB()const { return &coarseAABB; }	
	};
	class ColliderSystem:public System {
	public:

		ColliderSystem()  {}
		virtual Vector3 SupportPoint(EntityID id,const Vector3& direction)const { return Vector3(0, 0, 0); };
		virtual void UpdateDerivedData(EntityID id) {};
	};
	/// <summary>
	/// Center equals to origin, radius equals to 1
	/// </summary>
	class SphereColliderSystem :public ColliderSystem {
	
	public:
		friend Vector3 SphereSupport(const void* obj, const Vector3& dir);
		SphereColliderSystem(){
			
		}
		Vector3 SupportPoint(EntityID id, const Vector3& direction)const override;
		void UpdateDerivedData(EntityID id)override;
		
	};
	
	class BoxColliderSystem :public ColliderSystem {
		/// <summary>
		/// center and halfSize measured in local space
		/// </summary>
	public:

		friend Vector3 BoxSupport(const void* obj, const Vector3& dir);
		void UpdateDerivedData(EntityID id)override;
		Vector3 SupportPoint(EntityID id, const Vector3& direction)const override;
	};
	class MeshColliderSystem :public ColliderSystem {
	
	public:


		void UpdateDerivedData(EntityID id)override;
		Vector3 SupportPoint(EntityID id,const Vector3& direction)const override;
	};
}